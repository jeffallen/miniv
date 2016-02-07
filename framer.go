// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

/*
#include "miniv.h"
*/
import "C"

import (
	"io"
	"os"
)

type Framer struct {
	rwc io.ReadWriteCloser
}

func New(f io.ReadWriteCloser) *Framer {
	return &Framer{rwc: f}
}

func (f *Framer) fd() C.int {
	if file, ok := f.rwc.(*os.File); ok {
		return C.int(file.Fd())
	}
	panic("cannot get the FD")
}

func (f *Framer) ReadFrame() ([]byte, error) {
	var n C.ulong

	if err := C.frameReadLen(f.fd(), &n); err != C.ERR_OK {
		return nil, GoError(err)
	}

	out := make([]byte, int(n))
	_, err := f.rwc.Read(out)

	return out, err
}

func (f *Framer) WriteFrame(out []byte) (int, error) {
	if err := C.frameWrite(f.fd(), toBuf_t(out)); err != C.ERR_OK {
		return 0, GoError(err)
	}
	return len(out), nil
}
