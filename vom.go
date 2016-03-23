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

	"v.io/v23/uniqueid"
)

var ErrNotEqual = errors.New("not equal")

type inner struct {
	String string
}

type testStruct struct {
	A     int
	B     float64
	Inner inner
}

func decodeStruct(in []byte) (interface{}, error) {
	var zero testStruct

	C.v23_uniqueid_register()
	C.testStruct_register()

	var d C.decoder_t
	var v C.value_t
	defer C.valueDealloc(&v)
	var done C._Bool

	C.decoderFeed(&d, toBuf_t(in))
	err := C.vomDecode(&d, &v, &done)
	if err != C.ERR_OK {
		return zero, GoError(err)
	}

	if v.ptr == nil {
		return zero, errors.New("expected v.ptr not nil")
	}

	if v.typ == nil {
		return zero, errors.New("expected v.typ not nil")
	}
	typ := bufToString(v.typ.tname)

	switch typ {
	case "github.com/jeffallen/miniv.testStruct":
		ts := (*C.struct_testStruct)((v.ptr))
		return testStruct{
			A:     int(ts.A),
			B:     float64(ts.B),
			Inner: inner{String: bufToString(ts.Inner.String)},
		}, nil
	case "v.io/v23/uniqueid.Id":
		var id uniqueid.Id
		x := C.GoStringN((*C.char)(v.ptr), 16)
		copy(id[:], x)
		return id, nil
	default:
		return nil, nil
	}
}
