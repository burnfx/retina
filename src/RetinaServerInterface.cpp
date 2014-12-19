/*
 * RetinaServerInterface.cpp
 *
 *  Created on: 18.12.2014
 *      Author: richi-ubuntu
 */

#include "RetinaServerInterface.h"
#include "RetinaManager.h"


RetinaServerInterface::RetinaServerInterface(RetinaManager *retinaManager) {
	// TODO Auto-generated constructor stub
	this->retinaManager = retinaManager;
	strcpy(this->control, "");
	strcpy(this->filename,"");
	strcpy(this->mode.cstr,"");
//TODO: ned so!

//TODO: Anders!
	pendingRequest = false;
	pendingRequestFilename = false;
	pendingRequestRedGreen = false;
	pendingRequestControl = false;

	//TODO: so scho eher,ge
	mode.isPending = false;
}

RetinaServerInterface::~RetinaServerInterface() {
	// TODO Auto-generated destructor stub
}


int RetinaServerInterface::setRequestControl(char *control){
	if(strcmp(this->control, control)!=0){
		pendingRequestControl = true;
		pendingRequest = true;
		strcpy(this->control, control);
	}
	return pendingRequestControl;
}

void RetinaServerInterface::setRequestFile(char *filename)
{
	strcpy(this->filename,filename);
	pendingRequest = true;
	pendingRequestFilename = true;
}
int RetinaServerInterface::setRequestMode(char *mode){
	strcpy(this->mode.cstr,mode);
	this->mode.isPending = true;
	pendingRequest = true;
}

int RetinaServerInterface::ExecuteRequests(){
	if(pendingRequestFilename) {
		retinaManager->setFile(filename);
		pendingRequestFilename = false;
	}
	if(this->mode.isPending){
		retinaManager->setMode(this->mode.cstr);
		this->mode.isPending = false;
	}
	if(pendingRequestControl){
		retinaManager->setControl(control);
		pendingRequestControl = false;
	}

	pendingRequest = false; //FIXME
	return pendingRequest;
}


