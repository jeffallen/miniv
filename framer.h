// Copyright 2015 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "miniv.h"
#pragma once

err_t frameReadLen(int fd, int *len);
err_t frameWriteLen(int fd, int len);
