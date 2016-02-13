# Copyright 2016 The Vanadium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

cmake_minimum_required(VERSION 2.8)
project (miniv)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Weverything -Wno-padded -Wno-switch-enum -Wno-covered-switch-default -Wno-gnu-empty-initializer -Werror ${CMAKE_C_FLAGS_DEBUG}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wno-format ${CMAKE_C_FLAGS_DEBUG}")
endif()

find_library(nacl_lib nacl)
find_path(nacl_inc crypto_box.h PATHS /usr/include/nacl)
include_directories(${nacl_inc})

set(cfiles
	buf.c
	connection.c
	endpoint.c
	err.c
	message.c
	framer.c
	signature.c
	vom.c
)
add_library (miniv ${cfiles})
target_link_libraries(miniv LINK_PUBLIC ${nacl_lib})

target_include_directories (miniv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(gotest COMMAND go test -v ..)

add_subdirectory(mvrpc)



