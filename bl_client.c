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
#include <pthread.h>
#include <ctype.h>
#include <signal.h>

// #include "server_funcs.c"
#include "simpio.c"
// #include "blather.h"


simpio_t simpio_actual;
simpio_t *simpio = &simpio_actual;


pthread_t user_thread;                                          // data for user thread
pthread_t server_thread;                                                 // data for server thread

client_t client;
client_t *client_pt = &client;
//
//
// The server then waits for clients to write join requests to join FIFO.
// bl_client creates two FIFOs on startup:
//
//     A to-client FIFO for the server to write data intended for the client. The client reads this FIFO.
//     A to-server FIFO to which the client writes and from which the server reads.
//     The names of these files are inconsequential so long as they are fairly unique.
// Basing names for the FIFOs on the PID of the client is an easy way to do this.
// This avoids multiple clients in the same directory mistakenly using each other's FIFOs.
// //
// // A client writes a join request to the server which includes the names of its
// to-client and to-server FIFOs. All subsequent communication between client and
// server is done through the to-client and to-server FIFOs.
// A client reads typed input from users. When input is ready, it is sent as a mesgt to the server. @@
//
// read name of server and name of user from command line args
// create to-server and to-client FIFOs
// write a join_t request to the server FIFO
// start a user thread to read inpu
// start a server thread to listen to the server
// wait for threads to return
// restore standard terminal output
//
// user thread{
//   repeat:
//     read input using simpio
//     when a line is ready
//     create a mesg_t with the line and write it to the to-server FIFO
//   until end of input
//   write a DEPARTED mesg_t into to-server
//   cancel the server thread
//
// server thread{
//   repeat:
//     read a mesg_t from to-client FIFO
//     print appropriate response to terminal with simpio
//   until a SHUTDOWN mesg_t is read
//   cancel the user thread
int keepRunning = 1;
void handle_SIGTERM() {
        //keepRunning=0;
        mesg_t message;
        memset(&message, 0, sizeof(mesg_t));
        message.kind =BL_DEPARTED;
        sprintf(message.name, "%s", client.name); //set the message field with the name of to client
        //sprintf(message.body, "%s", simpio->buf);   //set the message field with the body
        write(client.to_server_fd, &message, sizeof(mesg_t));
        //pthread_cancel(server_thread); // cancel the user_thread thread
        //return NULL;
}
void handle_DEPART() {
        //keepRunning=0;
        mesg_t message;
        memset(&message, 0, sizeof(mesg_t));
        message.kind =BL_DEPARTED;
        // strcpy(message.body,"\0");
        // sprintf(message.body, "%s", "");
        sprintf(message.name, "%s", client.name); //set the message field with the name of to client
        //sprintf(message.body, "%s", simpio->buf);   //set the message field with the body
        write(client.to_server_fd, &message, sizeof(mesg_t));
        // write(client.to_server_fd, &message, sizeof(mesg_t));
        // ^^VALGRIND error, Syscall param write(buf) points to uninitialised byte(s)
        //pthread_cancel(server_thread); // cancel the user_thread thread
        //return NULL;
}

