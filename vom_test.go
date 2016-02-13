// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

import (
	"bytes"
	"testing"

	"v.io/v23/vom"
)

func TestVom(t *testing.T) {
	tests := []interface{}{
		true, false,
		byte(0), byte('V'), byte(0xff),
		int64(1), int64(-1), int64(863487625),
	}

	for _, x := range tests {
		buf := &bytes.Buffer{}
		e := vom.NewEncoder(buf)

		err := e.Encode(x)
		if err != nil {
			t.Fatal(err)
		}
		t.Log(x, "encoded as", buf.Bytes())

		err = decodeAndCheck(buf.Bytes(), x)
		if err != nil {
			t.Fatal(err)
		}
	}
}
