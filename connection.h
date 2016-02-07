// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdint.h>

#include "miniv.h"
#pragma once

struct connection {
  buf_t frame;
  int fd;
};

err_t connectionEnsureFrame(struct connection *c, unsigned long len);
err_t connectionReadFrame(struct connection *c);
err_t connectionOpen(const char *hostname, uint16_t port, struct connection *c);
err_t connectionHandshake(struct connection *c);