int main(int argc, char *argv[]){
        if(argc < 3) {
                printf("usage: %s <name_of_server> <your_name>\n",argv[0]);
                exit(1);
        }



        sprintf(client.to_client_fname, "%d_client.fifo", getpid());     // fifo named after pid (mildly unsafe naming)
        sprintf(client.to_server_fname,"%d_server.fifo", getpid());           // fifo named after pid for server
        //printf("client.to_client_fname: %s\n",client.to_client_fname);
        //printf("client.to_server_fname: %s\n",client.to_server_fname);
        strcpy(client.name, argv[2]);                   // copy command line arg in as query name
        //printf("connecting to %s\n",join_fname);
        //
        mkfifo(client.to_client_fname, DEFAULT_PERMS);        // create the client FIFO for a response
        mkfifo(client.to_server_fname, DEFAULT_PERMS);
        /*printf("CLIENT %5d: sending request: {fifo_file='%s' server_name='%s' }\n",
               getpid(), client.to_client_fname, client.to_server_fname);*/
        int client_fd = open(client.to_client_fname, O_RDWR);       // open FIFO client read/write in case server hasn't started
        int server_fd = open(client.to_server_fname, O_RDWR);         // open FIFO server read/write in case server hasn't started

        //update fields for client_t
        client.to_client_fd=client_fd;
        client.to_server_fd=server_fd;

        // // server_t: data pertaining to server operations
        // typedef struct {
        //   char server_name[MAXPATH];    // name of server which dictates file names for joining and logging
        //   int join_fd;                  // file descriptor of join file/FIFO
        //   int join_ready;               // flag indicating if a join is available
        //   int n_clients;                // number of clients communicating with server
        //   client_t client[MAXCLIENTS];  // array of clients populated up to n_clients
        //   int time_sec;                 // ADVANCED: time in seconds since server started
        //   int log_fd;                   // ADVANCED: file descriptor for log
        //   sem_t *log_sem;               // ADVANCED: posix semaphore to control who_t section of log file
        // } server_t;
        //
        // // join_t: structure for requests to join the chat room
        // typedef struct {
        //   char name[MAXPATH];            // name of the client joining the server
        //   char to_client_fname[MAXPATH]; // name of file server writes to to send to client
        //   char to_server_fname[MAXPATH]; // name of file client writes to to send to server
        // } join_t;

        join_t join;
        memset(&join, 0, sizeof(join_t));

        sprintf(join.name, "%s", argv[2]); //set the join name as the client name
        sprintf(join.to_client_fname, "%s", client.to_client_fname); //set the join_t field with the name of to client
        sprintf(join.to_server_fname, "%s", client.to_server_fname); //set the join_t field with the name of to server

        //written to the name of the server's FIFO
        // which will notify the server and other clients of the new user's arrival.
        char join_fname[MAXPATH];
        sprintf(join_fname,"%s.fifo",argv[1]);
        int join_fd = open(join_fname,O_RDWR);

        //printf("writing to to_server_fd(%d)\n",join_fd);
        //printf(" to_client_fd(%d)\n",client.to_client_fd);
        if(join_fd!=-1) {
                write(join_fd, &join, sizeof(join_t));
                close(join_fd);

                // ^^VALGRIND error, Syscall param write(buf) points to uninitialised byte(s)
        }
        else {
                close(join_fd);
                exit(1);
        }
        //printf("SUCCESS writing to to_server_fd\n");
        /*mesg_t message;
            message.kind =BL_JOINED;
            sprintf(message.name, "%s", client.name);   //set the message field with the name of to client
            sprintf(message.body, "%s", "just JOINED");   //set the message field with the body

            write(client.to_server_fd, &message, sizeof(mesg_t)); */// send request to server
        //
        // for(int i=0; i<MAX_ROW; i++) {                   // initialized board/mutex locks
        //         for(int j=0; j<MAX_COL; j++) {
        //                 board[i][j] = '.';               // dots fill board initially
        //                 pthread_mutex_init(&mutexes[i][j],NULL); // initialize lock
        //         }
        //         board[i][MAX_COL] = '\n';                // end each board row with a newline
        // }
        // board[MAX_ROW][0] = '\0';                        // board treated as single string, null terminate
        //
        // printf("\33[2J\33[1;1H");                        // erase screen, reset cursor pos to upper left, see note at bottom
        // printf("Running worm threads...\n");
        //

        void *user_thread_func();
        void *server_thread_func();

        // void handler(int signum);                        // handler for interrupts to restore terminal settings

        //iprintf(simpio,"%s",argv[2]);
        char prompt [MAXPATH];
        sprintf(prompt,"%s>> ",client.name);
        simpio_set_prompt(simpio,prompt);
        simpio_reset(simpio);
        simpio_noncanonical_terminal_mode();

        pthread_create(&user_thread, NULL,                 // start a user thread to read input
                       user_thread_func, NULL);

        pthread_create(&server_thread, NULL,                     // start a server thread to listen to the server
                       server_thread_func, NULL);
		//iprintf(simpio,"\nJOINING user thread\n");
        pthread_join(user_thread, NULL); //wait for thread to return
        //iprintf(simpio,"\nJOINING server thread\n");
        pthread_join(server_thread, NULL);//wait for thread to return
		//iprintf(simpio,"\nboth thread joined\n");

        // restore standard terminal output
        simpio_reset_terminal_mode();
        printf("\n");                 // newline just to make returning to the terminal prettier
        return 0;

}

