#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>

#include "../err.h"
#include "../framer/framer.h"
#include "../flow/message/message.h"

typedef struct {
  unsigned char *buf;
  int len;
  int cap;
} buf_t;

// thanks SO:
// https://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data
void hexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
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
  sprintf(desc, "Buffer len=%d, cap=%d\n", buf->len, buf->cap);
  hexDump(desc, buf->buf, buf->len);
}

err_t bufExpand(buf_t *b, int len) {
  if (b->cap < len) {
    free(b->buf);
    b->len = 0;
    b->cap = 0;
    b->buf = malloc(len);
    if (b->buf == NULL) {
      return ERR_MEM;
    }
    b->cap = len;
  }
  return ERR_OK;
}

struct connection {
  int fd;
  buf_t frame;
};

err_t connectionEnsureFrame(struct connection *c, int len) {
  return bufExpand(&c->frame, len);
}
  
err_t connectionReadFrame(struct connection *c) {
  int len;
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
    int n = read(c->fd, p, len - c->frame.len);
    if (n < 0) {
      return ERR_CONNECTION;
    }
    p += n;
    c->frame.len += n;
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

// Find the host:port in a Vanadium endpoint like this;
// @6@wsh@192.168.0.107:64994@@85b4dbc6d23fa02d98b74c9008c3f1b0@l@dev.v.io:u:jra@nella.org:bridge@@
err_t nameParse(char *name, char **host, char **port) {
  if (name == NULL) {
    return ERR_PARAM;
  }

  char *p = name;
  int ct = 0;
  while (*p) {
    if (ct == 3) {
      // Found hostname, terminate it.
      *host = p;
      p = index(p, ':');
      if (p == NULL) {
	break;
      }
      *p = 0;
      p++;
      *port = p;
      p = index(p, '@');
      if (p == NULL) {
	break;
      }
      *p = 0;      
      return ERR_OK;
    }

    if (*p == '@') {
      ct++;
    }
    p++;
  }
  
  return ERR_ENDPOINT;
}

static void ckerr(const char *what, err_t err) {
  if (err != ERR_OK) {
    fprintf(stderr, "Error %s: %s\n", what, errstr(err));
    exit(1);
  }
}

int main(int argc, char **argv) {
  struct connection c0 = {};
  struct connection *c = &c0;

  if (argc != 2) {
    fprintf(stderr, "usage: %s endpoint-name\n", argv[0]);
    exit(1);
  }

  char *name = strdup(argv[1]);
  char *host, *portstr;
  
  // attention: nameParse mutates name.
  ckerr("parsing name", nameParse(name, &host, &portstr));
  int port = atoi(portstr);

  printf("Connecting to %s:%d\n", host, port);
  ckerr("connecting", connectionOpen(host, port, c));
  
  host = NULL; portstr = NULL; free(name);

  // Talk to the server!
  // a hack until we are ready do send our own setup...
  const char *yo = "yo";
  write(c->fd, yo, 3);

  ckerr("read frame", connectionReadFrame(c));
  bufDump(&c->frame);

  // Whoo hoo!!

  close(c->fd);
}

