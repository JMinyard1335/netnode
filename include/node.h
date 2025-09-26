#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gensio/gensio.h>
#include <gensio/gensio_list.h>


/* ---------------------------------------------------------
 * Structures
 * ---------------------------------------------------------*/
// describes a single connection. the program can hold a list of these connections.
struct conn_info {
     struct node_info *ni;
     struct gensio *io;
     struct gensio_link link;
     bool closing;
     int err;
     char *ip;
     unsigned int port;
};


// describes a node in the network. holds all incoming and outgoing connections and general info.
struct node_info{
     struct gensio_os_funcs *o;
     struct gensio_accepter *acc;
     struct gensio_waiter *waiter;
     struct gensio_list conns;
     char *ip;
     unsigned int port;
     bool shutting_down;
     bool shutdown_called;
     bool active;
};


/* ---------------------------------------------------------
 * Functions
 * ---------------------------------------------------------*/
// API functions
int node_init(struct node_info *ni, void* node_data);
int request_connection(struct node_info *ni, struct conn_info *ci);



// Callback functions
void do_vlog(struct gensio_os_funcs *f, enum gensio_log_levels level, const char *log, va_list args);
void shutdown_done(struct gensio_accepter *acc, void *shutdown_data);
void check_shutdown(struct node_info *ni);
void start_close(struct conn_info *ci);
void close_done(struct gensio *io, void *close_data);
int accept_connection(struct gensio_accepter *acc, void *user_data, int event, void *idata);
int io_event(struct gensio *io, void *user_data, int event, int err,
		    unsigned char *buf, gensiods *buflen, const char *const *auxdata);

#endif
