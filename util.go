// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

// #cgo CFLAGS: -I/usr/include/nacl
// #include "miniv.h"
import "C"

import (
	"errors"
	"unsafe"
)

func GoError(e C.err_t) error {
	return errors.New(C.GoString(C.errstr(e)))
}

// Find the pointer to the underlying data for slice in.
func ptr(in []byte) unsafe.Pointer {
	return unsafe.Pointer(&in[0])
}

func toBuf_t(in []byte) C.buf_t {
	var buf C.buf_t
	buf.buf = (*C.uchar)(ptr(in))
	buf.len = C.ulong_t(len(in))
	buf.cap = C.ulong_t(cap(in))
	return buf
}

func bufToString(buf C.buf_t) string {
	return C.GoStringN((*C.char)(unsafe.Pointer(buf.buf)), C.int(buf.len))
}
