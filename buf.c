// Copyright 2016 The Vanadium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "miniv.h"

// thanks SO:
// https://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data
static void hexDump (char *desc, void *addr, unsigned long len) {
    unsigned int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

void bufDump(buf_t *buf) {
  char desc[100];
  sprintf(desc, "Buffer len=%ld, cap=%ld", buf->len, buf->cap);
  hexDump(desc, buf->buf, buf->len);
}

buf_t bufFromString(const char *str) {
  buf_t ret;
  ret.len = strlen(str);
  ret.buf = (unsigned char *)(uintptr_t)str;
  return ret;
}

err_t bufAppendChar(buf_t *b, unsigned char x) {
  err_t err = bufExpand(b, b->len + 1);
  if (err != ERR_OK) {
    return err;
  }
  b->buf[b->len] = x;
  b->len++;
  return ERR_OK;
}

err_t bufAppend(buf_t *b, buf_t in) {
  err_t err = bufExpand(b, b->len + in.len);
  if (err != ERR_OK) {
    return err;
  }
  memcpy(b->buf+b->len, in.buf, in.len);
  return ERR_OK;
}

err_t bufExpand(buf_t *b, unsigned long len) {
  if (b->cap < len) {
    unsigned char *p = malloc(len);
    if (p == NULL) {
      return ERR_MEM;
    }
    memcpy(p, b->buf, b->len);
    free(b->buf);
    b->buf = p;
    b->cap = len;
  }
  return ERR_OK;
}

unsigned long bufRemaining(buf_t *b) {
  if (b->len > b->cap) {
    return 0;
  }
  return b->cap - b->len;
}
