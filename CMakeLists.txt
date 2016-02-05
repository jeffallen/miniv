# Copyright 2015 The Vanadium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

cmake_minimum_required(VERSION 2.8)
project (miniv)

set(cfiles
	err.c
	message.c
	framer.c
)
add_library (miniv ${cfiles})
target_include_directories (miniv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(gotest COMMAND go test -v ..)

add_subdirectory(mvrpc)



