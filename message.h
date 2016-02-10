// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdint.h>
#include "miniv.h"
#include "signature.h"
#pragma once

typedef enum {
  lowestMessageType = 0x74,  // marker, used by messageType()

  // These are defined by the protocol.
  HealthCheckReply = 0x75,
  HealthCheckRequest = 0x76,
  Data = 0x77,
  Release = 0x78,
  OpenFlow = 0x79,
  Auth = 0x7a,
  AckLameDuck = 0x7b,
  EnterLameDuck = 0x7c,
  TearDown = 0x7d,
  Setup = 0x7e,
  
  Invalid = 0x7f,  // marker, used by messageType()
} MessageType;

typedef enum {
  peerNaClPublicKeyOption = 1,
  peerRemoteEndpointOption,
  peerLocalEndpointOption,
  mtuOption,
  sharedTokensOption
} SetupOption;

enum {
  RPC_VER_DEFAULT = 14
};

struct Setup {
  uint32_t ver_min, ver_max;
  unsigned char PeerNaClPublicKey[32];
  char *PeerRemoteEndpoint, *PeerLocalEndpoint;
  uint64_t mtu;
  uint64_t sharedTokens;
};

struct Data {
  uint64_t id;
  uint64_t flags;
  buf_t payload;
};

struct Auth {
  uint64_t BlessingsKey;
  uint64_t DischargeKey;
  struct Signature ChannelBinding;
};

struct Message {
  MessageType mtype;
  union {
    struct Setup Setup;
    struct Data Data;
    struct Auth Auth;
  } u;
};

// messageNew returns a new message, allocted from the heap. It is zeroed.
// Once filled via messageRead, it must be deallocated using messageFree, then
// the struct Message itself must be freed using free().
struct Message *messageNew(void);

// messageFree frees any allocated storage associated with the message. The
// struct Message itself is not freed, and must be freed by the caller (or not
// freed, if for example it was allocated on the stack).
void messageFree(struct Message *);

const char *messageTypeName(MessageType m);

// messageRead deserializes a message out of buffer in into the structure pointed
// to by m. The structure MUST be zero-valued before the call.
err_t messageRead(const buf_t in, struct Message *m);

// messageAppend serializes a message onto the end of buffer to.
err_t messageAppend(struct Message *m, buf_t *to);
