// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>

#include "miniv.h"

err_t connectionEnsureFrame(struct connection *c, unsigned long len) {
  return bufExpand(&c->frame, len);
}
  
err_t connectionReadFrame(struct connection *c) {
  unsigned long len;
  err_t err = frameReadLen(c->fd, &len);
  if (err != ERR_OK) {
    return err;
  }
  err = connectionEnsureFrame(c, len);
  if (err != ERR_OK) {
    return err;
  }

  c->frame.len = 0;
  unsigned char *p = c->frame.buf;
  while (c->frame.len < len) {
    ssize_t n = read(c->fd, p, len - c->frame.len);
    if (n < 0) {
      return ERR_CONNECTION;
    }
    p += n;
    c->frame.len += (unsigned long)n;
  }
  return ERR_OK;
}

err_t connectionOpen(const char *hostname, uint16_t port, struct connection *c) {
  if (c == NULL) {
    return ERR_PARAM;
  }
  
  struct sockaddr_in sa_in;
  const struct sockaddr *sa = (struct sockaddr *)&sa_in;
  struct hostent *server;

  /* socket: create the socket */
  c->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (c->fd < 0) {
    return ERR_SOCKET;
  }
  
  /* gethostbyname: get the server's DNS entry */
  server = gethostbyname(hostname);
  if (server == NULL) {
    return ERR_HOSTNAME;
  }
  
  /* build the server's Internet address */
  memset(&sa_in, 0, sizeof(sa_in));
  sa_in.sin_family = AF_INET;
  memcpy(server->h_addr, &sa_in.sin_addr.s_addr, server->h_length);
  sa_in.sin_port = htons(port);
  
  /* connect: create a connection with the server */
  if (connect(c->fd, sa, sizeof(sa_in)) < 0) {
    return ERR_SOCKETCONNECT;
  }
  
  return ERR_OK;
}

err_t connectionHandshake(struct connection *c) {
  struct Message s = {};
  s.mtype = Setup;
  s.u.Setup.ver_min = RPC_VER_DEFAULT;
  s.u.Setup.ver_max = RPC_VER_DEFAULT;
  
  c->frame.len = 0;
  err_t err = messageAppend(&s, &c->frame);
  if (err != ERR_OK) {
    return err;
  }
  
  bufDump(c->frame);
  return frameWrite(c->fd, c->frame);
}
