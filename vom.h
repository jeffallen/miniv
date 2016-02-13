// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "miniv.h"
#pragma once

typedef enum {
  vomVersion81 = 0x81,
} vomVersion;

// These are the built-in type id's, from the VOM spec.
typedef enum {
  tidBool = 1,
  tidByte = 2,
  tidInt64 = 9,
} vomType;

typedef enum {
  controlNone = 0,
  controlNil = 0xe0,
  controlEnd = 0xe1,
} vomControl;

typedef struct {
  buf_t buf;
} value_t;

typedef struct {
  uint64_t ctype;
  union {
    bool Bool;
    unsigned char Byte;
    int64_t Int64;
  } u;
} cell_t;

struct decoder {
  buf_t in;
  buf_t left;  // a slice on in showing what's left to be processed. Do not dealloc.
  vomVersion version;
  int64_t curTid;
};

// vomDecode decodes the VOM in "in" into the vom_t vout.
// vout must be zeroed before using vomZero.
err_t vomDecode(struct decoder *dec, value_t *vout, bool *done);

void decoderDealloc(struct decoder *dec);
err_t decoderFeed(struct decoder *dec, buf_t in);

// vomZero ensures that the vom_t is ready to be filled with a result.
// It does not deallocate underlying storage. For that, see vomDealloc.
void valueZero(value_t *v);

cell_t *valueGetCell(value_t *v, ulong_t cellnum);

