// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "miniv.h"
#include "crypto_box.h"

#include <stdio.h>

static void advanceNonce(struct stream *s) {
  s->u.intnonce[0]++;
}

void connectionNew(struct connection *c) {
  queueInit(&c->q, sizeof(struct Message *));
}

err_t connectionReadFrame(struct connection *c) {
  const unsigned long mac = crypto_box_BOXZEROBYTES;

  unsigned long len;
  err_t err = frameReadLen(c->rfd, &len); ck();
  
  bufTruncate(&c->frame1);
  bufExpand(&c->frame1, mac + len);
  c->frame1.len = mac;
  unsigned char *p = c->frame1.buf + c->frame1.len;
  while (c->frame1.len < len) {
    ssize_t n = read(c->rfd, p, len);
    if (n < 0) {
      return ERR_CONNECTION;
    }
    if (n == 0) {
      break;
    }
    p += n;
    len -= (unsigned long)n;
    c->frame1.len += (unsigned long)n;
  }

  if (c->encrypt) {
    // The caller must ensure, before calling the crypto_box_open function,
    // that the first crypto_box_BOXZEROBYTES bytes of the ciphertext c are all 0.
    memset(c->frame1.buf, 0, crypto_box_BOXZEROBYTES);
    // Make room in the destination buf.
    bufExpand(&c->frame2, c->frame1.len);
    int rc = crypto_box_open(c->frame2.buf, c->frame1.buf, c->frame1.len, c->dec.u.nonce,
		    c->remotepk, c->sk);
    advanceNonce(&c->dec);
    if (rc == -1) {
      return ERR_UNBOX;
    }
    c->frame.buf = c->frame2.buf + 2 * mac;
    c->frame.len = c->frame1.len - 2 * mac;
  } else {
    c->frame.buf = c->frame1.buf + mac;
    c->frame.len = c->frame1.len - mac;
  }
  
  return ERR_OK;
}

err_t connectionWriteFrame(struct connection *c, buf_t out) {
  const unsigned long mac = crypto_box_BOXZEROBYTES;

  bufExpand(&c->frame1, out.len + 2*mac);
  c->frame1.len = out.len + 2*mac;
  
  bufExpand(&c->frame2, c->frame1.len);
  c->frame2.len = c->frame1.len;
  
  memset(c->frame1.buf, 0, 2*mac);
  memcpy(c->frame1.buf+2*mac, out.buf, out.len);
  
  int rc = crypto_box(c->frame2.buf, c->frame1.buf, c->frame1.len,
		      c->enc.u.nonce, c->remotepk, c->sk);
  advanceNonce(&c->dec);
  if (rc != 0) {
    return ERR_BOX;
  }

  return frameWrite(c->wfd, bufWrap(c->frame2.buf+mac, c->frame2.len-mac));
}

err_t connectionOpen(const char *hostname, uint16_t port, struct connection *c) {
  if (c == NULL) {
    return ERR_PARAM;
  }

  c->dir = Dialer;
  
  struct sockaddr_in sa_in;
  const struct sockaddr *sa = (struct sockaddr *)&sa_in;
  struct hostent *server;

  /* socket: create the socket */
  c->rfd = socket(AF_INET, SOCK_STREAM, 0);
  if (c->rfd < 0) {
    return ERR_SOCKET;
  }
  c->wfd = c->rfd;
  
  /* gethostbyname: get the server's DNS entry */
  server = gethostbyname(hostname);
  if (server == NULL) {
    return ERR_HOSTNAME;
  }
  
  /* build the server's Internet address */
  memset(&sa_in, 0, sizeof(sa_in));
  sa_in.sin_family = AF_INET;
  memcpy(server->h_addr_list[0], &sa_in.sin_addr.s_addr, server->h_length);
  sa_in.sin_port = htons(port);
  
  /* connect: create a connection with the server */
  if (connect(c->rfd, sa, sizeof(sa_in)) < 0) {
    return ERR_SOCKETCONNECT;
  }
  
  return ERR_OK;
}

err_t connectionHandshake(struct connection *c) {
  crypto_box_keypair(c->pk, c->sk);

  // Send out our setup.
  struct Message lSetupM = {};
  lSetupM.mtype = Setup;
  lSetupM.u.Setup.ver_min = RPC_VER_DEFAULT;
  lSetupM.u.Setup.ver_max = RPC_VER_DEFAULT;
  for (unsigned long i = 0; i < sizeof(lSetupM.u.Setup.PeerNaClPublicKey); i++) {
    lSetupM.u.Setup.PeerNaClPublicKey[i] = c->pk[i];
  }
  
  bufTruncate(&c->frame);
  err_t err = messageAppend(&lSetupM, &c->frame); ck();
  err = frameWrite(c->wfd, c->frame); ck();

  // Read their setup.
  err = connectionReadFrame(c); ck();

  struct Message rSetupM = {};
  err = messageRead(c->frame, &rSetupM); ck();
  if (rSetupM.mtype != Setup) {
    return ERR_HANDSHAKE;
  }

  // Validate and process their setup.
  struct Setup *rSetup = &rSetupM.u.Setup;
  if (RPC_VER_DEFAULT < rSetup->ver_min || RPC_VER_DEFAULT > rSetup->ver_max) {
    return ERR_HANDSHAKE;
  }
  for (unsigned long i = 0; i < sizeof(c->remotepk); i++) {
    c->remotepk[i] = rSetup->PeerNaClPublicKey[i];
  }

  // Setup the nonces for enc/dec; see box_cipher.go:NewControlCipherRPC11 for
  // why the memcmp.
  if (memcmp(c->pk, c->remotepk, sizeof(c->pk)) < 0) {
    c->enc.u.nonce[8] = 1;
  } else {
    c->dec.u.nonce[8] = 1;
  }

  bufTruncate(&c->binding);
  if (c->dir == Dialer) {
    messageAppend(&lSetupM, &c->binding);
    messageAppend(&rSetupM, &c->binding);
  } else {
    messageAppend(&rSetupM, &c->binding);
    messageAppend(&lSetupM, &c->binding);
  }
  c->encrypt = true;
  return ERR_OK;
}

// This stupid stuff is needed by libnacl on Linux, and apparently doesn't
// hurt anything on MacOS.

static int fd = -1;

void randombytes(unsigned char *x,unsigned long long xlen) {
  ssize_t i;
  
  if (fd == -1) {
    for (;;) {
      fd = open("/dev/urandom",O_RDONLY);
      if (fd != -1) break;
      sleep(1);
    }
  }

  while (xlen > 0) {
    if (xlen < 1048576) i = (ssize_t)xlen; else i = 1048576;
    
    i = read(fd,x,(size_t)i);
    if (i < 1) {
      sleep(1);
      continue;
    }
    
    x += i;
    xlen -= (unsigned long long)i;
  }
}

int randombytes_close(void) {
  int rc = -1;
  if(fd != -1 && close(fd) == 0) {
    fd = -1;
    rc = 0;
  }
  return rc;
}

