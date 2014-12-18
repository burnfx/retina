/*
 * RetinaManager.cpp
 *
 *  Created on: 10.12.2014
 *      Author: richi-ubuntu
 */
#include "RetinaManager.h"
#include "ParameterManager.h"
#include <sstream>
#include <string.h>
#include <stdio.h>
#include "main.h"

#define getSignForEye(x) ((x)==0 ? (1) : (-1))

// std::mutex myMutex; // Hier und in server.cpp geht natürlich nicht (wäre doppelter lock)

RetinaManager::RetinaManager() {

}

RetinaManager::~RetinaManager() {
	// TODO Auto-generated destructor stub
}

int RetinaManager::isTimeElapsed() {
	RetinaManager::recordCounter++;
	if (RetinaManager::recordCounter > RetinaManager::recordPlayTime * 60) {
		return true;
	}
	return false;
}

/* ******************** Measure FPS ********************************************
 * FPS target in ms: 1000ms/FPS
 * 60FPS = 16.666667ms ## 75 FPS = 13.333333ms, etc.
 ******************************************************************************/
double RetinaManager::measureFPS() {

	double currentTime = glfwGetTime();
	nbFrames++;
	double d = 1.0;
	if (currentTime - initTime >= 1.0) {
		// must convert to const char* for glfwSetWindowTitle
		// If you want to show ms instead of fps change to "(double)1000/(double)(nbFrames);"
		d = (double) (nbFrames);
		std::stringstream ss;
		ss << d;
		const char* str = ss.str().c_str();

		glfwSetWindowTitle(pWindow, str);

		// counter zero, count until next second is finished
		nbFrames = 0;
		initTime += 1.0;
	}
	return d;
}

void RetinaManager::loadSettings() {
	FILE * settings = fopen("settings.txt", "r");

	if (settings == NULL) {
		printf("not able to open the file !\n");
	}

	int temp_updateInterval;
	float temp_translateBack_Offset;
	float temp_Viewport_Offset;
	float temp_cDecay;

	fscanf(settings, "updateInterval: %i \n"
			"cDecay: %f \n"
			"translateBack: %f \n"
			"viewport: %f \n ", &temp_updateInterval, &temp_cDecay, &temp_translateBack_Offset, &temp_Viewport_Offset);

	paramManager.setUpdateInterval(temp_updateInterval);
	paramManager.setTranslateBackOffset(temp_translateBack_Offset);
	paramManager.setViewportOffset(temp_Viewport_Offset);
	paramManager.setDecay(temp_cDecay);

	fclose(settings);

}

void RetinaManager::writeSettings() {

	FILE * settings = fopen("settings.txt", "w");

	if (settings == NULL) {
		printf("not able to open the file !\n");

	}
	int temp_updateInterval;
	float temp_translateBack_Offset;
	float temp_Viewport_Offset;
	float temp_cDecay;

	temp_updateInterval = paramManager.getUpdateInterval();
	temp_translateBack_Offset = paramManager.getTranslateBackOffset();
	temp_Viewport_Offset = paramManager.getViewportOffset();
	temp_cDecay = paramManager.getDecay();

	fprintf(settings, "updateInterval: %i \n"
			"cDecay: %f \n"
			"translateBack: %f \n"
			"viewport: %f \n ", temp_updateInterval, temp_cDecay, temp_translateBack_Offset, temp_Viewport_Offset);

	fclose(settings);
}

void RetinaManager::writeEventsToFile(FILE * file, edvs_event_t* event, int eventNum) {

	glm::vec4 tempEvent;
	for (int i = 0; i < eventNum; i++) {

		tempEvent.x = event[i].x;
		tempEvent.y = 127 - event[i].y;
		tempEvent.z = event[i].parity;
		tempEvent.w = event[i].t;

		if (file == NULL) {
			printf("not able to open the file !\n");

		}

		fprintf(file, "%i %i %i %lu;\n", (int) tempEvent.x, (int) tempEvent.y, (int) tempEvent.z,
				(unsigned long) tempEvent.w);
	}
}

