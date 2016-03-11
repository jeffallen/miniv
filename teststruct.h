// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// This file is included only when compiling via "go test".
// It is not included in libminiv.a.

#include "miniv.h"

struct inner {
  buf_t String;
};
  
struct testStruct {
  int A;
  double B;
  struct inner Inner;
};
  
void testStruct_register();
