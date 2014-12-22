#include "main.h"

#include <iostream>
#include <sstream>
#include "edvs.h"
#include "eDVSGL.h"
#include <OVR.h>
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include "shader.hpp"

//#include "server.h"
#include <string.h>
#include <stdio.h>
#include "RetinaManager.h"
#include "RetinaServerInterface.h"
#include "server.h"


//**********************************************************************
RetinaManager *retinaManager;
RetinaServerInterface *retInterface;
pthread_mutex_t myMutex;

/* *****************************************************************
 // This function reads in all the input params of the main function,
 * which is all input given in argv. The input should be given by
 * a name specifying the name of the variable, followed by the true
 * variable.
 * This function only checks for inputs mode and filename. The
 * correct spelling is defined in the corresponding #defines
 ********************************************************************/
/*
int initMainArguments(int argc, char *argv[]) {
	if (argc < 2) {
		return true;
	}
	char *edvsDataFileNameLeft;
	char *edvsDataFileNameRight;
	char *cMode = NULL;
	char *edvsDataFileName = NULL;
	int initModeViaKeyboard;
	int mode = DEFAULT_MODE;
	// 1. parse mode and filename if it is given
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], MODE_INDICATOR)) {
			edvsDataFileName = argv[i + 1];
		} else if (strcmp(argv[i], FILENAME_INDICATOR)) {
			cMode = argv[i + 1];
		}
	}
	if (cMode != NULL) {
		// 2. check wether mode is in the correct format, must be string of ONE digit.
		if (sizeof(cMode) != sizeof("0")) { //checks for != Null indirectly aswell..
			printf("Wrong mode given: %s", cMode);
			initModeViaKeyboard = true;
		} else { //mode is 1 digit (entered correctly)
			mode = atoi(cMode);
			initModeViaKeyboard = false;
		}
	}
	retinaManager->setMode(mode);
	if (edvsDataFileName != NULL) {
		edvsDataFileNameLeft = strcat(EDVS_DATA_FOLDER_NAME, edvsDataFileName);
		edvsDataFileNameLeft = strcat(edvsDataFileName, FILENAME_EXTENSION_LEFT);
		edvsDataFileNameRight = strcat(EDVS_DATA_FOLDER_NAME, edvsDataFileName);
		edvsDataFileNameRight = strcat(edvsDataFileName, FILENAME_EXTENSION_RIGHT);
	} else {
		printf("No Filename given!");
		edvsDataFileNameLeft = DEFAULT_EDVSDATA_LEFT_FILENAME;
		edvsDataFileNameRight = DEFAULT_EDVSDATA_RIGHT_FILENAME;
	}
	retinaManager->setEdvsFileNameRight(edvsDataFileNameLeft);
	retinaManager->setEdvsFileNameRight(edvsDataFileNameRight);
	return initModeViaKeyboard;
}
*/
static void WindowSizeCallback(GLFWwindow* p_Window, int p_Width, int p_Height) {
	if (glfwGetKey(retinaManager->getWindow(),
	GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		ovrGLConfig tempCfg = retinaManager->getCfg();
		tempCfg.OGL.Header.RTSize.w = p_Width;
		tempCfg.OGL.Header.RTSize.h = p_Height;
		retinaManager->setCfg(tempCfg);

		int distortionCaps = ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp;
		ovrHmd_ConfigureRendering(retinaManager->getHmd(), &(tempCfg.Config), distortionCaps,
				retinaManager->getEyeFov(), (ovrEyeRenderDesc *) (retinaManager->getEyeRenderDesc()));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}
}

int main(int argc, char *argv[]) {
	printf("narg = %i\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("%s\n", argv[i]);
	};
	printf("\n\n");
	pthread_mutex_init(&myMutex,NULL);
	/* *******************************************
	 * THIS IS THE STUFF FOR FELIX's SERVER
	 *********************************************/
	pthread_t srv;
	int value = 1;
	int server;
	pthread_create(&srv, 0, startServer, &server);
	pthread_detach(srv);

	// ************* UNTIL HERE ***********************

	int initModeViaKeyboard = true;
	// TODO: reading params from main has not been tested yet (initMainArguments)
	//initModeViaKeyboard = initMainArguments(argc, argv);

	retinaManager = new RetinaManager();
	retinaManager->Initialize(initModeViaKeyboard);
	//cannot take RetinaManager-Method for this, so do it here.
	glfwSetWindowSizeCallback(retinaManager->getWindow(), WindowSizeCallback);

	FileAndWindowStateType fileAndWindowState = Default;
	FileAndWindowStateType fileState;
	retInterface = new RetinaServerInterface(retinaManager);
	bool goBreak = false;
	while (1) {
		//pthread_mutex_lock(&myMutex);
		retinaManager->KeyControl();
		if (retInterface->hasRequests()){
			retInterface->ExecuteRequests();
		}
		// ************* File and Window ***************
		fileAndWindowState = retinaManager->getFileAndWindowState();
		if (fileAndWindowState == CloseWindowRequest) {
			break;
		}
		// ************* /File and Window ***************

		switch(retinaManager->getControl()){ //THIS IS NOT fileAndWindowState, but State for play pause etc.
			case Play:
				//pthread_mutex_lock(&myMutex);
				fileState = retinaManager->getFileState();
				if (fileState == EndOfFile || fileState == RecordTimeElapsed) {
					retinaManager->setControl(STOP);
					goBreak = true;
				}
				retinaManager->UpdateEvents();

				retinaManager->render();
				//pthread_mutex_unlock(&myMutex);
				break;
			case Pause:
				//render = false;
				break;
			case Stop:
				//retinaManager->setControl(PLAY);
				//retinaManager->render();
				//retinaManager->renderOvrEyes();
				//render = false;
				// FIXME: next 2 lines are just for testing --> now rerunning the same file works. Just delete the next 2 lines later
				//render = retinaManager->setFile("edvs");
				//retinaManager->setControl(PLAY);
				break;
		}
		//if(goBreak) break;
//		pthread_mutex_unlock(&myMutex);
/*
		if(render){
			// ******************* ONE RENDER LOOP ***************************
			retinaManager->render();
			// ****************** /ONE RENDER LOOP ***************************
		}
*/
		//myMutex.unlock();
	}



	retinaManager->TerminateWindow();
	terminateSocket(1);
	return 0;
}

