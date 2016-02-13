// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "miniv.h"
#include <stdio.h>

static void eat(struct decoder *dec, ulong_t n) {
  //bufDump("eat", dec->left);
  dec->left.buf += n;
  dec->left.len -= n;
}

static err_t readByte(struct decoder *dec, unsigned char *v) {
  if (dec->left.len == 0) {
    return ERR_DECVOM;
  }
  *v = dec->left.buf[0];
  eat(dec, 1);
  return ERR_OK;
}

static err_t readVar128(struct decoder *dec, uint64_t *v, vomControl *ctl) {
  *ctl = controlNone;
  
  if (dec->left.len == 0) {
    return ERR_DECVOM;
  }

  unsigned char uc = dec->left.buf[0];
  if (uc < 128) {
    *v = uc;
    eat(dec, 1);
    return ERR_OK;
  }

  if ((uc & 0xf0) == 0xf0) {
    ulong_t len = 0xf - (uc & 0x0f) + 1;

    // 65-128 bit ints not handled yet. oops.
    if (len > 8) {
      return ERR_DECVOM;
    }

    // check that there is enough data to read (we already read data[0]
    // into uc, above, thus len+1.
    if (dec->left.len < len+1) {
      return ERR_DECVOM_MORE;
    }

    uint64_t v0 = 0;
    for (ulong_t i = 0; i < len; i++) {
      v0 <<= 8;
      v0 |= dec->left.buf[i+1];
    }
    *v = v0;
    eat(dec, len+1);
    return ERR_OK;
  }

  // it is a control point
  eat(dec, 1);
  *ctl = uc;
  switch (uc) {
  case 0xe0:
  case 0xe1:
    break;
  default:
    // unknown control
    return ERR_DECVOM;
  }
  return ERR_OK;
}

static bool tidBuiltin(int64_t tid) {
  if (tid >= 1 && tid <= 15) {
    return true;
  }
  if (tid == 39 || tid == 40) {
    return true;
  }
  return false;
}

static err_t valueNextCell(value_t *v, ulong_t *cellp) {
  ulong_t newLen = v->buf.len+sizeof(cell_t);
  err_t err = bufExpand(&v->buf, newLen); ck();
  *cellp = (newLen/sizeof(cell_t))-1;
  v->buf.len = newLen;
  return ERR_OK;
}

cell_t *valueGetCell(value_t *v, ulong_t cell) {
  return (cell_t *)(uintptr_t)&v->buf.buf[cell * sizeof(cell_t)];
}

static int64_t unpackSign(uint64_t in) {
  if ((in & 1) == 0) {
    return in>>1;
  }
  return ~(in>>1);
}

err_t vomDecode(struct decoder *dec, value_t *vout, bool *done) {
  vomControl ctl = controlNone;
  err_t err = ERR_OK;

  // First time we are called with new input.
  if (dec->left.buf == NULL) {
    dec->left = dec->in;

    // Check the version.
    if (dec->left.len != 0 && dec->left.buf[0] != vomVersion81) {
      return ERR_DECVOM;
    }
    dec->version = vomVersion81;
    dec->curTid = 0;
    eat(dec, 1);
  }

  while (dec->left.len) {
    if (dec->curTid == 0) {
      uint64_t tid;
      err = readVar128(dec, &tid, &ctl); ck();
      if (ctl != controlNone) {
	if (ctl == controlEnd) {
	  return ERR_OK;
	}
      
	// unexpected control message here
	return ERR_DECVOM;
      }

      dec->curTid = unpackSign(tid);
      // go do the loop again, now that we have our tid
      continue;
    }

    if (tidBuiltin(dec->curTid)) {
      ulong_t cell;
      err = valueNextCell(vout, &cell); ck();
      cell_t *c = valueGetCell(vout, cell);
      c->ctype = (uint64_t)dec->curTid;
      
      uint64_t val;
      unsigned char b;
      switch (dec->curTid) {
      case tidBool:
	err = readByte(dec, &b); ck();
	c->u.Bool = (b == 1);
	break;
      case tidByte:
	err = readByte(dec, &b); ck();
	c->u.Byte = b;
	break;	
      case tidInt64:
	err = readVar128(dec, &val, &ctl); ck();
	if (ctl != controlNone) {
	  return ERR_DECVOM;
	}
	c->u.Int64 = unpackSign(val);
	break;
      default:
	return ERR_DECVOM;
      }

      // the only way to get here is by sucessfully decoding a
      // builtin. So congrats, we're done.
      *done = true;
      return ERR_OK;
    }
    
    // it is a type defn or a user-defined type, we don't do those yet
    return ERR_DECVOM;
  }
  
  // out of data to process
  return ERR_DECVOM_MORE;
}

void decoderDealloc(struct decoder *dec) {
  bufDealloc(&dec->in);
  dec->left.buf = NULL;
  dec->left.len = 0;
}


void valueZero(value_t *v) {
  if (v->buf.buf != NULL) {
    bufTruncate(&v->buf);
  }
}

err_t decoderFeed(struct decoder *dec, buf_t in) {
  return bufAppend(&dec->in, in);
}
