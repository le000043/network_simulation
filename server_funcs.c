#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <poll.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "blather.h"
client_t *server_get_client(server_t *server, int idx){
// Gets a pointer to the client_t struct at the given index. If the
// index is beyond n_clients, the behavior of the function is
// unspecified and may cause a program crash.

								/*if (server->n_clients > 0){
								   char name = (server->client)[idx].name;
								   printf("number of clients: %d with name [%s]\n",server->n_clients,name);
								   }*/
								//printf("idx is %d\n",idx);
								if (idx < server->n_clients) {
																return (&server->client[idx]);
								}
								perror("something wrong with server_get_client\n");
								return NULL;
}

void server_start(server_t *server, char *server_name, int perms){
// Initializes and starts the server with the given name. A join fifo
// called "server_name.fifo" should be created. Removes any existing
// file of that name prior to creation. Opens the FIFO and stores its
// file descriptor in join_fd._
//
// ADVANCED: create the log file "server_name.log" and write the
// initial empty who_t contents to its beginning. Ensure that the
// log_fd is position for appending to the end of the file. Create the
// POSIX semaphore "/server_name.sem" and initialize it to 1 to
// control access to the who_t portion of the log.
								printf("server_name is %s\n",server_name);
								strcpy(server->server_name,server_name);
								char currentFifoName[MAXPATH];
								sprintf(currentFifoName,"%s.fifo",server_name);
								//close(currentFifoName);
								int u_ret = unlink(currentFifoName);
								if (u_ret==-1){
									perror("unlink failed at server_start\n");
								}
								mkfifo(currentFifoName, perms); //A join fifo called "server_name.fifo" should be created
								printf("fifo name is %s\n",currentFifoName);
								int joinFd = open(currentFifoName, O_RDWR);
								if (joinFd!=-1){
								server->join_fd = joinFd; //Opens the FIFO and stores its file descriptor in join_fd.
							}
}
void server_shutdown(server_t *server){
// Shut down the server. Close the join FIFO and unlink (remove) it so
// that no further clients can join. Send a BL_SHUTDOWN message to all
// clients and proceed to remove all clients in any order.
//
// ADVANCED: Close the log file. Close the log semaphore and unlink
// it.
								printf("from SERVER: calling server_shutdown()\n");
								mesg_t newMsg;
								memset(&newMsg, 0, sizeof(mesg_t));
								newMsg.kind = BL_SHUTDOWN;
//sprintf(newMsg . body,"%s\n","server shutting down");
								int r = server_broadcast(server,&newMsg); // broadcast this message to all clients
								if (r==-1){
									perror("server broadcasting failed\n");
								}
								for (int i=0; i < (server->n_clients); i++) {
																int retval = server_remove_client(server,0); // using server_remove_client() to remove a client
																if (!retval) {
																								printf("SUCCESS removing client %d\n",i);
																} else {
																								perror("CANNOT REMOVE CLIENT\n");
																}
								}
								close(server->join_fd); // Close the join FIFO

								char currentFifoName[MAXPATH];
								sprintf(currentFifoName,"%s.fifo",server->server_name);
								int u_ret = unlink(currentFifoName); // remove it so that no further clients can join
								if (u_ret==-1){perror("unlink failed\n");}
// constructing a message
}
int server_add_client(server_t *server, join_t *join){
// Adds a client to the server according to the parameter join which
// should have fileds such as name filed in.  The client data is
// copied into the client[] array and file descriptors are opened for
// its to-server and to-client FIFOs. Initializes the data_ready field
// for the client to 0. Returns 0 on success and non-zero if the
// server as no space for clients (n_clients == MAXCLIENTS).

								if (server->n_clients==MAXCLIENTS) {
																printf("no more rooms\n");
																return -1;
								}
								//printf("adding a client\n");
								client_t newClient; // create a new client_t struct
								/*strncpy(newClient.name,join->name,MAXNAME);
								strncpy(newClient.to_client_fname,join->to_client_fname,MAXNAME);
								strncpy(newClient.to_server_fname,join->to_server_fname,MAXNAME);
								*/
								
								snprintf(newClient.name,MAXNAME,"%s",join->name);
								snprintf(newClient.to_client_fname,MAXNAME,"%s",join->to_client_fname);
								snprintf(newClient.to_server_fname,MAXNAME,"%s",join->to_server_fname);
								
								//printf("join client fname:<%s>\n",join->to_client_fname);
								//printf("new client name: %s\n",newClient.name);
								//printf("%s 's to_client_fname: %s\n",newClient.name, newClient.to_client_fname);
								newClient.to_client_fd = open(join->to_client_fname,O_RDWR);
								newClient.to_server_fd = open(join->to_server_fname,O_RDWR);
								newClient.data_ready = 0;
								(server->client)[server->n_clients] = newClient; // let the last slot in client[] points to the new client struct
								server->n_clients= server->n_clients+1;
								return 0;

}

