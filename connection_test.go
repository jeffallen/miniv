// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

import (
	"log"
	"os"
	"testing"
)

type snoop struct {
}

func newSnoop(f1, f2 *os.File) (pw, pr *os.File) {
	var err error
	pr, pw, err = os.Pipe()
	if err != nil {
		log.Fatal("snoop pipe")
	}

	// read f1, snoop, write pw
	go func() {
		var buf [1024]byte
		for {
			n, err := f1.Read(buf[:])
			if err != nil {
				return
			}
			log.Println("snoop: ", n, " data: ", buf[0:n])
			pw.Write(buf[0:n])
		}
	}()

	go func() {
		var buf [1024]byte
		for {
			n, err := f2.Read(buf[:])
			if err != nil {
				return
			}
			log.Println("snoop: ", n, " data: ", buf[0:n])
			pr.Write(buf[0:n])
		}
	}()

	return pw, pr
}

func TestConnectionPair(t *testing.T) {
	c := ConnectionPair()
	out, err := c.handshakeAndReceive()
	if err != nil {
		t.Fatal(err)
	}
	t.Log("got ", string(out))
}
