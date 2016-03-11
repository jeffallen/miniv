// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

// #cgo CFLAGS: -I/usr/include/nacl -std=c99
// #include "miniv.h"
// #include "teststruct.h"
import "C"

import "errors"

var ErrNotEqual = errors.New("not equal")

func decodeStruct(in []byte) (*testStruct, error) {
	C.testStruct_register()

	var d C.decoder_t
	var v C.value_t
	var done C._Bool

	C.decoderFeed(&d, toBuf_t(in))
	err := C.vomDecode(&d, &v, &done)
	if err != C.ERR_OK {
		return nil, GoError(err)
	}

	if v.ptr == nil {
		return nil, errors.New("expected v.ptr not nil")
	}

	ts := (*C.struct_testStruct)((v.ptr))

	return &testStruct{A: int(ts.A), B: float64(ts.B)}, nil
}