void RetinaManager::UpdateEvents(int eyeIndex) {
	// Read events in different modes

	if (RetinaManager::paramManager.getMode() == 0 || RetinaManager::paramManager.getMode() == 1
			|| RetinaManager::paramManager.getMode() == 4) {
		RetinaManager::ReadEventsFromSensor();
	}
	if (RetinaManager::paramManager.getMode() == 1 || RetinaManager::paramManager.getMode() == 4) {
		RetinaManager::writeEventsToFile(RetinaManager::edvsFile[0], RetinaManager::events[0],
				RetinaManager::eventNum[eyeIndex]);
	}

	switch (RetinaManager::paramManager.getMode()) {
		case 0: {
			RetinaManager::eDVS[eyeIndex]->updateEvent(RetinaManager::events[eyeIndex],
					RetinaManager::eventNum[eyeIndex], RetinaManager::paramManager.getUpdateInterval());
			break;
		}

		case 1: {
			RetinaManager::eDVS[eyeIndex]->updateEvent(RetinaManager::events[eyeIndex],
					RetinaManager::eventNum[eyeIndex], RetinaManager::paramManager.getUpdateInterval());
			break;
		}
		case 2: {
			if (!RetinaManager::eDVS[eyeIndex]->eof()) {
				RetinaManager::eDVS[eyeIndex]->updateEvent(RetinaManager::edvsFile[eyeIndex],
						RetinaManager::paramManager.getUpdateInterval(),
						RetinaManager::paramManager.getDisplayInterval());
			} else {
				fclose(RetinaManager::edvsFile[eyeIndex]);
				RetinaManager::edvsFile[eyeIndex] = NULL; // sadly, fclose does not do this automatically.
			}
			break;

		}
		case 4: {
			RetinaManager::eDVS[eyeIndex]->updateEvent(RetinaManager::events[eyeIndex],
					RetinaManager::eventNum[eyeIndex], RetinaManager::paramManager.getUpdateInterval());
			break;
		}
	}
}

/* *************************************************************************
 * This function reads Events from edvs-sensors and stores them into
 ***************************************************************************/
void RetinaManager::ReadEventsFromSensor() {
	// Obtain event stream from eDVS
	RetinaManager::eventNum[0] = edvs_read(RetinaManager::streamHandle[0], RetinaManager::events[0],
			RetinaManager::num_max_events);
	RetinaManager::eventNum[1] = edvs_read(RetinaManager::streamHandle[1], RetinaManager::events[1],
			RetinaManager::num_max_events);
}

glm::mat4 RetinaManager::CalcTransMatrix(ovrEyeType Eye) {
	glm::mat4 projMat;
	glm::mat4 modelViewMat;
	// Get Projection and ModelView matrices from the device
	OVR::Matrix4f projectionMatrix = ovrMatrix4f_Projection(RetinaManager::eyeRenderDesc[Eye].Fov, 0.3f, 1000.0f, true);

	// Convert the matrices into OpenGl form
	memcpy(glm::value_ptr(projMat), &(projectionMatrix.Transposed().M[0][0]), sizeof(projectionMatrix));
	modelViewMat = glm::mat4(1.0); //Identity matrix for model-view
	// Adjust IPD and the distance from FOV
	glm::mat4 translateIPD = glm::translate(glm::mat4(1.0),
			glm::vec3(RetinaManager::eyeRenderDesc[Eye].ViewAdjust.x, RetinaManager::eyeRenderDesc[Eye].ViewAdjust.y,
					RetinaManager::eyeRenderDesc[Eye].ViewAdjust.z));

	glm::mat4 translateBack = glm::translate(glm::mat4(1.0),
			glm::vec3(0, 0, RetinaManager::paramManager.getTranslateBackOffset()));

	// Calc and Return the transformed Mat
	return projMat * modelViewMat * translateBack * translateIPD;;
}

