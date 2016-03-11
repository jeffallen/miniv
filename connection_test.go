// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

import (
	"testing"
)

func TestConnectionPair(t *testing.T) {
	c := ConnectionPair()
	out, err := c.handshakeAndReceive()
	if err != nil {
		t.Fatal(err)
	}
	if string(out) != "hello from c2" {
		t.Error("unexpected: got ", string(out))
	}
}
