// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "miniv.h"
#include <stdio.h>

static struct utype knownUserTypes[utidMax];
static int nKnownUserTypes = 0;

static int64_t decodeSign(uint64_t in) {
  if ((in & 1) == 0) {
    return in>>1;
  }
  return ~(in>>1);
}

static void eat(buf_t *in, ulong_t n) {
  /*
  char buf[100];
  snprintf(buf, sizeof(buf), "eat %lu", n);
  bufDump(buf, *in);
  */

  if (n > in->len) {
    // Do not allow underflow
    printf("underflow!\n");
    in->buf += in->len;
    in->len = 0;
  } else {
    in->buf += n;
    in->len -= n;
  }
}

err_t decodeString(buf_t *in, buf_t *v) {
  vomControl ctl = controlNone;
  uint64_t len;
  err_t err = decodeVar128(in, &len, &ctl); ck();
  if (ctl != controlNone) {
    // unexpected control message here
    err = ERR_DECVOM; ck();
  }
  
  v->buf = in->buf;
  v->len = len;
  // TODO: buf needs a flag system to say, "this is a slice, don't realloc/dealloc"
  v->cap = 0;

  eat(in, len);
  
  return ERR_OK;
}

err_t decodeInt(buf_t *in, int *v) {
  vomControl ctl = controlNone;
  uint64_t i;
  err_t err = decodeVar128(in, &i, &ctl); ck();
  if (ctl != controlNone) {
    // unexpected control message here
    err = ERR_DECVOM; ck();
  }
  *v = (int)decodeSign(i);
  return ERR_OK;
}

err_t decodeDouble(buf_t *in, double *v) {
  vomControl ctl = controlNone;
  uint64_t uval;
  err_t err = decodeVar128(in, &uval, &ctl); ck();
  if (ctl != controlNone) {
    // unexpected control message here
    err = ERR_DECVOM; ck();
  }

  uint64_t ieee = (uval&0xff)<<56 |
    (uval&0xff00)<<40 |
    (uval&0xff0000)<<24 |
    (uval&0xff000000)<<8 |
    (uval&0xff00000000)>>8 |
    (uval&0xff0000000000)>>24 |
    (uval&0xff000000000000)>>40 |
    (uval&0xff00000000000000)>>56;

  union {
    uint64_t u64;
    double d;
  } u;

  u.u64 = ieee;
  *v = u.d;
  
  return ERR_OK;
}

err_t decodeByte(buf_t *in, unsigned char *v) {
  if (in->len == 0) {
    return ERR_DECVOM;
  }
  *v = in->buf[0];
  eat(in, 1);
  return ERR_OK;
}

