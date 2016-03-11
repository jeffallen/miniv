// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// This file is included only when compiling via "go test".
// It is not included in libminiv.a.

#include "miniv.h"

struct testStruct {
  int A;
  double B;
};
  
void testStruct_register();
