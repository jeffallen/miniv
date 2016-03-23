// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

import (
	"bytes"
	"testing"

	"v.io/v23/uniqueid"
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

	out, err := decodeStruct(buf.Bytes())
	if err != nil {
		t.Fatal(err)
	}
	ts1 := out.(testStruct)

	if ts0 != ts1 {
		t.Error(ts0, "does not equal", ts1)
	}
}

func TestVomUid(t *testing.T) {
	uid0, err := uniqueid.Random()
	if err != nil {
		t.Fatal(err)
	}
	t.Log("the uniqueid is", uid0)

	buf := &bytes.Buffer{}
	e := vom.NewEncoder(buf)

	err = e.Encode(uid0)
	if err != nil {
		t.Fatal(err)
	}

	out, err := decodeStruct(buf.Bytes())
	if err != nil {
		t.Fatal(err)
	}
	uid := out.(uniqueid.Id)
	if !bytes.Equal(uid[:], uid0[:]) {
		t.Error("uid not equal:", uid0, uid)
	}
}
