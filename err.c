// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdio.h>
#include "miniv.h"

// This table is generated from err.h by hand using:
//   awk -F// '/  ERR/ { err=$1; msg=$2;
//     sub(/,/, "", err); sub(/^ /, "", msg);
//     printf("[%s] = \"%s\",\n", err, msg)}' < err.h

static const char *errstrs[] = {
[  ERR_OK      ] = "No error.",
[  ERR         ] = "Unspecified error.",
[  ERR_PARAM   ] = "Missing required param to a function.",
[  ERR_BADMSG  ] = "Failed to decode a message.",
[  ERR_VARINT  ] = "Failed to decode a varint.",
[  ERR_SETUPOPTION ] = "Failed to read a setup option.",
[  ERR_MEM     ] = "Out of memory.",
[  ERR_SOCKET  ] = "Cannot open socket.",
[  ERR_HOSTNAME ] = "Cannot find the hostname.",
[  ERR_ENDPOINT ] = "Could not parse endpoint.",
[  ERR_SOCKETCONNECT ] = "Could not connect.",
[  ERR_CONNECTION ] = "Connection broken.",
};
static const int numerrs = sizeof(errstrs)/sizeof(const char *);

const char *errstr(err_t err) {
  if (err >= 0 && err < numerrs) {
    return errstrs[err];
  }
  
  static char buf[250];
  sprintf(buf, "unknown err %d", err);
  return buf;
}


  
