// Copyright 2015 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "miniv.h"
#pragma once

#include <stdint.h>

typedef enum {
  lowestMessageType = 0x71,  // marker, used by messageType()

  // These are defined by the protocol.
  HealthCheckReply = 0x72,
  HealthCheckRequest,
  Data, Release, OpenFlow, Auth,
  AckLameDuck,EnterLameDuck, TearDown, Setup,
  
  highestMessageType,  // marker, used by messageType()

  // This is not defined by the protocol. Used internally.
  Unknown,
} MessageType;

typedef enum {
  peerNaClPublicKeyOption = 1,
  peerRemoteEndpointOption,
  peerLocalEndpointOption,
  mtuOption,
  sharedTokensOption
} SetupOption;

struct Setup {
  uint32_t ver_min, ver_max;
  unsigned char PeerNaClPublicKey[32];
  uint64_t mtu;
  uint64_t sharedTokens;
};

struct Message {
  MessageType mtype;
  union {
    struct Setup Setup;
  } u;
};

struct Message *messageNew(void);
err_t messageRead(const unsigned char *in, uint64_t len, struct Message *m);
err_t messageAppend(struct Message *m, buf_t *to);
