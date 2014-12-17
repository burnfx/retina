#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "main.h"
#include "RetinaManager.h"

#include "server.h"
#define SERVER_PORT 1600
#define MAX_MSG 50
#define MAX_CMD 50

int server;


/* handles incoming communication of GUI */
void *handleGUI(void * paramsd) {
    int client_local = *((int *)paramsd);

    char line[MAX_MSG];
    char cmd[MAX_CMD];
    char param[MAX_CMD];
    int read_size;

    /* read_size > 0 	== message received
     * read_size == 0 	== no message received */
    while (	(read_size = recv(client_local , line , MAX_MSG , 0)) >= 0 ) {
        // check for command
        if (strncmp(line, "-", 1) == 0) {
        	printf("command + parameter: %s\n", line);
        	memset(cmd, 0, MAX_CMD);
        	memset(param, 0, MAX_CMD);
        	// read command and parameter
        	if (sscanf(line, "%s %s", cmd, param) > 0) {
        		// searching for a valid command sent from the client
        		if (strcmp(cmd, "-quit")==0)  {
        			printf("command received: %s\n", cmd); printf("parameter: %s\n", param); fflush(stdout);
        			break;
        		}
        		else if (strcmp(cmd, "-mode") == 0) {
        			retinaManager->setMode(param);
        			printf("command received: %s\n", cmd); printf("parameter: %s\n", param); fflush(stdout);
        		}
        		else if (strcmp(cmd, "-control") == 0) {
        			retinaManager->setControl(param);
					printf("command received: %s\n", cmd); printf("parameter: %s\n", param); fflush(stdout);
				}
        		else if (strcmp(cmd, "-cDecay") == 0) {
        			retinaManager->setcDecay(param);
					printf("command received: %s\n", cmd); printf("parameter: %s\n", param); fflush(stdout);
				}
        		else if (strcmp(cmd, "-updateInterval") == 0) {
        			retinaManager->setUpdateInterval(param);
					printf("command received: %s\n", cmd); printf("parameter: %s\n", param); fflush(stdout);
				}
        		else if (strcmp(cmd, "-translateBack_Offset") == 0) {
        			retinaManager->setTranslateBack_Offset(param);
					printf("command received: %s\n", cmd); printf("parameter: %s\n", param); fflush(stdout);
				}
        		else if (strcmp(cmd, "-viewport_Offset") == 0) {
        			retinaManager->setViewport_Offset(param);
					printf("command received: %s\n", cmd); printf("parameter: %s\n", param); fflush(stdout);
				}
        		else if (strcmp(cmd, "-file") == 0) {
        			retinaManager->setFile(param);
					printf("command received: %s\n", cmd); printf("parameter: %s\n", param); fflush(stdout);
				}

        		else if (strcmp(cmd, "-redgreen") == 0) {
        			retinaManager->setRedGreen(param);
					printf("command received: %s\n", cmd); printf("parameter: %s\n", param); fflush(stdout);
				}


        	} else {
        		printf("command not found: %s \n",line);
        		// strcpy(reply, "non existing command: "); strcat(reply, line); strcat(reply, "please use \"-setting value\" e.g. \"-mode 2\""); send(client_local, reply,strlen(reply),0);
        	}
        } else {
        	printf("wrong usage: %s. please use \"-setting value\" e.g. \"-mode 2\"",line);
        	// strcpy(reply, "wrong usage: "); strcat(reply, line); strcat(reply, ""); send(client_local, reply,strlen(reply),0);
        }
    	memset(line,0,MAX_MSG);
    }
    if (read_size == -1) {
    	perror("recv failed");
    }
    close(client_local);
    printf("client quit connection\n");
    pthread_exit(NULL);
    return 0;
}

/* creates server socket communication */
void *startServer(void *param) {
	int client;
	int addr_len;
	pthread_t thread;

	struct sockaddr_in cliAddr;
	struct sockaddr_in servAddr;

	signal(SIGINT, terminateSocket);
	signal(SIGTERM, terminateSocket);

	// IPv4, Stream Socket=TCP, 0=default protocol
	server = socket(PF_INET, SOCK_STREAM, 0);
	if (server < 0) {
		perror("cannot open socket");
	}

	// fill in
	servAddr.sin_family = AF_INET;
	// http://beej.us/guide/bgnet/output/html/multipage/htonsman.html
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(SERVER_PORT);
	memset(servAddr.sin_zero, 0, 8);

	// bind socket to ip:port
	if (bind(server, (struct sockaddr *) &servAddr, sizeof(struct sockaddr)) < 0) {
		perror("cannot bind port ");
	}

	// wait for connection, pending queue = 5
	listen(server, 5);
	printf("server runs on TCP %u\n", SERVER_PORT);

	while(1) {

		addr_len = sizeof(cliAddr);

		// accept blocks, until a connection is made
		client = accept(server, (struct sockaddr *) &cliAddr, (socklen_t *) &addr_len);
		if (client < 0) {
			perror("cannot accept connection ");
			break;
		} else printf ("client accepted\n");

		// new thread is created
		pthread_create(&thread, 0, handleGUI, &client);

		// only one GUI at a time, waiting till thread is joined
		void* retval;
		pthread_join(thread, &retval);
		if (*((int *)&retval) != 0) {
			perror("pthread handleGUI crashed");
		}
	}

	return 0;
}

void terminateSocket(int signum) {

	close(server);
	pthread_exit(NULL);
    return;
}

int serverMain(int argc, char *argv[]) {
	pthread_t srv;
	int value = 1;
	pthread_create(&srv, 0, startServer, &value);
	pthread_detach(srv);
}
