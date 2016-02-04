// Copyright 2015 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "message.h"

#include <stdlib.h>
#include <string.h>

// for debug
//#include <stdio.h>

MessageType messageType(unsigned char in) {
  switch (in) {
    case 0x7e:
      return Setup;
    case 0x7d:
      return TearDown;
    case 0x7c:
      return EnterLameDuck;
    case 0x7b:
      return AckLameDuck;
    case 0x7a:
      return Auth;
    case 0x79:
      return OpenFlow;
    case 0x78:
      return Release;
    case 0x77:
      return Data;
    case 0x73:
      return HealthCheckRequest;
    case 0x72:
      return HealthCheckReply;
    default:
      return Unknown;
    }
}

struct Message *messageNew() {
  return calloc(sizeof(struct Message), 1);
}

// Reads a varuint64 off of the front of the buffer described by data/len
// and updates the caller's data/len to indicate the data has been consumed.
err_t readVarUint64(const unsigned char **data, uint64_t *len, uint64_t *v) {
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

err_t readLenBytes(const unsigned char **data, uint64_t *len,
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

  memcpy((void *)*payload, *data, *plen);
  *data = *data + *plen;
  *len = *len - *plen;
  
  return ERR_OK;
}

err_t readSetupOption(const unsigned char **data, uint64_t *len,
		      uint64_t *opt, const unsigned char **payload, uint64_t *plen) {
  err_t err = readVarUint64(data, len, opt);
  if (err != ERR_OK) {
    return err;
  }
  err = readLenBytes(data, len, payload, plen);
  return err;
}

err_t messageReadSetup(const unsigned char *data, uint64_t len, struct Setup *s) {
  uint64_t v;
  err_t err = readVarUint64(&data, &len, &v);
  if (err != ERR_OK) {
      return err;
  }
  s->ver_min = v;

  err = readVarUint64(&data, &len, &v);
  if (err != ERR_OK) {
      return err;
  }
  s->ver_max = v;

  while (err == ERR_OK && len > 0) {
    uint64_t o64;
    
    const unsigned char *payload = NULL;
    uint64_t plen;
    
    err_t err = readSetupOption(&data, &len, &o64, &payload, &plen);
    if (err != ERR_OK) {
      return err;
    }

    SetupOption opt = o64;
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
    default:
      // Unhandled options are silently dropped for now.
      break;
    }
  err:  
    free((void *)payload);
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

