// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

// #cgo CFLAGS: -I/usr/include/nacl -std=c99
// #include "miniv.h"
// #include "teststruct.h"
import "C"

import (
	"errors"
	"fmt"
	"unsafe"
)

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

func decodeAndCheckPrim(in []byte, vin interface{}) error {
	var v C.value_t
	var d C.decoder_t
	var done C._Bool

	C.decoderFeed(&d, toBuf_t(in))
	err := C.vomDecode(&d, &v, &done)
	if err != C.ERR_OK {
		return GoError(err)
	}

	if !done {
		return errors.New("done is not set")
	}

	c := C.valueGetCell(&v, 0)

	switch c.ctype {
	case C.tidBool:
		if vin.(bool) != (c.u[0] == 1) {
			return ErrNotEqual
		}
	case C.tidByte:
		if vin.(byte) != c.u[0] {
			return ErrNotEqual
		}
	case C.tidInt64:
		u := (*C.int64_t)(unsafe.Pointer(ptr(c.u[:])))
		if int64(*u) != vin.(int64) {
			return ErrNotEqual
		}
	default:
		return fmt.Errorf("unexpected ctype %d", c.ctype)
	}

	return nil
}
