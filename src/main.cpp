#include "main.h"

#include <iostream>
#include <sstream>
#include "edvs.h"
#include "eDVSGL.h"
#include <OVR.h>
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include "shader.hpp"

#include "server.h"
#include <string.h>
#include <stdio.h>
#include "RetinaManager.h"



// Defines
//*********************************************************************
#define MODE_INDICATOR "-mode"
#define FILENAME_INDICATOR "-filename"
#define EDVS_DATA_FOLDER_NAME "edvsdata/"
#define FILENAME_EXTENSION_RIGHT "_right.txt"
#define FILENAME_EXTENSION_LEFT "_left.txt"

#define SHOW_ON_OCULUS
#define DEFAULT_MODE 2

//**********************************************************************
RetinaManager *retinaManager;

/* *****************************************************************
 // This function reads in all the input params of the main function,
 * which is all input given in argv. The input should be given by
 * a name specifying the name of the variable, followed by the true
 * variable.
 * This function only checks for inputs mode and filename. The
 * correct spelling is defined in the corresponding #defines
 ********************************************************************/
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

	/* *******************************************
	 * THIS IS THE STUFF FOR FELIX's SERVER
	 *********************************************/
	pthread_t srv;
	int value = 1;
	pthread_create(&srv, 0, startServer, &value);
	pthread_detach(srv);

	// ************* UNTIL HERE ***********************

	int initModeViaKeyboard;
	// TODO: reading params from main has not been tested yet (initMainArguments)
	initModeViaKeyboard = initMainArguments(argc, argv);

	retinaManager = new RetinaManager();
	retinaManager->Initialize(initModeViaKeyboard);
	glfwSetWindowSizeCallback(retinaManager->getWindow(), WindowSizeCallback);
	// TEST setMode und changeFile functions
	//retinaManager->setMode(2);
	//retinaManager->changeFile("edvsdata/e0d30l1m1r6h444_left.txt","edvsdata/e0d30l1m1r6h444_right.txt");


	RetinaReturnType status;
	while (1) {
		// GET INPUTS --> mode change? DO IT HERE, BEFORE RENDER..
		retinaManager->KeyControl();
		status = retinaManager->render();
		if (status == RecordTimeElapsed || status == CloseWindowRequest) {
			break;
		} else if (status == EndOfFile) {
			//TODO: do what ever you want.... for example break loop...
			//break;
			retinaManager->StopVideo();
			retinaManager->setFile("e0d30l1m1r1h444");
		}
	}

	// Play/Pause über "weiter machen mit retinaManager->render()"
	// Back / Forward über changeFile
	// Stop: fehlt noch --> Switch in einen "mode", der nur schwarz zeigt (mode 5 z.B.)
//	while(1){
//		while(status == ContinueRunning){
//			//updates = getGUIUpdates(); // Hier fehlt dann auch noch was man mit den updates tut
//			// if(updates.status == StopRunning) {retinaManager->setMode(5); break; //so oder ähnlich...
//
//			retinaManager->KeyControl();
//			status = retinaManager->render();
//		}
//		if (status == CloseWindowRequest){
//			break;
//		}
//		if (status == EndOfFile){
//			retinaManager->setMode(5);
//		}
//	}



	retinaManager->TerminateWindow();
	// ************ UND HIER NOCHMAL ************
	terminateSocket(1);
	return 0;
}

