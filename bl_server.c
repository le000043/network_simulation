#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
// #include "server_funcs.c"
#include "blather.h"
server_t server;       // global var so that handle_SIGTERM and handle_SIGINT can see it

int keepRunning=1;
void handle_SIGTERM(){
								keepRunning=0;
								server_shutdown(&server);

}

void handle_SIGINT(){
								keepRunning=0;
								server_shutdown(&server);
}

int main(int argc, char*argv[]){
								signal(SIGINT,handle_SIGINT);
								signal(SIGTERM,handle_SIGTERM);
// need a signal handler

/*struct sigaction my_sa = {};                // portable signal handling setup with sigaction()
   sigemptyset(&my_sa.sa_mask);              // don't block any other signals during handling
   my_sa.sa_flags = SA_RESTART;              // always restart system calls on signals possible

   my_sa.sa_handler = handle_SIGTERM;       // run function handle_SIGTERM
   sigaction(SIGTERM, &my_sa, NULL);       // register SIGTERM with given action

   my_sa.sa_handler = handle_SIGINT;          // run function handle_SIGINT
   sigaction(SIGINT,  &my_sa, NULL); */// register SIGINT with given action


// start a server
//sigset_t block_all, defaults;
//sigfillset( &block_all);
//Sigprocmask(SIG_SETMASK,& block_all, &defaults);
// blocking signal here
//printf("==Server ABOUT to START==\n");
								server_start(&server,argv[1],DEFAULT_PERMS);
//printf("==Server ALREADY START==\n");
// unblock after server starts
//Sigprocmask (SIG_SETMASK, &defaults, NULL);

/*
   REPEAT:
   check all sources
   handle a join request if one is ready
   for each client{
    if the client is ready handle data from it
   }
   }
 * */

								while(keepRunning) {
//printf("# of clients: %d",server.n_clients);
//printf("==Server ABOUT to checking sources==\n");
																if (!keepRunning) {
																								printf("SERVER  1\n");
																								return 0;
																}
																server_check_sources(&server);
																if (!keepRunning) {printf("SERVER  2\n");
																																			return 0; }
//printf("==Server FINISHED checking resources==\n");
																int retval;
																if(server.join_ready) {
																								if (!keepRunning) {
																																printf("SERVER BAILING OUT\n");
																																return 0;
																								}
																								retval = server_handle_join(&server);
																								if (!keepRunning) {printf("SERVER  3\n");
																																											return 0; }
																}

																if (retval == -1) {
																								perror("something wrong in server_handle_join\n");
																								printf("SERVER  4\n");
																								exit(1);

																}
																for (int i=0; i < server.n_clients; i++) {
																								if (!keepRunning) {
																																printf("SERVER  5\n");
																																return 0;
																								}
																								int ret = server_handle_client(&server, i); // handle each client. TERMINATE HERE :(
																								if (!keepRunning) {
																																printf("SERVER  6\n");
																																return 0;
																								}
																								/*if (ret==-1){
																								   perror("something wrong in handling client\n");
																								   exit(1);
																								   }*/
																}
								} //
//server_shutdown(&server);
								printf("SERVER  7\n");
								return 0;
}
