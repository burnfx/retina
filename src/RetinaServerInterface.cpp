/*
 * RetinaServerInterface.cpp
 *
 *  Created on: 18.12.2014
 *      Author: richi-ubuntu
 */

#include "RetinaServerInterface.h"
#include "RetinaManager.h"


RetinaServerInterface::RetinaServerInterface(RetinaManager *retinaManager) {
	this->retinaManager = retinaManager;

	pendingRequest = false;
	control.isPending = false;
	filename.isPending = false;
	mode.isPending = false;
	redGreen.isPending = false;
	updateInterval.isPending = false;
	displayInterval.isPending = false;
	cDecay.isPending = false;
	viewport_Offset.isPending = false;
	translateBack_Offset.isPending = false;
}

RetinaServerInterface::~RetinaServerInterface() {
	// TODO Auto-generated destructor stub
}




int RetinaServerInterface::setRequestFile(char *filename)
{
	strcpy(this->filename.cstr,filename);
	this->filename.isPending = true;
	this->pendingRequest = true;
	return this->filename.isPending;
}
int RetinaServerInterface::setRequestMode(char *mode){
	strcpy(this->mode.cstr,mode);
	this->mode.isPending = true;
	this->pendingRequest = true;
	return this->mode.isPending;
}
int RetinaServerInterface::setRequestControl(char *control){
	//if(strcmp(this->control.cstr, control)!=0){
	strcpy(this->control.cstr, control);
	this->control.isPending = true;
	pendingRequest = true;
	//}
	return this->control.isPending;
}
int RetinaServerInterface::setRequestRedGreen(char *RedGreen){
	strcpy(this->redGreen.cstr,RedGreen);
	this->redGreen.isPending = true;
	this->pendingRequest = true;
	return this->redGreen.isPending;
}
int RetinaServerInterface::setRequestUpdateInterval(char *updateInterval){
	strcpy(this->updateInterval.cstr,updateInterval);
	this->updateInterval.isPending = true;
	this->pendingRequest = true;
	return this->updateInterval.isPending;
}
int RetinaServerInterface::setRequestDisplayInterval(char *displayInterval){
	strcpy(this->displayInterval.cstr,displayInterval);
	this->displayInterval.isPending = true;
	this->pendingRequest = true;
	return this->displayInterval.isPending;
}
int RetinaServerInterface::setRequestcDecay(char *cDecay){
	strcpy(this->cDecay.cstr,cDecay);
	this->cDecay.isPending = true;
	this->pendingRequest = true;
	return this->cDecay.isPending;
}
int RetinaServerInterface::setRequestViewport_Offset(char *viewport_Offset){
	strcpy(this->viewport_Offset.cstr,viewport_Offset);
	this->viewport_Offset.isPending = true;
	this->pendingRequest = true;
	return this->viewport_Offset.isPending;
}
int RetinaServerInterface::setRequestTranslateBack_Offset(char *translateBack_Offset){
	strcpy(this->translateBack_Offset.cstr,translateBack_Offset);
	this->translateBack_Offset.isPending = true;
	this->pendingRequest = true;
	return this->translateBack_Offset.isPending;
}

int RetinaServerInterface::ExecuteRequests(){
	if(this->filename.isPending == true) {
		retinaManager->setFile(this->filename.cstr);
		this->filename.isPending = false;
	}
	if(this->mode.isPending){
		retinaManager->setMode(this->mode.cstr);
		this->mode.isPending = false;
	}
	if(this->control.isPending == true){
		retinaManager->setControl(this->control.cstr);
		this->control.isPending = false;
	}
	if(this->redGreen.isPending == true){
		retinaManager->setRedGreen(this->redGreen.cstr);
		this->redGreen.isPending = false;
	}
	if(this->updateInterval.isPending == true){
		retinaManager->setUpdateInterval(this->updateInterval.cstr);
		this->updateInterval.isPending = false;
	}
	if(this->displayInterval.isPending == true){
		retinaManager->setDisplayInterval(this->displayInterval.cstr);
		this->displayInterval.isPending = false;
	}
	if(this->cDecay.isPending == true){
		retinaManager->setcDecay(this->cDecay.cstr);
		this->cDecay.isPending = false;
	}
	if(this->viewport_Offset.isPending == true){
		retinaManager->setViewport_Offset(this->viewport_Offset.cstr);
		this->viewport_Offset.isPending = false;
	}
	if(this->translateBack_Offset.isPending == true){
		retinaManager->setTranslateBack_Offset(this->translateBack_Offset.cstr);
		this->translateBack_Offset.isPending = false;
	}

	pendingRequest = false;
	return pendingRequest;
}


