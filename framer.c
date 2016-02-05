// Copyright 2015 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "miniv.h"

#include <sys/types.h>
#include <unistd.h>

const unsigned int maxPacketSize = 0xffffff;

err_t frameReadLen(int fd, int *len) {
  unsigned char buf[3];
  ssize_t n = read(fd, buf, 3);
  if (n != 3) {
    return ERR;
  }
  
  *len = maxPacketSize - (buf[0]<<16 | buf[1]<<8 | buf[2]);
  return ERR_OK;
}

err_t frameWriteLen(int fd, int len) {
  unsigned char dst[3];
  
  len = maxPacketSize - len;
  dst[0] = (len & 0xff0000) >> 16;
  dst[1] = (len & 0x00ff00) >> 8;
  dst[2] = len & 0x0000ff;

  ssize_t n = write(fd, dst, 3);
  if (n != 3) {
    return ERR;
  }
  return ERR_OK;
}

