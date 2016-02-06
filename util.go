// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package miniv

/*
#include "miniv.h"
*/
import "C"

import "errors"

func GoError(e C.err_t) error {
	return errors.New(C.GoString(C.errstr(e)))
}
