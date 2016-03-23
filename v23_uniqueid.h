// Copyright 2015 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// This file was auto-generated by the vanadium vdl tool.
// Package: v.io/v23/uniqueid

#pragma once


// An Id is a likely globally unique identifier.
typedef unsigned char v23_uniqueid_Id[16];


void v23_uniqueid_register(void);