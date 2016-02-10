// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdlib.h>
#include <string.h>

#include "miniv.h"

const char *messageTypeName(MessageType m) {
  static const char *names[] = {
    "HealthCheckReply", "HealthCheckRequest", "Data", "Release",
    "OpenFlow", "Auth", "AckLameDuck", "EnterLameDuck", "TearDown",
    "Setup",
  };

  if (m > lowestMessageType && m < Invalid) {
    return names[m-lowestMessageType-1];
  }
  return "Invalid";
}

static MessageType messageType(unsigned char in) {
  MessageType m = in;
  if (in > lowestMessageType && in < Invalid) {
    return m;
  }
  return Invalid;
}

struct Message *messageNew() {
  return calloc(sizeof(struct Message), 1);
}

void messageFree(struct Message *m) {
  switch (m->mtype) {
  case Setup:
    free(m->u.Setup.PeerRemoteEndpoint);
    free(m->u.Setup.PeerLocalEndpoint);
    break;
  case Data:
    bufDealloc(&m->u.Data.payload);
    break;
  case Auth:
    signatureDealloc(&m->u.Auth.ChannelBinding);
    break;
  default:
    break;
  }
}

// Reads a varuint64 off of the front of the buffer described by data/len
// and updates the caller's data/len to indicate the data has been consumed.
static err_t readVarUint64(buf_t *in, uint64_t *v) {
  if (in == NULL) {
    return ERR_PARAM;
  }
  if (in->len == 0) {
    return ERR_VARINT;
  }
  
  unsigned char l = in->buf[0];
  if (l <= 0x7f) {
      *v = l;
      in->len--;
      in->buf++;
      return ERR_OK;
  }
    
  l = 0xff - l + 1;
  if (l > 8 || in->len-1 < l) {
    return ERR_VARINT;
  }

  const unsigned char *p = in->buf;
  for (int i = 1; i < l+1; i++) {
    *v = (*v << 8 | (uint64_t)p[i]);
    in->len--;
    in->buf++;
  }

  return ERR_OK;
}

static err_t readLenBytes(buf_t *in, buf_t *payload) {
  if (in == NULL || payload == NULL) {
    return ERR_PARAM;
  }

  uint64_t plen;
  err_t err = readVarUint64(in, &plen);
  if (err != ERR_OK) {
    return err;
  }

  if (in->len < plen) {
    return ERR_SETUPOPTION;
  }

  // We reuse existing buf if it was already malloced.
  bufCopy(payload, bufWrap(in->buf, plen));

  in->buf += payload->len;
  in->len -= payload->len;

  return ERR_OK;
}

static err_t readSetupOption(buf_t *in, uint64_t *opt, buf_t *payload) {
  err_t err = readVarUint64(in, opt);
  if (err != ERR_OK) {
    return err;
  }
  err = readLenBytes(in, payload);
  return err;
}

