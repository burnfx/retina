#ifndef SERVER_H_
#define SERVER_H_

	extern int server;
	extern int client;
	void *startServer(void *param);
	void *handleGUI(void * paramsd);
	void terminateSocket(int signum);
	void sendQt(char* cmd, int client);

#endif
