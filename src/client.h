#ifndef CLIENT_H_
#define CLIENT_H_
    extern int client;
    void sendR(char** cmd);
    void cleanup(int signum);
    void startClient();
    void stopClient(int signum);
#endif

