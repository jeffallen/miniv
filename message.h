// Copyright 2015 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "miniv.h"
#pragma once

#include <stdint.h>

typedef enum {
  Setup, TearDown, EnterLameDuck, AckLameDuck,
  Auth, OpenFlow, Release, Data,
  HealthCheckRequest, HealthCheckReply,
  Unknown
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

struct Message *messageNew();
err_t messageRead(const unsigned char *in, uint64_t len, struct Message *m);