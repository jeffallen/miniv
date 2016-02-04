// Copyright 2015 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package message

/*
#include "message.h"
#include <stdlib.h>
*/
import "C"
import (
	"fmt"
	"reflect"
	"unsafe"

	"v.io/v23/flow/message"
	"v.io/v23/rpc/version"
)

func Read(in []byte) (message.Message, error) {
	m := C.messageNew()
	defer C.free(unsafe.Pointer(m))

	err := C.messageRead((*C.uchar)(ptr(in)), C.uint64_t(len(in)), m)
	if err != C.ERR_OK {
		return nil, fmt.Errorf("message read: error %v", err)
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
