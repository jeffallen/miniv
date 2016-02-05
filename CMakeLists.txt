# Copyright 2015 The Vanadium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

cmake_minimum_required(VERSION 2.8)
project (miniv)

set(cfiles
	err.c
	flow/message/message.c
	framer/framer.c
)
add_library (miniv ${cfiles})

add_subdirectory(mvrpc)



