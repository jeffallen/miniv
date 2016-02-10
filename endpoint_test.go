// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

import (
	"testing"
)

func TestEndpoint(t *testing.T) {
	var tests = []struct {
		in, h, p, err string
	}{
		{"@6@wsh@192.168.0.107:54879@@0c4fbc24ac9cf7c967620b9791b0b785@l@dev.v.io:u:jra@nella.org:bridge@@", "192.168.0.107", "54879", ""},
//		{"@6@wsh@[:]:54879@@0c4fbc24ac9cf7c967620b9791b0b785@l@dev.v.io:u:jra@nella.org:bridge@@", "[:]", "54879", ""},
		{"@6@wsh@", "", "", "Could not parse endpoint."},
	}

	for _, x := range tests {
		h, p, err := nameParse(x.in)
		if err != nil && err.Error() != x.err {
			t.Error("unexpected error:", err)
			continue
		}

		if h != x.h {
			t.Errorf("h: want %v got %v", x.h, h)
		}
		if p != x.p {
			t.Errorf("p: want %v got %v", x.p, p)
		}
	}
}