void RetinaManager::renderOvrEyes() {
	ovrHmd_BeginFrame((RetinaManager::hmd), 0);
	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
		ovrEyeType Eye = RetinaManager::hmdDesc.EyeRenderOrder[eyeIndex];
		ovrPosef eyePose = ovrHmd_BeginEyeRender(RetinaManager::hmd, Eye);

		glViewport(
				RetinaManager::eyeTexture[Eye].OGL.Header.RenderViewport.Pos.x
						+ getSignForEye(eyeIndex) * RetinaManager::paramManager.getViewportOffset(),
				RetinaManager::eyeTexture[Eye].OGL.Header.RenderViewport.Pos.y,
				RetinaManager::eyeTexture[Eye].OGL.Header.RenderViewport.Size.w,
				RetinaManager::eyeTexture[Eye].OGL.Header.RenderViewport.Size.h);

		glm::mat4 transMatrix = RetinaManager::CalcTransMatrix(Eye);

		// Use the shader
		glUseProgram(RetinaManager::programID);
		glUniformMatrix4fv(RetinaManager::MatrixID, 1, GL_FALSE, &transMatrix[0][0]);

		// Draw the events
		RetinaManager::eDVS[eyeIndex]->draw(RetinaManager::paramManager.getDecay());

		ovrHmd_EndEyeRender((RetinaManager::hmd), Eye, eyePose, &(RetinaManager::eyeTexture[Eye]).Texture);
	}
	ovrHmd_EndFrame((RetinaManager::hmd));
}

void RetinaManager::render() {

	RetinaManager::measureFPS();

	// Bind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, RetinaManager::FBOId);
	// Clear the background
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// OVR RENDERING
	RetinaManager::renderOvrEyes();

	// Swap buffers
	glfwSwapBuffers(RetinaManager::pWindow);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Clean up
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	return;
}

FileAndWindowStateType RetinaManager::getFileAndWindowState() {
	FileAndWindowStateType tempState = Default;
	glfwPollEvents();
	if (RetinaManager::paramManager.getMode() == 4 && RetinaManager::isTimeElapsed()) {
		tempState = tempState | RecordTimeElapsed;
	} else {
		tempState = tempState & ~RecordTimeElapsed;
	}
	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(pWindow)) {
		tempState = tempState | CloseWindowRequest;
	} else {
		tempState = tempState & ~CloseWindowRequest;
	}

	return tempState;
}

