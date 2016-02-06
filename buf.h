// Copyright 2015 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#pragma once

#include "miniv.h"

typedef struct {
  unsigned char *buf;
  int len;
  int cap;
} buf_t;

void bufDump(buf_t *buf);
err_t bufExpand(buf_t *b, int len);
buf_t bufFromString(const char *str);
err_t bufAppend(buf_t *b, buf_t in);
