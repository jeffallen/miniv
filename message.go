// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

// #cgo CFLAGS: -I/usr/local/include
// #cgo LDFLAGS: -L/usr/local/lib -lnacl
// #include "miniv.h"
import "C"
import (
	"unsafe"

	"v.io/v23/flow/message"
	"v.io/v23/rpc/version"
)

func Read(in []byte) (message.Message, error) {
	m := C.messageNew()
	defer C.messageFree(m)

	err := C.messageRead(toBuf_t(in), m)
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

func Write(m message.Message) ([]byte, error) {
	var b C.buf_t
	m2 := C.messageNew()

	switch m1 := m.(type) {
	case *message.Setup:
		m2.mtype = C.Setup
		s := (*C.struct_Setup)(unsafe.Pointer(ptr(m2.u[:])))
		s.ver_min = C.uint32_t(m1.Versions.Min)
		s.ver_max = C.uint32_t(m1.Versions.Max)
		for i, x := range m1.PeerNaClPublicKey {
			s.PeerNaClPublicKey[i] = C.uchar(x)
		}
		s.mtu = C.uint64_t(m1.Mtu)
		s.sharedTokens = C.uint64_t(m1.SharedTokens)
	default:
		panic("not impl yet")
	}

	err := C.messageAppend(m2, &b)
	if err != C.ERR_OK {
		return nil, GoError(err)
	}

	out := C.GoBytes(unsafe.Pointer(b.buf), C.int(b.len))
	C.bufDealloc(&b)
	return out, nil
}
