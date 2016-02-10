// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <string.h>
#include "miniv.h"

const char *SignatureSHA256 = "SHA256";

void signatureDealloc(struct Signature *s) {
  bufDealloc(&s->Purpose);
  bufDealloc(&s->R);
  bufDealloc(&s->S);
}

err_t signatureSetHash(struct Signature *s, buf_t hash) {
  if (hash.len != 6 || memcmp(hash.buf, SignatureSHA256, 4) != 0) {
    return ERR_HASH;
  }
  s->Hash = SignatureSHA256;
  return ERR_OK;
}
