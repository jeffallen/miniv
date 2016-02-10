// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

// #cgo CFLAGS: -I/usr/include/nacl
// #include "miniv.h"
// #include <stdlib.h>
// #include <string.h>
import "C"
import "unsafe"

func nameParse(in string) (host string, port string, err error) {
	tmp := C.CString(in)
	defer C.free(unsafe.Pointer(tmp))

	var h, p *C.char
	e := C.nameParse(tmp, &h, &p)
	if e != C.ERR_OK {
		return "", "", GoError(e)
	}

	host = C.GoString(h)
	port = C.GoString(p)
	return
}