err_t decodeVar128(buf_t *in, uint64_t *v, vomControl *ctl) {
  *ctl = controlNone;
  
  if (in->len == 0) {
    return ERR_DECVOM;
  }

  unsigned char uc = in->buf[0];
  if (uc < 128) {
    *v = uc;
    eat(in, 1);
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
    if (in->len < len+1) {
      return ERR_DECVOM_MORE;
    }

    uint64_t v0 = 0;
    for (ulong_t i = 0; i < len; i++) {
      v0 <<= 8;
      v0 |= in->buf[i+1];
    }
    *v = v0;
    eat(in, len+1);
    return ERR_OK;
  }

  // it is a control point
  eat(in, 1);
  *v = 0;
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

static bool isPrimitive(int64_t tid) {
  if (tid >= 1 && tid <= 15) {
    return true;
  }
  if (tid == 39 || tid == 40) {
    return true;
  }
  return false;
}

static struct utype *findUt(buf_t name) {
  for (int i = 0; i < nKnownUserTypes; i++) {
    if (bufEqual(knownUserTypes[i].tname, name)) {
      return &knownUserTypes[i];
    }
  }
  return NULL;
}

static struct utype *lookupUt(int64_t tid) {
  int64_t i = tid - utidBase;
  if (i < 0 || i >= nKnownUserTypes) {
    return NULL;
  }
  return &knownUserTypes[i];
}

static err_t decodeType(decoder_t *dec) {
  vomControl ctl = controlNone;
  err_t err = ERR_OK;

  while (dec->left.len > 0) {
    uint64_t tlen;
    err = decodeVar128(&dec->left, &tlen, &ctl); ck();
    if (ctl != controlNone) {
      if (ctl == controlEnd) {
	return ERR_OK;
      }
      
      // unexpected control message here
      err = ERR_DECVOM; ck();
    }

    int64_t curTid = -1 * dec->curTid;
    if (curTid > utidMax) {
      err = ERR_DECVOM; ck();
    }

    // Hack our way through parsing the wireStruct to find the name
    // This is all trash that will get more elegant when I understand it better.

    if (dec->left.len < 2 ||
	dec->left.buf[0] != 6 ||
	dec->left.buf[1] != 0) {
      // why isn't there a 06 00 on the front? so mean!
      bufDump("no 06 00?", dec->left);
      err = ERR_DECVOM; ck();
    }
    
    buf_t lenbuf = dec->left;
    // eat the 06 00 off the front of it.
    lenbuf.buf += 2;
    lenbuf.len -= 2;
    
    uint64_t len;
    err = decodeVar128(&lenbuf, &len, &ctl); ck();
    if (ctl != controlNone) {
      // unexpected control message here
      err = ERR_DECVOM; ck();
    }
    
    buf_t name = lenbuf;
    name.len = len;
    dec->utypes[curTid - utidBase] = findUt(name);

    eat(&dec->left, tlen);
    return ERR_OK;      
  }

  return ERR_DECVOM_MORE;
}

err_t vomDecode(decoder_t *dec, value_t *vout, bool *done) {
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
    eat(&dec->left, 1);
  }

  while (dec->left.len > 0) {
    if (dec->curTid == 0) {
      uint64_t tid;
      err = decodeVar128(&dec->left, &tid, &ctl); ck();
      if (ctl != controlNone) {
	if (ctl == controlEnd) {
	  return ERR_OK;
	}
      
	// unexpected control message here
	return ERR_DECVOM;
      }

      dec->curTid = decodeSign(tid);
      // go do the loop again, now that we have our tid
      continue;
    }

    if (isPrimitive(dec->curTid)) {
      // with the current design, we shoud only be seeing primitives
      // in decoder callbacks.
      err = ERR_DECVOM; ck();
    }
    
    if (dec->curTid < 0) {
      err = decodeType(dec); ck();
      // back to the top of the while to see if there is a value to process now
      dec->curTid = 0;
      continue;
    }

    // it is a user-defined type. lookup and decode.

    struct utype *ut = lookupUt(dec->curTid);
    if (ut == NULL) {
      // unknown type
      err = ERR_DECVOM; ck();
    }

    uint64_t vlen;    
    err = decodeVar128(&dec->left, &vlen, &ctl); ck();
    if (ctl != controlNone) {
      // unexpected control message here
      err = ERR_DECVOM; ck();
    }
  
    buf_t valbuf = dec->left;
    valbuf.len = vlen;
    eat(&dec->left, vlen);

    // prepare the destination buffer: put the input that the
    // decoder will point back at in front, then put a place for
    // the structure itself at the end.
    bufExpand(&vout->buf, vlen + (uint64_t)ut->sz);
    vout->buf.len = 0;
    bufAppend(&vout->buf, valbuf);
    vout->ptr = &vout->buf.buf[vlen];

    err = ut->decoder(vout->buf, vout->ptr); ck();
    *done = true;
    return ERR_OK;
  }
  
  // out of data to process
  err = ERR_DECVOM_MORE; ck();
  return ERR_OK;
}

void decoderDealloc(decoder_t *dec) {
  bufDealloc(&dec->in);
  dec->left.buf = NULL;
  dec->left.len = 0;
}

void valueZero(value_t *v) {
  if (v->buf.buf != NULL) {
    bufTruncate(&v->buf);
  }
}

err_t decoderFeed(decoder_t *dec, buf_t in) {
  return bufAppend(&dec->in, in);
}

err_t vomRegister(buf_t tname, ssize_t sz, err_t (*decoder)(buf_t, void *)) {
  if (nKnownUserTypes >= utidMax) {
    return ERR_DECVOM;
  }

  knownUserTypes[nKnownUserTypes].tname = tname;
  knownUserTypes[nKnownUserTypes].sz = sz;
  knownUserTypes[nKnownUserTypes].decoder = decoder;
  
  nKnownUserTypes++;
  return ERR_OK;
}
