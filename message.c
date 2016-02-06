// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdlib.h>
#include <string.h>

#include "miniv.h"

// for debug
//#include <stdio.h>

static MessageType messageType(unsigned char in) {
  MessageType m = in;
  if (in > lowestMessageType && in < highestMessageType) {
    return m;
  }
  return Unknown;
}

struct Message *messageNew() {
  return calloc(sizeof(struct Message), 1);
}

// Reads a varuint64 off of the front of the buffer described by data/len
// and updates the caller's data/len to indicate the data has been consumed.
static err_t readVarUint64(const unsigned char **data, uint64_t *len, uint64_t *v) {
  if (data == NULL || len == NULL) {
    return ERR_PARAM;
  }
  if (*len == 0) {
    return ERR_VARINT;
  }
  
  unsigned char l = *data[0];
  if (l <= 0x7f) {
      *v = l;
      (*len)--;
      (*data)++;
      return ERR_OK;
  }
    
  l = 0xff - l + 1;
  if (l > 8 || *len-1 < l) {
    return ERR_VARINT;
  }

  const unsigned char *p = *data;
  for (int i = 1; i < l+1; i++) {
    *v = (*v << 8 | (uint64_t)p[i]);
    (*len)--;
    (*data)++;
  }

  return ERR_OK;
}

static err_t readLenBytes(const unsigned char **data, uint64_t *len,
		   const unsigned char **payload, uint64_t *plen) {
  if (data == NULL || len == NULL || payload == NULL || plen == NULL) {
    return ERR_PARAM;
  }
  
  err_t err = readVarUint64(data, len, plen);
  if (err != ERR_OK) {
    return err;
  }

  if (*plen == 0) {
    *payload = NULL;
    return ERR_OK;
  }
  
  if ((uint64_t)len < *plen) {
    return ERR_SETUPOPTION;
  }
  
  *payload = malloc(*plen);
  if (! *payload) {
    return ERR_MEM;
  }

  /* dump payload */
  /*
  for (int i = 0; i < *plen; i++) {
    printf("%x ", (*data)[i]);
  }
  printf("\n");
  */

  memcpy((void *)(uintptr_t)*payload, *data, *plen);
  *data = *data + *plen;
  *len = *len - *plen;
  
  return ERR_OK;
}

static err_t readSetupOption(const unsigned char **data, uint64_t *len,
		      uint64_t *opt, const unsigned char **payload, uint64_t *plen) {
  err_t err = readVarUint64(data, len, opt);
  if (err != ERR_OK) {
    return err;
  }
  err = readLenBytes(data, len, payload, plen);
  return err;
}

static err_t messageReadSetup(const unsigned char *data, uint64_t len, struct Setup *s) {
  uint64_t v;
  err_t err = readVarUint64(&data, &len, &v);
  if (err != ERR_OK) {
      return err;
  }
  s->ver_min = (uint32_t)v;

  err = readVarUint64(&data, &len, &v);
  if (err != ERR_OK) {
      return err;
  }
  s->ver_max = (uint32_t)v;

  while (err == ERR_OK && len > 0) {
    uint64_t o64;
    
    const unsigned char *payload = NULL;
    uint64_t plen;
    
    err = readSetupOption(&data, &len, &o64, &payload, &plen);
    if (err != ERR_OK) {
      return err;
    }

    SetupOption opt = (uint32_t)o64;
    switch (opt) {
    case peerNaClPublicKeyOption:
      if (plen != 32) {
	err = ERR_SETUPOPTION;
	goto err;
      }
      memcpy(s->PeerNaClPublicKey, payload, 32);
      break;
    case mtuOption:
      {
	// so that payload does not get incremented, making free() fail
	const unsigned char *tmp = payload;
	err = readVarUint64(&tmp, &plen, &s->mtu);
	if (err != ERR_OK) {
	  goto err;
	}
	break;
      }
    case sharedTokensOption:
      {
	// so that payload does not get incremented, making free() fail
	const unsigned char *tmp = payload;
	err = readVarUint64(&tmp, &plen, &s->sharedTokens);
	if (err != ERR_OK) {
	  goto err;
	}
	break;
      }
    case peerRemoteEndpointOption:
    case peerLocalEndpointOption:
    default:
      // Unhandled options are silently dropped for now.
      break;
    }
  err:  
    free((void*)(uintptr_t)payload);
  }

  return err;
}

err_t messageRead(const unsigned char *data, uint64_t len, struct Message *m) {
  if (!m || len < 1) {
    return ERR_PARAM;
  }

  m->mtype = messageType(data[0]);
  len--;
  data++;
  
  switch (m->mtype) {
  case Setup:
    return messageReadSetup(data, len, &m->u.Setup);
  default:
      return ERR_BADMSG;
    }
}

static err_t writeVarUint64(buf_t *to, uint64_t u) {
  // Since writeVarUint64 is often called one after another, when some is needed,
  // add lots.
  if (bufRemaining(to) < 9) {
    err_t err = bufExpand(to, 64);
    if (err != ERR_OK) {
      return err;
    }
  }
  
  if (u <= 0x7f) {
      return bufAppendChar(to, (unsigned char)u);
  }
  int shift = 56;
  unsigned char l = 7;
  for (; shift >= 0 && ((u>>shift)&0xff) == 0; shift -= 8, l--) {
  }
  bufAppendChar(to, 0xff-l);
  for (; shift >= 0; shift--) {
    bufAppendChar(to, (u>>shift & 0xff));
  }
  return ERR_OK;
}

#define ck(x) do { err_t err = (x); if (err != ERR_OK) return err; } while (1)
static err_t messageAppendSetup(struct Setup *s, buf_t *to) {
  ck(bufAppendChar(to, Setup));
  ck(writeVarUint64(to, s->ver_min));
  ck(writeVarUint64(to, s->ver_max));
  //return ERR_BADMSG;
}
#undef ck

err_t messageAppend(struct Message *m, buf_t *to) {
  if (!m || !to) {
    return ERR_PARAM;
  }

  switch (m->mtype) {
  case Setup:
    return messageAppendSetup(&m->u.Setup, to);
  default:
    return ERR_BADMSG;
  }
}