int server_remove_client(server_t *server, int idx){
// Remove the given client likely due to its having departed or
// disconnected. Close fifos associated with the client and remove
// them.  Shift the remaining clients to lower indices of the client[]
// preserving their order in the array; decreases n_clients.
								if (server->n_clients <= 0) {
																perror("ZERO clients to remove");
																return 1;
								}
// Close fifos associated with the client
								close (server_get_client(server,idx)->to_client_fd);
								close (server_get_client(server,idx)->to_server_fd);
// remove
								int u_retClient = unlink(server_get_client(server,idx)->to_client_fname);
								int u_retServer = unlink(server_get_client(server,idx)->to_server_fname);
								if (u_retClient==-1){
									perror("unlink to_client_fname failed");
								}
								if (u_retServer==-1){
									perror("unlink to_server_fname failed");
								}
// code below Shift the remaining clients to lower indices of the client[]
//    preserving their order in the array
								for (int i=idx; i < (server->n_clients); i++) {
																server->client[i] = server->client[i+1];
								}
								server->n_clients -= 1; // decreases n_clients
// (server->client)[server->n_clients] = NULL;		// unlink the last slot with the last client
// iterate thru
								printf("removing client number [%d]\n",idx);
								for (int i=0; i < (server->n_clients); i++) {
																printf("client %d is %s\n",i,server_get_client(server,i)->name);
								}
								return 0;
}

int server_broadcast(server_t *server, mesg_t *mesg){
// Send the given message to all clients connected to the server by
// writing it to the file descriptors associated with them.
//
// ADVANCED: Log the broadcast message unless it is a PING which
// should not be written to the log.

// NOTE to our team, if a message is missing the last character, change +1 to +2

//mesg_kind_t mesgKind;
								printf("<broadcasting mesg>\n");
//sprintf(mesg->body,"%s\n","<EMPTY>");
								for (int i=0; i < (server->n_clients); i++) {
																printf("(server->client)[%d].to_client_fname: %s\n",i,(server->client)[i].to_client_fname);
																int client_fifo_fd = server_get_client(server,i)->to_client_fd;
																
																
																if (mesg->kind==(BL_DEPARTED)) {
																								char message [MAXLINE];
																								snprintf(message, strlen(mesg->name)+1, "==%s DEPARTED==\n", mesg->name);
																								//write(client_fifo_fd,message,strlen(message));
																}
																else if (mesg->kind==(BL_SHUTDOWN)) {
																								char message [MAXLINE];
																								snprintf(message, strlen(mesg->body)+1, "%s\n", mesg->body);
																								// valgrind error: conditional jump depends on uninitialized
																}
																else {
																								char message [MAXLINE];
																								snprintf(message, 8, "%s\n", "OH NO!");
																								//write(client_fifo_fd,message,strlen(message));
																}
																int r_write = write(client_fifo_fd,mesg,sizeof(mesg_t)); //valgrind error,
																// Syscall param write(buf) points to uninitialised byte(s)
																if (r_write==-1){
																	perror("write failed\n");
																	return -1;
																	}
								}
								return 0;
}

void server_check_sources(server_t *server){
// Checks all sources of data for the server to determine if any are
// ready for reading. Sets the servers join_ready flag and the
// data_ready flags of each of client if data is ready for them.
// Makes use of the poll() system call to efficiently determine
// which sources are ready.



								struct pollfd pfds[(server->n_clients)+1];
//printf("number of clients %d\n",server->n_clients);
								for (int i=0; i < server->n_clients; i++) {
																printf("setting pfds[%d]\n",i);
																pfds[i].fd = server_get_client(server,i)->to_server_fd;
																pfds[i].events = POLLIN;

																pfds[i].revents = 0;

																server_get_client(server,i)->data_ready = 0;
																printf("%d 's server fd is %s\n",i,server_get_client(server,i)->name);
								}
								pfds[server->n_clients].fd = server->join_fd;
								pfds[server->n_clients].events = POLLIN;
								pfds[server->n_clients].revents = 0;
								server->join_ready = 0;
// Sets the servers join_ready flag to true

//if (server->n_clients > 0) {
//printf("=== polling ===\n");
//int ret01 = poll (pfds_join, 1 , -1);
								int ret = poll (pfds, (server->n_clients)+1, -1); // checking any message is found

//printf("ret of poll:%d\n",ret);
								if (ret < 0) {
																perror("poll failed\n");
																return;
								}

// set data_ready flags of each of client if data is ready for them
//printf("number of clients: %d\n",server->n_clients);
								for (int i=0; i < server->n_clients; i++) {
																if (pfds[i].revents && POLLIN) {
																								// data ready, setting client flag
																								printf("client number %d's data is ready\n",i);
																								server_get_client(server,i)->data_ready = 1;
																}
																else {server_get_client(server,i)->data_ready = 0;}
								}
								if (pfds[server->n_clients].revents && POLLIN) {
																// valgrind error: conditional jump depends on uninitialized

																server->join_ready = 1;
//printf("join_ready is %d\n",server -> join_ready);
// server_handle_join(server);
								}
//}
}

