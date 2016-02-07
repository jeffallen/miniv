// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#pragma once

#include "miniv.h"

typedef struct {
  unsigned char *buf;
  unsigned long len;
  unsigned long cap;
} buf_t;

void bufDump(const buf_t buf);

// bufFree deallocates the underlying buffer and sets
// len and cap to 0. The buf_t remains allocated, it is the
// caller's responsibility to deallocate it, if it came from
// the heap.
void bufDealloc(buf_t *buf);

err_t bufExpand(buf_t *b, unsigned long len);
unsigned long bufRemaining(buf_t *b);
buf_t bufFromString(const char *str);

// bufAppend appends in onto the end of the buffer pointed to by b.
// It will expand the buffer if necessary to make space.
err_t bufAppend(buf_t *b, const buf_t in);

// bufAppend appends a single character x onto the end of the
// buffer pointed to by b. It will expand the buffer if necessary
// to make space.
err_t bufAppendChar(buf_t *b, unsigned char x);
