// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

/*
#include "miniv.h"
#include <stdlib.h>
*/
import "C"
import (
	"reflect"
	"unsafe"

	"v.io/v23/flow/message"
	"v.io/v23/rpc/version"
)

func Read(in []byte) (message.Message, error) {
	m := C.messageNew()
	defer C.free(unsafe.Pointer(m))

	var buf C.buf_t
	buf.buf = (*C.uchar)(ptr(in))
	buf.len = C.ulong(len(in))
	buf.cap = C.ulong(cap(in))

	err := C.messageRead(buf, m)
	if err != C.ERR_OK {
		return nil, GoError(err)
	}

	if m.mtype == C.Setup {
		s := (*C.struct_Setup)(unsafe.Pointer(ptr(m.u[:])))

		var key [32]byte
		for i, b := range s.PeerNaClPublicKey {
			key[i] = byte(b)
		}

		return &message.Setup{
			Versions: version.RPCVersionRange{
				Min: version.RPCVersion(s.ver_min),
				Max: version.RPCVersion(s.ver_max),
			},
			PeerNaClPublicKey: &key,
			Mtu:               uint64(s.mtu),
			SharedTokens:      uint64(s.sharedTokens),
		}, nil
	}

	return nil, nil
}

func ptr(in []byte) unsafe.Pointer {
	hdrp := (*reflect.SliceHeader)(unsafe.Pointer(&in))
	return unsafe.Pointer(hdrp.Data)
}