int server_join_ready(server_t *server){
// Return the join_ready flag from the server which indicates whether
// a call to server_handle_join() is safe.
								return (server->join_ready);
}

int server_handle_join(server_t *server){
// Call this function only if server_join_ready() returns true. Read a
// join request and add the new client to the server. After finishing,
// set the servers join_ready flag to 0.
								printf("==server handling join==\n");
								if (!server_join_ready(server)) {
																printf("server join not ready\n");
																return -1;
								}
								server->join_ready = 0;
// join_ready is non-zero, proceed...
// Read a join request
//printf("server join ready is true\n");
								join_t join;
//printf("server->join_fd is %d\n", server->join_fd);
								int nread = read(server->join_fd, &join, sizeof(join_t));
								if (nread ==-1){
										perror("read failed\n");
										return -1;
								}
/*if (nread !=-1){
   server -> join_ready = 1;}*/
//printf("nread is %d\n",nread);
								int ret = server_add_client(server, &join);
								if (ret==-1){
									perror("add client failed\n");
									return -1;
								}
								//if (ret)
// letting every client knows someone just join
								mesg_t newMsg;
								memset(&newMsg, 0, sizeof(mesg_t));
								newMsg.kind = BL_JOINED;
								sprintf(newMsg.name,"%s",join.name);
								return server_broadcast(server,&newMsg);


								//return 0;
}
int server_client_ready(server_t *server, int idx){
// Return the data_ready field of the given client which indicates
// whether the client has data ready to be read from it.
								printf("server_client_ready\n");
								return (server_get_client(server,idx)->data_ready);
}

int server_handle_client(server_t *server, int idx){
// Process a message from the specified client. This function should
// only be called if server_client_ready() returns true. Read a
// message from to_server_fd and analyze the message kind. Departure
// and Message types should be broadcast to all other clients.  Ping
// responses should only change the last_contact_time below. Behavior
// for other message types is not specified. Clear the client's
// data_ready flag so it has value 0.
//
// ADVANCED: Update the last_contact_time of the client to the current
// server time_sec.
//printf("handling each client..\n");
								if (server_client_ready(server,idx)) {
																printf("client %d's message ready !!\n",idx);
																printf("handling client number %d\n",idx);
																mesg_t newMessage;
																//printf("reading from to_server_fd\n");
																int nread = read(server_get_client(server,idx)->to_server_fd, &newMessage, sizeof(mesg_t));
																if (nread==-1) {perror("SERVER read failed\n"); exit(1);}

																else if (newMessage.kind == BL_MESG) {
																								printf("message from [%s] is <%s>\n",newMessage.name,newMessage.body);
																}
																if (idx < server->n_clients) {
																								printf("here\n");
																								server_get_client(server,idx)->data_ready = 0;
																}

																// ^^ read info from client at idx's to_server_fd to a mesg_t struct then broadcast that message
																int r = server_broadcast(server, &newMessage);
																if (r==-1){
																	perror("server broadcasting failed\n");
																}
																if (newMessage.kind == BL_DEPARTED) {
																								printf("====DEPART MESG from %d======\n",idx);
																								int r = server_remove_client(server,idx);
																								if (r==-1){
																									perror("removing client failed\n");
																									return -1;
																								}
																}
																return 1;
								}
//printf("%d not ready\n",idx);
								return -1;
}
void server_tick(server_t *server);
// ADVANCED: Increment the time for the server

void server_ping_clients(server_t *server);
// ADVANCED: Ping all clients in the server by broadcasting a ping.

void server_remove_disconnected(server_t *server, int disconnect_secs);
// ADVANCED: Check all clients to see if they have contacted the
// server recently. Any client with a last_contact_time field equal to
// or greater than the parameter disconnect_secs should be
// removed. Broadcast that the client was disconnected to remaining
// clients.  Process clients from lowest to highest and take care of
// loop indexing as clients may be removed during the loop
// necessitating index adjustments.

void server_write_who(server_t *server);
// ADVANCED: Write the current set of clients logged into the server
// to the BEGINNING the log_fd. Ensure that the write is protected by
// locking the semaphore associated with the log file. Since it may
// take some time to complete this operation (acquire semaphore then
// write) it should likely be done in its own thread to preven the
// main server operations from stalling.  For threaded I/O, consider
// using the pwrite() function to write to a specific location in an
// open file descriptor which will not alter the position of log_fd so
// that appends continue to write to the end of the file.

void server_log_message(server_t *server, mesg_t *mesg);
// ADVANCED: Write the given message to the end of log file associated
// with the server.
