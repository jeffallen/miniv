// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

import (
	"bytes"
	"testing"

	"v.io/v23/vom"
)

func TestVomStruct(t *testing.T) {
	ts0 := testStruct{A: 1, B: 3.14, Inner: inner{String: "Hello."}}
	buf := &bytes.Buffer{}
	e := vom.NewEncoder(buf)

	err := e.Encode(ts0)
	if err != nil {
		t.Fatal(err)
	}

	ts1, err := decodeStruct(buf.Bytes())
	if err != nil {
		t.Fatal(err)
	}

	if ts0 != ts1 {
		t.Error(ts0, "does not equal", ts1)
	}
}
