// Copyright 2015 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdlib.h>
#include <strings.h>

#include "miniv.h"

// Find the host:port in a Vanadium endpoint like this;
// @6@wsh@192.168.0.107:64994@@85b4dbc6d23fa02d98b74c9008c3f1b0@l@dev.v.io:u:jra@nella.org:bridge@@
err_t nameParse(char *name, char **host, char **port) {
  if (name == NULL) {
    return ERR_PARAM;
  }

  char *p = name;
  int ct = 0;
  while (*p) {
    if (ct == 3) {
      // Found hostname, terminate it.
      *host = p;
      p = index(p, ':');
      if (p == NULL) {
	break;
      }
      *p = 0;
      p++;
      *port = p;
      p = index(p, '@');
      if (p == NULL) {
	break;
      }
      *p = 0;      
      return ERR_OK;
    }

    if (*p == '@') {
      ct++;
    }
    p++;
  }
  
  return ERR_ENDPOINT;
}

