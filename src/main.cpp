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


	pthread_t srv;
	int server;
	pthread_create(&srv, 0, startServer, &server);
	pthread_detach(srv);

	retInterface = new RetinaServerInterface(retinaManager);
	retinaManager = new RetinaManager();
	retinaManager->Initialize(false, retInterface);


	// ******Initialize eDVS *******
	retinaManager->setMode(2);
	retinaManager->setFile("somethingWrong");
	// NOTE: Type "T" to start file edvs (for debugging).
	//retinaManager->setFile("edvs");

	//cannot take RetinaManager-Method as a glfw callback fun, so do it here with a "general" fun.
	glfwSetWindowSizeCallback(retinaManager->getWindow(), WindowSizeCallback);

	FileAndWindowStateType fileAndWindowState = Default;
	FileAndWindowStateType fileState;
	//TODO: fileState and fileAndWindowState ist übrig geblieben beim splitten des typedefs...
	// passt so nicht, v.A. von der Namensgebung!


	while (1) {
		retinaManager->KeyControl();
		if (retInterface->hasRequests()){
			retInterface->ExecuteRequests();
		}

		// ************* File and Window ***************
		fileAndWindowState = retinaManager->getFileAndWindowState();
		if (fileAndWindowState == CloseWindowRequest) {
			break;
		}
		switch(retinaManager->getControl()){ //THIS IS NOT fileAndWindowState, but State for play pause etc.
			case Play:
				fileState = retinaManager->getFileState();
				if (fileState == EndOfFile || fileState == RecordTimeElapsed) {
					retinaManager->setControl(STOP);
				}
				retinaManager->UpdateEvents();
				retinaManager->render();
				//if(retinaManager->isFirstFrame()){retinaManager->measureStartTime();}
				break;
			case Pause:
				break;
			case Stop:
				break;
		}
	}
	retinaManager->TerminateWindow();
	terminateSocket(1);
	return 0;
}

