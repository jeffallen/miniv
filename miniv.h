// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#pragma once

typedef int err_t;
const char *errstr(err_t err);

enum {
  ERR_OK,      // No error.
  ERR,         // Unspecified error.
  ERR_PARAM,   // Missing required param to a function.
  ERR_BADMSG,  // Failed to decode a message.
  ERR_VARINT,  // Failed to decode a varint.
  ERR_SETUPOPTION, // Failed to read a setup option.
  ERR_MEM,     // Out of memory.
  ERR_SOCKET,  // Cannot open socket.
  ERR_HOSTNAME, // Cannot find the hostname.
  ERR_ENDPOINT, // Could not parse endpoint.
  ERR_SOCKETCONNECT, // Could not connect.
  ERR_CONNECTION, // Connection broken.
};

#include "buf.h"
#include "connection.h"
#include "framer.h"
#include "endpoint.h"
#include "message.h"
