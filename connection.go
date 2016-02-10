// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

// #cgo CFLAGS: -I/usr/include/nacl -std=c99
// #include "miniv.h"
import "C"

import (
	"log"
	"os"
	"unsafe"
)

type connection struct {
	c1, c2 C.struct_connection
}

func ConnectionPair() *connection {
	c := &connection{}
	p1r, p1w, _ := os.Pipe()
	p2r, p2w, _ := os.Pipe()

	C.connectionNew(&c.c1)
	c.c1.rfd = C.int(p1r.Fd())
	c.c1.wfd = C.int(p2w.Fd())

	// c2 is the "server" who will write to c1
	C.connectionNew(&c.c2)
	c.c2.rfd = C.int(p2r.Fd())
	c.c2.wfd = C.int(p1w.Fd())

	return c
}

func (c *connection) handshakeAndReceive() ([]byte, error) {
	go func() {
		// start c2
		err := C.connectionHandshake(&c.c2)
		if err != C.ERR_OK {
			log.Fatal("c2 handshake:", GoError(err))
		}

		// send some data towards c1
		hello := []byte("hello from c2")
		err = C.connectionWriteFrame(&c.c2, toBuf_t(hello))
		if err != C.ERR_OK {
			log.Fatal("c2 write:", GoError(err))
		}
	}()
	err := C.connectionHandshake(&c.c1)
	if err != C.ERR_OK {
		return nil, GoError(err)
	}

	err = C.connectionReadFrame(&c.c1)
	if err != C.ERR_OK {
		return nil, GoError(err)
	}

	return C.GoBytes(unsafe.Pointer(c.c1.frame.buf), C.int(c.c1.frame.len)), nil
}
