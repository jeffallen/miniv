// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

// #cgo CFLAGS: -I/usr/include/nacl -std=c99
// #include "miniv.h"
import "C"

import (
	"errors"
	"fmt"
	"unsafe"
)

var ErrNotEqual = errors.New("not equal")

func decodeAndCheck(in []byte, vin interface{}) error {
	var v C.value_t
	var d C.struct_decoder
	var done C._Bool
	C.decoderFeed(&d, toBuf_t(in))
	err := C.vomDecode(&d, &v, &done)
	if err != C.ERR_OK {
		return GoError(err)
	}

	c := C.valueGetCell(&v, 0)
	//	fmt.Println(c)

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
