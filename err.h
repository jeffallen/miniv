// Copyright 2015 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

typedef int err_t;

enum {
  ERR_OK,      // 0: No error.
  ERR,         // 1: Unspecified error.
  ERR_PARAM,   // 2: Missing required param to a function.
  ERR_BADMSG,  // 3: Failed to decode a message.
  ERR_VARINT,  // 4: Failed to decode a varint.
  ERR_SETUPOPTION, // 5: Failed to read a setup option.
  ERR_MEM      // 6: Out of memory.
};