int RetinaManager::Initialize(int initModeViaKeyboard) {
	// Init File Names
	RetinaManager::edvsFileName_left = DEFAULT_EDVSDATA_LEFT_FILENAME;
	RetinaManager::edvsFileName_right = DEFAULT_EDVSDATA_RIGHT_FILENAME;

	//TODO: ned hier
	this->useOculus = false;

	if (initModeViaKeyboard) {
		int nTries = 0;
		printf("Please choose the mode: \n"
				"0: Read from devices, no recording \n"
				"1: Read from devices, recording \n"
				"2: Read from files \n"
				"3: Experiment - Not implemented yet \n"
				"4: 10 sec record \n"
				"mode:");
		int tempMode = -1;
		do {
			scanf("%i", &(tempMode));
			nTries += 1;
		} while (!(tempMode >= 0 && tempMode <= 4 && nTries < 5));
		if (nTries >= 5) tempMode = DEFAULT_MODE;

		RetinaManager::setMode(tempMode);
	} //ELSE - paramManager.mode must be initialized ELSEWHERE!

	// ********************* Initialize OVR ******************************
	// Initializes LibOVR.
	ovr_Initialize();
	RetinaManager::hmd = ovrHmd_Create(0);

	if (!RetinaManager::hmd) {
		// If we didn't detect an Hmd, create a simulated one for debugging.
		if (OCULUS_DK_VERSION_DEBUG == 2) {
			RetinaManager::hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
		} else {
			RetinaManager::hmd = ovrHmd_CreateDebug(ovrHmd_DK1);
		}
		if (!RetinaManager::hmd) {   // Failed Hmd creation.
			return 1;
		}
	}
	// Get more details about the HMD
	ovrHmd_GetDesc(RetinaManager::hmd, &(RetinaManager::hmdDesc));

	// Start the sensor which provides the Rift’s pose and motion.
	ovrHmd_StartSensor(RetinaManager::hmd,
			ovrSensorCap_Orientation | ovrSensorCap_YawCorrection | ovrSensorCap_Position | ovrHmdCap_LowPersistence,
			ovrSensorCap_Orientation);

	ovrSizei clientSize;
	clientSize.w = RetinaManager::hmdDesc.Resolution.w;
	clientSize.h = RetinaManager::hmdDesc.Resolution.h;

	// Configure Stereo settings.
	ovrSizei texSizeLeft = ovrHmd_GetFovTextureSize(RetinaManager::hmd, ovrEye_Left,
			RetinaManager::hmdDesc.DefaultEyeFov[0], 1.0f);
	ovrSizei texSizeRight = ovrHmd_GetFovTextureSize(RetinaManager::hmd, ovrEye_Right,
			RetinaManager::hmdDesc.DefaultEyeFov[1], 1.0f);
	ovrSizei texSize;
	texSize.w = texSizeLeft.w + texSizeRight.w;
	texSize.h = (texSizeLeft.h > texSizeRight.h ? texSizeRight.h : texSizeRight.h);
	// ******************** /InitializeT OVR ******************************

	// ******************** Initialize GLFW *****************************
	if (!glfwInit()) {
		fprintf( stderr, "Failed to initialize GLFW\n");
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a pWindow and create its OpenGL context
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	GLFWmonitor *monitor;
	if (count > 1) { // This means, there is a second monitor
		monitor = monitors[count - 1]; // this will make Fullscreen paramManager.mode on last monitor. This should be the Occulus!
		printf("\n\n");
		printf(glfwGetMonitorName(monitor));
		printf("\n\n");
	} else {
		monitor = NULL;
	}
	// Note: The second last param specifies wether to open a new pWindow (NULL) or create the pWindow in full screen paramManager.mode
	// at the specified pWindow.
	if (RetinaManager::isUsingOculus()) {
		pWindow = glfwCreateWindow(clientSize.w, clientSize.h, "eDVS: FPS=0", monitor, NULL);
	} else {
		pWindow = glfwCreateWindow(clientSize.w, clientSize.h, "eDVS: FPS=0", NULL, NULL);
	}

	if (pWindow == NULL) {
		fprintf( stderr, "Failed to open GLFW pWindow. Make sure your graphic card supports OpenGL 3.3+. \n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(pWindow);
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(pWindow, GLFW_STICKY_KEYS, GL_TRUE);

	// Print OpenGL version information
	int Major = glfwGetWindowAttrib(pWindow, GLFW_CONTEXT_VERSION_MAJOR);
	int Minor = glfwGetWindowAttrib(pWindow, GLFW_CONTEXT_VERSION_MINOR);
	int Profile = glfwGetWindowAttrib(pWindow, GLFW_OPENGL_PROFILE);
	printf("OpenGL: %d.%d ", Major, Minor);
	if (Profile == GLFW_OPENGL_COMPAT_PROFILE)
		printf("GLFW_OPENGL_COMPAT_PROFILE\n");
	else
		printf("GLFW_OPENGL_CORE_PROFILE\n");
	printf("Vendor: %s\n", (char*) glGetString(GL_VENDOR));
	printf("Renderer: %s\n", (char*) glGetString(GL_RENDERER));
	// ******************* /Initialize GLFW *****************************

	// ******************* Initialize GLEW *********************************
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
	// ****************** /Initialize GLEW *********************************

	// Create the texture we're going to render to...
	GLuint texId;
	glGenTextures(1, &texId);
	// "Bind" the newly created texture : all future texture functions will modify this texture...
	glBindTexture(GL_TEXTURE_2D, texId);
	// Give an empty image to OpenGL (the last "0")
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize.w, texSize.h, 0, GL_RGBA,
	GL_UNSIGNED_BYTE, 0);
	// Linear filtering...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// *********************** Create BUFFERS ************************************
	// Create FBO
	glGenFramebuffers(1, &(RetinaManager::FBOId));
	glBindFramebuffer(GL_FRAMEBUFFER, RetinaManager::FBOId);
	// Create Depth Buffer
	GLuint depthBufferId;
	glGenRenderbuffers(1, &depthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texSize.w, texSize.h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferId);
	// Create vertexshader buffer
	glGenBuffers(1, &(RetinaManager::vertexbuffer));
	glGenBuffers(1, &(RetinaManager::colorbuffer));

	// Create VertexArray
	glGenVertexArrays(1, &(RetinaManager::VertexArrayID));
	glBindVertexArray(RetinaManager::VertexArrayID);

	// Create and compile GLSL program from the shaders
	RetinaManager::programID = LoadShaders("src/shader/vertexshader.txt", "src/shader/fragmentshader.txt");
	RetinaManager::MatrixID = glGetUniformLocation(RetinaManager::programID, "transMatrix");
	// ********************** /Create BUFFERS ************************************

	// Set the texture as our colour attachment #0...
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texId, 0);
	// Set the list of draw buffers...
	GLenum GLDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, GLDrawBuffers); // "1" is the size of DrawBuffers

	// Check if everything is OK...
	GLenum Check = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (Check != GL_FRAMEBUFFER_COMPLETE) {
		printf("There is a problem with the FBO.\n");
		exit(EXIT_FAILURE);
	}

	// Unbind...
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ******************* OVR eye configurations ************************
	RetinaManager::eyeFov[0] = RetinaManager::hmdDesc.DefaultEyeFov[0];
	RetinaManager::eyeFov[1] = RetinaManager::hmdDesc.DefaultEyeFov[1];

	RetinaManager::Cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	RetinaManager::Cfg.OGL.Header.Multisample = (1);
	RetinaManager::Cfg.OGL.Header.RTSize.w = clientSize.w;
	RetinaManager::Cfg.OGL.Header.RTSize.h = clientSize.h;

	int distortionCaps = ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp;
	ovrHmd_ConfigureRendering(RetinaManager::hmd, &(RetinaManager::Cfg.Config), distortionCaps, RetinaManager::eyeFov,
			RetinaManager::eyeRenderDesc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	RetinaManager::eyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
	RetinaManager::eyeTexture[0].OGL.Header.TextureSize.w = texSize.w;
	RetinaManager::eyeTexture[0].OGL.Header.TextureSize.h = texSize.h;
	RetinaManager::eyeTexture[0].OGL.Header.RenderViewport.Pos.x = 0;
	RetinaManager::eyeTexture[0].OGL.Header.RenderViewport.Pos.y = 0;
	RetinaManager::eyeTexture[0].OGL.Header.RenderViewport.Size.w = texSize.w / 2;
	RetinaManager::eyeTexture[0].OGL.Header.RenderViewport.Size.h = texSize.h;
	RetinaManager::eyeTexture[0].OGL.TexId = texId;

	RetinaManager::eyeTexture[1] = RetinaManager::eyeTexture[0];
	RetinaManager::eyeTexture[1].OGL.Header.RenderViewport.Pos.x = (texSize.w + 1) / 2;
	// ******************* /OVR eye configurations ************************

	// *********************** Initialize eDVS *************************************
	this->CreateEDVSGL();
	// *********************** /Initialize eDVS *************************************

	RetinaManager::control = Stop;
	return 1;
}

void RetinaManager::setMode(int mode) {
	//RetinaManager::setControl(STOP);
	//RetinaManager::setFile(RetinaManager::edvsFileName);
	//myMutex.lock(); // Hier und in server.cpp geht natürlich nicht (wäre doppelter lock)
	switch (mode) {
		case 0: {

			//Initialize eDVS
			RetinaManager::streamHandle[0] = edvs_open("/dev/ttyUSB0?baudrate=4000000");
			RetinaManager::streamHandle[1] = edvs_open("/dev/ttyUSB1?baudrate=4000000");

			RetinaManager::events[0] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
			RetinaManager::events[1] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
			break;
		}

		case 1: {
			//Initialize eDVS

			RetinaManager::streamHandle[0] = edvs_open("/dev/ttyUSB0?baudrate=4000000");
			RetinaManager::streamHandle[1] = edvs_open("/dev/ttyUSB1?baudrate=4000000");

			RetinaManager::events[0] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
			RetinaManager::events[1] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
			RetinaManager::edvsFile[0] = fopen(RetinaManager::getEdvsFileNameLeft(), "w");
			RetinaManager::edvsFile[1] = fopen(RetinaManager::getEdvsFileNameRight(), "w");
			break;
		}
		case 2: {
			RetinaManager::edvsFile[0] = fopen(RetinaManager::getEdvsFileNameLeft(), "r");
			RetinaManager::edvsFile[1] = fopen(RetinaManager::getEdvsFileNameRight(), "r");
			break;
		}
		case 3: {
			printf("Not implemented yet");
			break;
		}
		case 4: {
			RetinaManager::streamHandle[0] = edvs_open("/dev/ttyUSB0?baudrate=4000000");
			RetinaManager::streamHandle[1] = edvs_open("/dev/ttyUSB1?baudrate=4000000");

			RetinaManager::events[0] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
			RetinaManager::events[1] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
			RetinaManager::edvsFile[0] = fopen(RetinaManager::getEdvsFileNameLeft(), "w");
			RetinaManager::edvsFile[1] = fopen(RetinaManager::getEdvsFileNameRight(), "w");
			break;
		}
	}
	RetinaManager::paramManager.setMode(mode);


	// this->CreateEDVSGL(); //TODO: Darf hier nicht stehen, gibt bei der Init Probleme, weil es zu früh aufgerufen wird, bevor
	// verexbuffer etc gesetzt ist. Frage ist ob es für später hier NÖTIG ist. --> TESTEN!
	//myMutex.unlock(); // Hier und in server.cpp geht natürlich nicht (wäre doppelter lock)
}

void RetinaManager::TerminateWindow() {
	glDeleteBuffers(1, &(RetinaManager::vertexbuffer));
	glDeleteVertexArrays(1, &(RetinaManager::VertexArrayID));
	glDeleteProgram(RetinaManager::programID);

	// Shutdown Oculusrift
	ovrHmd_Destroy(RetinaManager::hmd);
	ovr_Shutdown();

	//Cleanup eDVS
	edvs_close(RetinaManager::streamHandle[0]);
	edvs_close(RetinaManager::streamHandle[1]);
	free(RetinaManager::events[0]);
	free(RetinaManager::events[1]);

	// Close OpenGL pWindow and terminate GLFW
	glfwTerminate();
}

void RetinaManager::KeyControl() {

	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_UP) == GLFW_PRESS) {
		RetinaManager::paramManager.incTranslateBackOffset();
	}

	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
		RetinaManager::paramManager.decTranslateBackOffset();
	}

	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		RetinaManager::paramManager.decViewportOffset();
	}

	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
		RetinaManager::paramManager.incViewportOffset();
	}

	if ((glfwGetKey(RetinaManager::pWindow, GLFW_KEY_W) == GLFW_PRESS)
			& (RetinaManager::paramManager.getDecay() < 0.5)) {
		RetinaManager::paramManager.incDecay();
	}

	if ((glfwGetKey(RetinaManager::pWindow, GLFW_KEY_S) == GLFW_PRESS) & (RetinaManager::paramManager.getDecay() > 0)) {
		RetinaManager::paramManager.decDecay();
	}

	if ((glfwGetKey(RetinaManager::pWindow, GLFW_KEY_D) == GLFW_PRESS)
			& (RetinaManager::paramManager.getUpdateInterval() > 1000)) {
		RetinaManager::paramManager.decUpdateInterval();
	}

	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_PAUSE) == GLFW_PRESS) {
		RetinaManager::setControl(PAUSE);
	}
	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_P) == GLFW_PRESS) {
		RetinaManager::setControl(PLAY);
	}
	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_X) == GLFW_PRESS) {
		RetinaManager::setControl(STOP);
	}

	if ((glfwGetKey(RetinaManager::pWindow, GLFW_KEY_A) == GLFW_PRESS)
			& (RetinaManager::paramManager.getUpdateInterval() < 20000000)) {
		RetinaManager::paramManager.incUpdateInterval();
	}

	if ((glfwGetKey(RetinaManager::pWindow, GLFW_KEY_F1) == GLFW_PRESS)) {
		RetinaManager::writeSettings();
	}

	if ((glfwGetKey(RetinaManager::pWindow, GLFW_KEY_F2) == GLFW_PRESS)) {
		RetinaManager::loadSettings();
	}

	if ((glfwGetKey(RetinaManager::pWindow, GLFW_KEY_O) == GLFW_PRESS)) {
		RetinaManager::tryToUseOculus();
	}
}

