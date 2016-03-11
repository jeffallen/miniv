// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

import (
	"bytes"
	"testing"

	"v.io/v23/vom"
)

type testStruct struct {
	A int
	B float64
}

func TestVomStruct(t *testing.T) {
	ts0 := testStruct{A: 1, B: 3.14}
	buf := &bytes.Buffer{}
	e := vom.NewEncoder(buf)

	err := e.Encode(ts0)
	if err != nil {
		t.Fatal(err)
	}

	ts1, err := decodeStruct(buf.Bytes())
	if err != nil {
		t.Error(err)
	}
	if ts0 != *ts1 {
		t.Error(ts0, "does not equal", ts1)
	}
}

func TestVomPrim(t *testing.T) {
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

		err = decodeAndCheckPrim(buf.Bytes(), x)
		if err != nil {
			t.Fatal(err)
		}
	}
}