void *user_thread_func(void *arg){
        //signal(SIGINT,handle_DEPART);
        //signal(SIGTERM,handle_SIGTERM);
        int count = 1;
        while(!simpio->end_of_input && keepRunning) {
                if (errno != EINTR) {
                        simpio_reset(simpio);
                        //(simpio,">>");                              // print prompt
                        while(!simpio->line_ready && !simpio->end_of_input) { // read until line is complete
                                simpio_get_char(simpio);
                        }
                        if(simpio->line_ready) {
                                //   create a mesg_t with the line and write it to the to-server FIFO
                                mesg_t message;
                                memset(&message, 0, sizeof(mesg_t));
                                //message.kind =BL_DEPARTED;
                                message.kind =BL_MESG;
                                strncpy(message.name, client_pt->name, MAXNAME);
                                strncpy(message.body, simpio->buf, MAXLINE);

                                //strcpy(message.name,client.name);
                                //        sprintf(message.name, "%s", client.name);   //set the message field with the name of to client
                                //      sprintf(message.body, "%s", simpio->buf);   //set the message field with the body

                                //iprintf(simpio, "%2d You entered: <%s>\n",count,simpio->buf);

                                write(client.to_server_fd, &message, sizeof(mesg_t));

                                count++;
                        }
                } else {break;}
        }
        

        pthread_cancel(server_thread); // cancel the user_thread thread
		close(client.to_client_fd);
        // close(client.to_server_fd);
        unlink(client.to_client_fname);
        unlink(client.to_server_fname);

        //iprintf(simpio,"@@");
        handle_DEPART();
        return NULL;
        // repeat:
        //   read input using simpio
        //   when a line is ready

        // until end of input


        // write a DEPARTED mesg_t into to-server
        // cancel the server thread
}
void *server_thread_func(){
        //signal(SIGINT,handle_DEPART);
        //signal(SIGTERM,handle_SIGTERM);
        while(1) {
                if (errno == EINTR) {break;}
                //read a mesg_t from to-client FIFO

                //char response[256];
                mesg_t response;
                memset(&response, 0, sizeof(mesg_t));
                // int client_fifo_fd = open(request.client_fifo, O_RDWR);      // open client FIFO to receive response
                // printf("CLIENT %5d: fifo opened, awaiting server response\n",getpid());
                int nread = read(client.to_client_fd, &response, sizeof(mesg_t));             // read a mesg_t from to-client FIFO
                //response[nread] = '\0';                                      // null terminate the string
                if (nread !=-1) {
                        if (response.kind == BL_MESG) {
                                iprintf(simpio,"[%s] : %s\n",response.name,response.body);
                        }
                        else if (response.kind == BL_JOINED) {
                                iprintf(simpio,"-- %s JOINED --\n",response.name); // seg fault
                        }
                        else if (response.kind == BL_DEPARTED) {
                                iprintf(simpio,"-- %s DEPARTED --\n",response.name);
                        }
                        else if (response.kind == BL_SHUTDOWN) {
                                iprintf(simpio,"!!! server is shutting down !!!\n");
                                break; //shutting down this thread
                        }
                }
                else {break;}
                // printf("CLIENT %5d: response for name '%s' is email '%s'\n", // report response
                //        getpid(), request.query_name, response);
                //until a SHUTDOWN mesg_t is read

                /*if(strcmp(response,"BL_SHUTDOWN")==0) {
                        break;
                   }*/

                //print appropriate response to terminal with simpio

                //simpio_reset(simpio);
                /*iprintf(simpio, ">>");                        // print prompt
                   while(!simpio->line_ready && !simpio->end_of_input) { // read until line is complete
                        simpio_get_char(simpio);
                   }
                   if(simpio->line_ready) {
                        iprintf(simpio, "what you entered: %s\n",simpio->buf);
                        //simpio_reset(simpio);
                   }*/
        }
        //   repeat:
        //
        //
        //   until a SHUTDOWN mesg_t is read
        //   cancel the user thread
        close(client.to_client_fd);
        // close(client.to_server_fd);
        unlink(client.to_client_fname);
        unlink(client.to_server_fname);
		//iprintf(simpio,"\nCancelling user thread\n");
        pthread_cancel(user_thread);
		//iprintf(simpio,"\nDONE Cancelling user thread\n");
       // handle_DEPART();
        //iprintf(simpio,">>");
        return NULL;

}



// void handler(int signum);                        // handler for interrupts to restore terminal settings


// printf("CLIENT %5d: opening '%s'\n",
//        getpid(), request.client_fifo);
//
// char response[256];
// int client_fifo_fd = open(request.client_fifo, O_RDWR); // open client FIFO to receive response
// printf("CLIENT %5d: fifo opened, awaiting server response\n",getpid());
//
// int nread = read(client_fifo_fd, response, 255);       // read response from server
// response[nread] = '\0';                                // null terminate the string
// printf("CLIENT %5d: response for name '%s' is email '%s'\n", // report response
//        getpid(), request.query_name, response);
//
// close(client_fifo_fd);
// close(requests_fd);2d
// remove(request.client_fifo);
//
// exit(0);
