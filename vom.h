// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "miniv.h"
#pragma once
#include <sys/types.h>

typedef enum {
  vomVersion81 = 0x81,
} vomVersion;

// These are the primitive type id's, from the VOM spec.
typedef enum {
  tidBool = 1,
  tidByte = 2,
  tidInt64 = 9,
  tidString = 40,
} vomType;

typedef enum {
  controlNone = 0,
  controlNil = 0xe0,
  controlEnd = 0xe1,
} vomControl;

typedef struct {
  buf_t buf;
  void *ptr;
} value_t;

// TODO: dynamically allocate typedefn table
enum {
  utidBase = 41,
  utidMax = 100,
};

struct utype {
  buf_t tname;
  ssize_t sz;
  err_t (*decoder)(buf_t, void *);
};

typedef struct {
  buf_t in;
  buf_t left;  // a slice on in showing what's left to be processed. Do not dealloc.
  vomVersion version;
  int64_t curTid; // 0 means "not known yet"
  struct utype *utypes[utidMax];
} decoder_t;

// vomDecode decodes the VOM previously fed into the decoder into the vom_t vout.
// vout must be zeroed before using vomZero. On ERR_DEC_MORE, feed more input
// into dec and call vomDecode again.
err_t vomDecode(decoder_t *dec, value_t *vout, bool *done);

void decoderDealloc(decoder_t *dec);

// decoderFeed copies bytes into the decoder's buffer.
err_t decoderFeed(decoder_t *dec, buf_t in);

// valueZero ensures that the vom_t is ready to be filled with a result.
// It does not deallocate underlying storage. For that, see valueDealloc.
void valueZero(value_t *v);

// vomRegister registers a given type, so that vomDecode will
// know which callbacks to use to encode and decode it.
err_t vomRegister(buf_t name, ssize_t sz, err_t (*decoder)(buf_t, void *));

// Here are all the primitive decoders that will be called by
// the structure decoders.

err_t decodeByte(buf_t *in, unsigned char *v);
err_t decodeString(buf_t *in, buf_t *v);
err_t decodeInt(buf_t *in, int *v);
err_t decodeDouble(buf_t *in, double *v);
err_t decodeVar128(buf_t *in, uint64_t *v, vomControl *ctl);

