// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "miniv.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/uio.h>

static const unsigned int maxPacketSize = 0xffffff;

err_t frameReadLen(int fd, unsigned long *len) {
  unsigned char buf[3];
  ssize_t n = read(fd, buf, 3);
  if (n != 3) {
    return ERR_CONNECTION;
  }
  
  *len = maxPacketSize - (unsigned long)(buf[0]<<16 | buf[1]<<8 | buf[2]);
  return ERR_OK;
}

err_t frameWrite(int fd, buf_t b) {
  unsigned char dst[3];
  unsigned long len = maxPacketSize - b.len;
  dst[0] = (len & 0xff0000) >> 16;
  dst[1] = (len & 0x00ff00) >> 8;
  dst[2] = len & 0x0000ff;

  struct iovec iov[2] = {
    { .iov_base = dst, .iov_len = 3 },
    { .iov_base = b.buf, .iov_len = b.len }
  };
  
  ssize_t n = writev(fd, iov, 2);
  if (n < 0) {
    return ERR_CONNECTION;
  }
  return ERR_OK;
}
