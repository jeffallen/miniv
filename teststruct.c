// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// This file is included only when compiling via "go test".
// It is not included in libminiv.a.

// This file was hand-created, as a prototype for the files that
// will be made by the VDL tool.

#include <stdio.h>
#include <stdlib.h>

#include "miniv.h"
#include "teststruct.h"

static err_t inner_decode(buf_t *in, void *out) {
  struct inner *x = (struct inner*)out;

  vomControl ctl = controlNone;
  uint64_t index;

  while (true) {
    err_t err = decodeVar128(in, &index, &ctl); ck();
    if (ctl == controlEnd) {
      return ERR_OK;
    }
    if (ctl != controlNone) {
      // unexpected control message here
      err = ERR_DECVOM; ck();
    }

    switch (index) {
    case 0:
      err = decodeString(in, &x->String); ck();
      break;
    default:
      err = ERR_DECVOM; ck();      
    }
  }
}

static err_t testStruct_decode(buf_t *in, void *out) {
  struct testStruct *x = (struct testStruct *)out;

  vomControl ctl = controlNone;
  uint64_t index;

  while (true) {
    err_t err = decodeVar128(in, &index, &ctl); ck();
    if (ctl == controlEnd) {
      return ERR_OK;
    }
    if (ctl != controlNone) {
      // unexpected control message here
      err = ERR_DECVOM; ck();
    }

    switch (index) {
    case 0:
      err = decodeInt(in, &x->A); ck();
      break; 
    case 1:
      err = decodeDouble(in, &x->B); ck();
      break;
    case 2:
      err = inner_decode(in, &x->Inner); ck();
      break;
    default:
      // unexpected index number.
      err = ERR_DECVOM; ck();
    }
  }

  return ERR_OK;
};

void testStruct_register() {
  err_t err = vomRegister(bufFromString("github.com/jeffallen/miniv.testStruct"),
			  kindStruct, sizeof(struct testStruct),
			  testStruct_decode);
  if (err != ERR_OK) {
    fprintf(stderr, "could not register testStruct: %s\n", errstr(err));
    exit(1);
  }
}
