/*
 * RetinaServerInterface.h
 *
 *  Created on: 18.12.2014
 *      Author: richi-ubuntu
 */

#ifndef RETINASERVERINTERFACE_H_
#define RETINASERVERINTERFACE_H_
#include "main.h"

typedef struct RequestType{
	char cstr[40];
	bool isPending;
};

class RetinaManager;

class RetinaServerInterface {
private:
	bool pendingRequest;
	bool pendingRequestFilename;
	bool pendingRequestRedGreen;
	bool pendingRequestControl;
	bool pendingRequestUpdateInterval;
	bool pendingRequestDisplayInterval;
	bool pendingRequestCDecay;
	bool pendingRequestViewport_Offset;
	bool pendingRequestTranslateBack_Offset;

	char filename[40];
	char control[40];
	RetinaManager *retinaManager;
	RequestType mode;
public:
	bool hasRequests(){ return pendingRequest;}
	void setRequestFile(char *filename);
	//void setRequestRedGreen(int RedGreen){this->redGreen = RedGreen; pendingRequest = true; pendingRequestRedGreen = true;};

	int setRequestControl(char *control);
	int setRequestMode(char *mode);
	int ExecuteRequests();

	RetinaServerInterface(RetinaManager *retinaManager);
	virtual ~RetinaServerInterface();
};

#endif /* RETINASERVERINTERFACE_H_ */
