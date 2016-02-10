// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#pragma once

#include "miniv.h"

struct Signature {
  // Purpose of the signature. Can be used to prevent type attacks.
  // (See Section 4.2 of http://www-users.cs.york.ac.uk/~jac/PublishedPapers/reviewV1_1997.pdf for example).
  // The actual signature (R, S values for ECDSA keys) is produced by signing: Hash(Hash(message), Hash(Purpose)).
  buf_t Purpose;
  const char *Hash;
  buf_t R, S; // Pair of integers that make up an ECDSA signature.
};

extern const char *SignatureSHA256;
void signatureDealloc(struct Signature *s);
err_t signatureSetHash(struct Signature *s, buf_t hash);