void RetinaManager::setRedGreen(char *colorVal) {
	glm::vec3 midColor;
	glm::vec3 onColor;
	glm::vec3 offColor;
	if (atoi(colorVal)) {
		midColor = glm::vec3(0.5f, 0.5f, 0.5f);
		onColor = glm::vec3(1.0f, 0.0f, 0.0f);
		offColor = glm::vec3(0.0f, 1.0f, 0.0f);
	} else {
// TODO: 1. CHANGE 2nd color, 2nd. MAKE DEFINES? (2. not necissary)...
		midColor = glm::vec3(0.5f, 0.5f, 0.5f);
		onColor = glm::vec3(1.0f, 1.0f, 1.0f);
		offColor = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	this->getParamManager().setMidColor(midColor);
	this->getParamManager().setOnColor(onColor);
	this->getParamManager().setOffColor(offColor);
}

int RetinaManager::setFile(char *filename) {

	std::string edvsFileNameS_left;
	std::string edvsFileNameS_right;
	char *edvsFileName_left;
	char *edvsFileName_right;

	// Cast from char * to string, then concatenating, then cast back to char* (yes, strcat would have also worked)
	edvsFileNameS_left = (std::string) EDVS_DATA_FOLDER_NAME + (std::string) filename
			+ (std::string) FILENAME_EXTENSION_LEFT;
	edvsFileNameS_right = (std::string) EDVS_DATA_FOLDER_NAME + (std::string) filename
			+ (std::string) FILENAME_EXTENSION_RIGHT;

	edvsFileName_left = (char *) edvsFileNameS_left.c_str();
	edvsFileName_right = (char *) edvsFileNameS_right.c_str();

	// myMutex.lock(); // Hier und in server.cpp geht natürlich nicht (wäre doppelter lock)
	// set corresponding left and right filename
	RetinaManager::setEdvsFileNameRight(edvsFileName_right);
	RetinaManager::setEdvsFileNameLeft(edvsFileName_left);

	// Close old file
	if (this->edvsFile[0] != NULL) {
		fclose(this->edvsFile[0]);
		RetinaManager::edvsFile[0] = NULL;
	}
	if (this->edvsFile[1] != NULL) {
		fclose(this->edvsFile[1]);
		RetinaManager::edvsFile[0] = NULL;
	}
	switch (RetinaManager::paramManager.getMode()) {
		case 1: {
			RetinaManager::edvsFile[0] = fopen(edvsFileName_left, "w");
			RetinaManager::edvsFile[1] = fopen(edvsFileName_right, "w");
			break;
		}
		case 2: {
			RetinaManager::edvsFile[0] = fopen(edvsFileName_left, "r");
			RetinaManager::edvsFile[1] = fopen(edvsFileName_right, "r");
			break;
		}
		case 3: {
			printf("Not implemented yet");
			break;
		}
		case 4: {
			RetinaManager::edvsFile[0] = fopen(edvsFileName_left, "w");
			RetinaManager::edvsFile[1] = fopen(edvsFileName_right, "w");
			break;
		}
	}

	this->CreateEDVSGL();
	// myMutex.unlock(); // Hier und in server.cpp geht natürlich nicht (wäre doppelter lock)
	if (RetinaManager::edvsFile[0] != NULL && RetinaManager::edvsFile[1] != NULL) {
		return 1;
	} else {
		return 0;
	}
}

void RetinaManager::setControl(char *control) {
	// myMutex.lock(); // Hier und in server.cpp geht natürlich nicht (wäre doppelter lock)
	if (strcmp(control, PLAY) == 0) {

		if (this->control == Stop) this->setFile("edvs"); // Reinitializes eDVSGL objects (for next Play)
		this->control = Play;
	} else if (strcmp(control, PAUSE) == 0) {
		this->control = Pause;
	} else if (strcmp(control, STOP) == 0) {
		this->control = Stop;
	}
	// myMutex.unlock(); // Hier und in server.cpp geht natürlich nicht (wäre doppelter lock)
}

void RetinaManager::CreateEDVSGL() {
	// *********************** Initialize eDVS *************************************
	if (RetinaManager::eDVS[0] != NULL) {
		delete (RetinaManager::eDVS[0]);
	}
	RetinaManager::eDVS[0] = new eDVSGL;
	RetinaManager::eDVS[0]->initialize(RetinaManager::paramManager.getMidColor(),
			RetinaManager::paramManager.getOnColor(), paramManager.getOffColor());
	RetinaManager::eDVS[0]->setGL(pWindow, RetinaManager::vertexbuffer, RetinaManager::colorbuffer,
			RetinaManager::programID);

	if (RetinaManager::eDVS[1] != NULL) {
		delete (RetinaManager::eDVS[1]);
	}
	RetinaManager::eDVS[1] = new eDVSGL;
	RetinaManager::eDVS[1]->initialize(RetinaManager::paramManager.getMidColor(),
			RetinaManager::paramManager.getOnColor(), paramManager.getOffColor());
	RetinaManager::eDVS[1]->setGL(pWindow, RetinaManager::vertexbuffer, RetinaManager::colorbuffer,
			RetinaManager::programID);
	// *********************** /Initialize eDVS *************************************
}

// TODO: Typ ändern
FileAndWindowStateType RetinaManager::getFileState() {
	FileAndWindowStateType tempState = Default;
	if (RetinaManager::eDVS[0]->eof() || RetinaManager::eDVS[1]->eof()) {
		tempState = tempState | EndOfFile;
	} else {
		tempState = tempState & ~EndOfFile;
	}
	return tempState;
}

int RetinaManager::tryToUseOculus() {
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	GLFWmonitor *monitor;
	if (count > 1) { // This means, there is a second monitor
		monitor = monitors[count - 1]; // this will make Fullscreen paramManager.mode on last monitor. This should be the Occulus!
		printf("\n\n");
		printf(glfwGetMonitorName(monitor));
		printf("\n\n");
		this->useOculus = true;
	} else {
		monitor = NULL;
		this->useOculus = false;
	}
	ovrSizei clientSize;
	clientSize.w = RetinaManager::hmdDesc.Resolution.w;
	clientSize.h = RetinaManager::hmdDesc.Resolution.h;
	// Note: The second last param specifies wether to open a new pWindow (NULL) or create the pWindow in full screen paramManager.mode
	// at the specified pWindow.
	if (this->useOculus) {
		pWindow = glfwCreateWindow(clientSize.w, clientSize.h, "eDVS: FPS=0", monitor, NULL);
	} else {
		pWindow = glfwCreateWindow(clientSize.w, clientSize.h, "eDVS: FPS=0", NULL, NULL);
	}

	if (pWindow == NULL) {
		fprintf( stderr, "Failed to open GLFW pWindow. Make sure your graphic card supports OpenGL 3.3+. \n");
		glfwTerminate();
		return 0;
	}
	glfwMakeContextCurrent(pWindow);
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(pWindow, GLFW_STICKY_KEYS, GL_TRUE);

	return this->useOculus;
}
