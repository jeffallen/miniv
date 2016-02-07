// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "miniv.h"

#include <stdio.h>
#include <stdlib.h>
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

  connectionHandshake(c);
  
  while (1) {
    struct Message m;
    ckerr("read frame", connectionReadFrame(c));
    ckerr("decode message", messageRead(c->frame, &m));
    printf(" -> %s\n", messageTypeName(m.mtype));
    if (m.mtype == Setup) {
      printf(" public key: \n");
      buf_t key = { .buf = m.u.Setup.PeerNaClPublicKey, .len = 32 };
      bufDump(key);
    }
  }
}

