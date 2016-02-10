// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdint.h>
#include <stdbool.h>
#include "crypto_box.h"

#include "miniv.h"
#pragma once

struct stream {
  union {
    uint64_t intnonce[3];
    unsigned char nonce[crypto_box_NONCEBYTES];
  } u;
};

typedef enum {
  Dialer, Accepter
} Direction;

struct connection {
  int rfd, wfd;
  buf_t frame; // the final message; points to frame1.buf (unencrypted) or frame2.buf+mac. Do no dealloc!
  buf_t frame1;  // cipher
  buf_t frame2;  // zeroed mac + message
  unsigned char k[crypto_box_BEFORENMBYTES];
  unsigned char pk[crypto_box_PUBLICKEYBYTES];
  unsigned char sk[crypto_box_SECRETKEYBYTES];
  unsigned char remotepk[crypto_box_PUBLICKEYBYTES];
  buf_t binding;
  bool encrypt;
  struct stream enc;
  struct stream dec;
  Direction dir;
  queue_t q;
};

void connectionNew(struct connection *c);
err_t connectionReadFrame(struct connection *c);
err_t connectionWriteFrame(struct connection *c, buf_t out);
err_t connectionOpen(const char *hostname, uint16_t port, struct connection *c);

err_t connectionHandshake(struct connection *c);
