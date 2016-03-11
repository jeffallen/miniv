// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "miniv.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef __linux__
// to get strdup on linux (for the love of God, why?)
#define __USE_BSD
#endif
#include <string.h>
#include <unistd.h>

static void ckerr(const char *what, err_t err) {
  if (err != ERR_OK) {
    fprintf(stderr, "Error %s: %s\n", what, errstr(err));
    exit(1);
  }
}

int main(int argc, char **argv) {
  struct connection c0 = {};
  struct connection *c = &c0;
  connectionNew(c);
  
  if (argc != 2) {
    fprintf(stderr, "usage: %s endpoint-name\n", argv[0]);
    exit(1);
  }

  char *name = strdup(argv[1]);
  char *host, *portstr;
  
  // attention: nameParse mutates name.
  ckerr("parsing name", nameParse(name, &host, &portstr));
  uint16_t port = (uint16_t)atoi(portstr);

  printf("Connecting to %s:%d\n", host, port);
  ckerr("connecting", connectionOpen(host, port, c));
  
  host = NULL; portstr = NULL; free(name);

  ckerr("handshake:", connectionHandshake(c));
  
  while (1) {
    struct Message m = {};
    ckerr("read frame", connectionReadFrame(c));
    ckerr("decode message", messageRead(c->frame, &m));
    printf(" -> (%x)%s\n", m.mtype, messageTypeName(m.mtype));
    switch (m.mtype) {
    case Auth:
      printf("    blessings: %lld discharge: %lld\n", m.u.Auth.BlessingsKey, m.u.Auth.DischargeKey);
      bufDump("    signature Purpose:", m.u.Auth.ChannelBinding.Purpose);
      bufDump("    signature R:", m.u.Auth.ChannelBinding.R);
      bufDump("    signature S:", m.u.Auth.ChannelBinding.S);
      break;
    case Data:
      printf("    ID: %lld\n", m.u.Data.id);
      printf("    flags: %lld\n", m.u.Data.flags);
      bufDump("    payload:", m.u.Data.payload);
      break;
    default:
      break;
    }
  }
}

