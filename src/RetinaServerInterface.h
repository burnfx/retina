/*
 * RetinaServerInterface.h
 *
 *  Created on: 18.12.2014
 *      Author: richi-ubuntu
 */

#ifndef RETINASERVERINTERFACE_H_
#define RETINASERVERINTERFACE_H_
#include "main.h"

typedef struct{
	char cstr[40];
	bool isPending;
} RequestType;

typedef struct{
	char cstr[40];
	bool isPending;
} ReplyType;

class RetinaManager;

class RetinaServerInterface {
private:
	bool pendingRequest;
	bool pendingReply;

	// playTime in ms
	ReplyType playTime;

	RetinaManager *retinaManager;
	RequestType mode;
	RequestType control;
	RequestType filename;
	RequestType redGreen;
	RequestType updateInterval;
	RequestType displayInterval;
	RequestType cDecay;
	RequestType viewport_Offset;
	RequestType translateBack_Offset;
	RequestType time;


public:
	int setReplyTime(std::string time);

	int setRequestFile(char *filename);
	int setRequestRedGreen(char *RedGreen);
	int setRequestControl(char *control);
	int setRequestMode(char *mode);
	int setRequestUpdateInterval(char *updateInterval);
	int setRequestDisplayInterval(char *updateInterval);
	int setRequestcDecay(char *updateInterval);
	int setRequestViewport_Offset(char *viewport_Offset);
	int setRequestTranslateBack_Offset(char *viewport_Offset);
	int setRequestTime(char *time);

	bool hasRequests(){return pendingRequest;}
	bool hasReplies() {return pendingReply;}

	const char* getReplyTime();
	int ExecuteRequests();

	RetinaServerInterface(RetinaManager *retinaManager);
	virtual ~RetinaServerInterface();
};

#endif /* RETINASERVERINTERFACE_H_ */
