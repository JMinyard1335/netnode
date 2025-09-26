#include "node.h"


//REGION Logging ---------------------------------------------------------------
/* Function to handle the internal logging of the gensio framework. */
void do_vlog(struct gensio_os_funcs *f, enum gensio_log_levels level,
             const char *log, va_list args)
{     
  fprintf(stderr, "gensio %s log: ", gensio_log_level_to_str(level));
  vfprintf(stderr, log, args);
  fprintf(   stderr, "\n");
}


//ENDREGION --------------------------------------------------------------------


//REGION SHUTDOWN --------------------------------------------------------------
/* Checks to see if the accepter shutdown should be started. This is done when
 * there are no more connections. called whenever a shutdown might need to be done.
 */
void check_shutdown(struct node_info *ni)
{
  int rv = 0;
  if (ni->shutting_down && gensio_list_empty(&ni->conns) && !ni->shutdown_called) {
    // all ready shutdown
    ni->shutdown_called = true;
    rv = gensio_acc_shutdown(ni->acc, shutdown_done, ni);
  }

  if (rv) {
    fprintf(stderr, "Error shutting down accepter: %s\n",
            gensio_err_to_str(rv));
    shutdown_done(NULL, ni);
  }
}


/* This function is given to the accepter to call when the shutdown is done.
 * Wakes up the main waiter to terminate the program.
 */
void shutdown_done(struct gensio_accepter *acc, void *shutdown_data)
{
  struct node_info *ni = shutdown_data;
  gensio_os_funcs_wake(ni->o, ni->waiter);
}


/* Called to tell us a connection has finished closing.
 * the data is freed. gensio garuntees that no callbacks
 * will be done when this is called.
 */
void close_done(struct gensio *io, void *close_data)
{
  struct conn_info *ci = close_data;
  gensio_os_funcs_wake(ci->ni->o, ci->ni->waiter);
}


/* Called to start closing the connection. once the connection
 * has finished close_done will be called.
 * @param *ci: a refrence to the connection that needs to be closed.
 */
void start_close(struct conn_info *ci)
{
  int rv = 0;

  if (ci->closing)
    return;

  ci->closing = true;
  rv = gensio_close(ci->io, close_done, ci);
  
  if (rv) {
    ci->err = rv;
    fprintf(stderr, "Error Closing io: %s\n", gensio_err_to_str(rv));
    close_done(ci->io, ci);    
  }
}


//ENDREGION --------------------------------------------------------------------


//REGION CONNECTIONS -----------------------------------------------------------
/* Called when the accepter gets a new connection.
 * Also called on logs events from the accepter.
 * @param acc: the accpeter the the connect came in on.
 * @param *user_data: a refrence to some node_info.
 * @param event: the event to listen for.
 * @param idata: any log info that might be needed.
 */
int accept_connection(struct gensio_accepter *acc, void *user_data,
                             int event, void *idata)
{
  struct node_info *ni = user_data;
  struct gensio *io;
  struct conn_info *ci;
  char str[100];
  gensiods size = sizeof(str);
  int rv;
  
  if (event == GENSIO_ACC_EVENT_LOG) {
    struct gensio_loginfo *li = idata;
    vfprintf(stderr, li->str, li->args);
    fprintf(stderr, "\n");
    return 0;
  }

  if (event != GENSIO_ACC_EVENT_NEW_CONNECTION)
    return GE_NOTSUP;

  io = idata;
  
  printf("Got Connection from the following address: \n");
  snprintf(str, sizeof(str), "%u", 0);
  rv = gensio_control(io,
		      GENSIO_CONTROL_DEPTH_FIRST,
		      GENSIO_CONTROL_GET,
		      GENSIO_CONTROL_RADDR,
		      str, &size);
  if (!rv)
    printf("  %s\n", str);
  
  if (ni->shutting_down) {
    gensio_free(io);
    return 0;
  }
  
  ci = calloc(1, sizeof(*ci));
  if (!ci) {
    fprintf(stderr, "Could not allocate info for new io\n");
    gensio_free(io);
    return 0;
  }
  
  ci->io = io;
  ci->ni = ni;
  gensio_list_add_tail(&ni->conns, &ci->link);
  gensio_set_callback(ci->io, io_event, ci);
  gensio_set_read_callback_enable(ci->io, true);
  return 0;
}


/* 
 * 
 */
int request_connection(struct node_info *ni, struct conn_info *ci)
{
  return 0;
}


//ENDREGION --------------------------------------------------------------------


/* Called when there is a readevent on the gensio or when it can write.
 */
int io_event(struct gensio *io, void *user_data, int event, int err,
             unsigned char *buf, gensiods *buflen, const char *const *auxdata)
{
  struct conn_info *ci = user_data;
  switch (event) {
  case GENSIO_EVENT_READ:
    break;
  case GENSIO_EVENT_WRITE_READY:
    break;
  default:
    return GE_NOTSUP;
  }
  
  return 0;
}