// by the time we are called, buf_t in is already a copy and can be modified.
static err_t messageReadSetup(buf_t in, struct Setup *s) {
  uint64_t v;
  err_t err = readVarUint64(&in, &v);
  if (err != ERR_OK) {
      return err;
  }
  s->ver_min = (uint32_t)v;

  err = readVarUint64(&in, &v);
  if (err != ERR_OK) {
      return err;
  }
  s->ver_max = (uint32_t)v;

  while (err == ERR_OK && in.len > 0) {
    uint64_t o64;
    buf_t payload = {};
    
    err = readSetupOption(&in, &o64, &payload);
    if (err != ERR_OK) {
      // goto due to need to cleanup payload. (I miss defer.)
      goto err;
    }

    SetupOption opt = (uint32_t)o64;
    switch (opt) {
    case peerNaClPublicKeyOption:
      if (payload.len != 32) {
	err = ERR_SETUPOPTION;
	goto err;
      }
      memcpy(s->PeerNaClPublicKey, payload.buf, 32);
      break;
    case mtuOption:
      {
	buf_t mtu = payload;
	err = readVarUint64(&mtu, &s->mtu);
	if (err != ERR_OK) {
	  goto err;
	}
	break;
      }
    case sharedTokensOption:
      {
	// so that payload does not get incremented, making it impossible to free
	buf_t tokens = payload;
	err = readVarUint64(&tokens, &s->sharedTokens);
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
    bufDealloc(&payload);
  }

  return err;
}

static err_t messageReadData(buf_t in, struct Data *s) {
  err_t err = readVarUint64(&in, &s->id);
  if (err != ERR_OK) {
      return err;
  }
  err = readVarUint64(&in, &s->flags);
  if (err != ERR_OK) {
      return err;
  }
  bufCopy(&s->payload, in);
  return ERR_OK;
}

static err_t messageReadAuth(buf_t in, struct Auth *a) {
  err_t err = readVarUint64(&in, &a->BlessingsKey);
  if (err != ERR_OK) {
      return err;
  }
  err = readVarUint64(&in, &a->DischargeKey);
  if (err != ERR_OK) {
      return err;
  }
  readLenBytes(&in, &a->ChannelBinding.Purpose); ck();
  
  buf_t hash = {};
  readLenBytes(&in, &hash); ck();
  err = signatureSetHash(&a->ChannelBinding, hash); ck();
  bufDealloc(&hash);
  
  readLenBytes(&in, &a->ChannelBinding.R); ck();
  readLenBytes(&in, &a->ChannelBinding.S); ck();
  return ERR_OK;
}

err_t messageRead(const buf_t in, struct Message *m) {
  if (!m || in.len < 1) {
    return ERR_PARAM;
  }

  // take a copy to work on (the underlying buffer it shared,
  // just the pointer into it will be modified)
  buf_t b = in;

  m->mtype = messageType(b.buf[0]);
  b.len--;
  b.buf++;
  
  switch (m->mtype) {
  case Setup:
    return messageReadSetup(b, &m->u.Setup);
  case Data:
    return messageReadData(b, &m->u.Data);
  case Auth:
    return messageReadAuth(b, &m->u.Auth);
  default:
      return ERR_OK;
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
  for (; shift >= 0; shift -= 8) {
    bufAppendChar(to, (u>>shift & 0xff));
  }
  return ERR_OK;
}

static err_t appendLenBytes(const buf_t data, buf_t *to) {
  err_t err = writeVarUint64(to, data.len); ck();
  err = bufAppend(to, data);
  return err;
}
  
static err_t appendSetupOption(SetupOption opt, const buf_t data, buf_t *to) {
  err_t err = writeVarUint64(to, opt); ck();
  err = appendLenBytes(data, to);
  return err;
}

static err_t messageAppendSetup(struct Setup *s, buf_t *to) {
  err_t err = bufAppendChar(to, Setup); ck();
  err = writeVarUint64(to, s->ver_min); ck();
  err = writeVarUint64(to, s->ver_max); ck();

  // in Vanadium, encoding these is optional, but since we
  // only implement RPC v14, I think we'll always have it.
  err = appendSetupOption(peerNaClPublicKeyOption,
			  bufWrap(s->PeerNaClPublicKey, 32), to); ck();

  if (s->PeerRemoteEndpoint) {
    err = appendSetupOption(peerRemoteEndpointOption,
			    bufFromString(s->PeerRemoteEndpoint), to); ck();
  }
  if (s->PeerLocalEndpoint) {
    err = appendSetupOption(peerLocalEndpointOption,
			    bufFromString(s->PeerLocalEndpoint), to); ck();
  }
  
  if (s->mtu != 0) {
    unsigned char space[9];
    buf_t tmp = { .buf = space, .len = 0, .cap = sizeof(space) };
    err = writeVarUint64(&tmp, s->mtu); ck();
    err = appendSetupOption(mtuOption, tmp, to); ck();
  }
  if (s->sharedTokens != 0) {
    unsigned char space[9];
    buf_t tmp = { .buf = space, .len = 0, .cap = sizeof(space) };
    err = writeVarUint64(&tmp, s->sharedTokens); ck();
    err = appendSetupOption(sharedTokensOption, tmp, to); ck();
  }
  // uninterpreted options not stored yet
  
  return err;
}

err_t messageAppend(struct Message *m, buf_t *to) {
  if (!m || !to) {
    return ERR_PARAM;
  }

  switch (m->mtype) {
  case Setup:
    return messageAppendSetup(&m->u.Setup, to);
  default:
    return ERR_OK;
  }
}

