// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#pragma once

#include "miniv.h"

typedef struct {
  unsigned char *buf;
  ulong_t len;
  ulong_t cap;
} buf_t;

void bufDump(const char *what, const buf_t buf);

// bufFree deallocates the underlying buffer and sets
// len and cap to 0. The buf_t remains allocated, it is the
// caller's responsibility to deallocate it, if it came from
// the heap.
void bufDealloc(buf_t *buf);

// bufExpand ensures that the capacity of the buffer is at least len.
// If it needs to reallocate the underlying storage, it does so, copying
// b->len bytes into the new buffer.
err_t bufExpand(buf_t *b, ulong_t len);

// bufRemaining returns the number of bytes between cap and len.
ulong_t bufRemaining(buf_t *b);

buf_t bufFromString(const char *str);
buf_t bufWrap(void *p, ulong_t len);

// bufAppend appends in onto the end of the buffer pointed to by b.
// It will expand the buffer if necessary to make space.
err_t bufAppend(buf_t *b, const buf_t in);

// bufAppend appends a single character x onto the end of the
// buffer pointed to by b. It will expand the buffer if necessary
// to make space.
err_t bufAppendChar(buf_t *b, unsigned char x);

// bufTruncate sets the length to 0. It does not reduce capacity,
// deallocate storage, or zero the underlying buffer.
void bufTruncate(buf_t *b);

// bufCopy puts into dest a copy of src.
err_t bufCopy(buf_t *dest, buf_t src);

typedef struct {
  buf_t buf;
  ulong_t itemSize;
} queue_t;

// Initialize a queue to receive items of itemSize. All current entries
// in the queue are dropped.
void queueInit(queue_t *q, ulong_t itemSize);

void queueDealloc(queue_t *q);

// queueAppend appends the bytes p[0]..p[q->itemSize-1] into the queue.
err_t queueAppend(queue_t *q, const unsigned char *p);
