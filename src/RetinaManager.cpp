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
#include <map>
#include "main.h"

#define getSignForEye(x) ((x)==0 ? (1) : (-1))

// For printing messages during debugging
#define DEBUG //Uncomment this in order to stop debug messages
#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

// ********************************************************************************************************
// Sadly, AFAIK GLFW_BUTTON_UP or anything similar does not exist... Only GLFW_PRESS exists,
// which is always true as long as the button is pressed, and not only once. Therefore create it manually...
std::map<int, int> KeyMap;
int getOldKeyState(int key) {
	std::map<int, int>::iterator iterator;
	iterator = KeyMap.find(key);
	if (iterator != KeyMap.end()) {
		return iterator->second;
	} else {
		return -1;
	}
}
void setKeyState(int key, int val) {
	std::map<int, int>::iterator iterator;
	iterator = KeyMap.find(key);
	if (iterator != KeyMap.end()) {
		iterator->second = val;
	} else {
		KeyMap.insert(std::pair<int, int>(key, val));
	}
}
// ********************************************************************************************************

RetinaManager::RetinaManager() {
	this->useOculus = false;
	isInitialized = false;
}

RetinaManager::~RetinaManager() {
	// TODO Auto-generated destructor stub
}

int RetinaManager::isTimeElapsed() {
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
	this->recordCounter = this->recordCounter + 1;
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
//BIS HIER
void RetinaManager::UpdateEvents() {
	// Read events in different modes
	if (RetinaManager::paramManager.getMode() == 0 || RetinaManager::paramManager.getMode() == 1
			|| RetinaManager::paramManager.getMode() == 4) {
		RetinaManager::ReadEventsFromSensor();
	}

	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
		if (RetinaManager::paramManager.getMode() == 1 || RetinaManager::paramManager.getMode() == 4) {
			RetinaManager::writeEventsToFile(RetinaManager::edvsFile[eyeIndex], RetinaManager::events[eyeIndex],
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
					RetinaManager::edvsFile[eyeIndex] = NULL;
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

int RetinaManager::initGL(ovrSizei clientSize, ovrSizei texSize, GLFWmonitor *monitor) {
	//if(&(RetinaManager::depthBufferId)!=NULL)   glDeleteRenderbuffers(1, &(RetinaManager::depthBufferId));
	//if(&(RetinaManager::FBOId)!=NULL)	glDeleteFramebuffers(1, &(RetinaManager::FBOId));
	//if(&(RetinaManager::colorbuffer)!=NULL)	glDeleteBuffers(1, &(RetinaManager::colorbuffer));
	//if(&(RetinaManager::vertexbuffer)!=NULL)	glDeleteBuffers(1, &(RetinaManager::vertexbuffer));
	//if(&(RetinaManager::VertexArrayID)!=NULL)	glDeleteVertexArrays(1, &(RetinaManager::VertexArrayID));
	if (&(RetinaManager::texId) != NULL) glDeleteTextures(1, &(RetinaManager::texId));

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

	//glfwDestroyWindow(RetinaManager::pWindow);
	//RetinaManager::pWindow = glfwCreateWindow(clientSize.w, clientSize.h, "eDVS: FPS=0", NULL, NULL);

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
	glfwSetInputMode(pWindow, GLFW_STICKY_KEYS, GL_TRUE);
	// ******************** /Initialize GLFW *****************************

	// ******************* Initialize GLEW *********************************
	glGenTextures(1, &(RetinaManager::texId));
	// "Bind" the newly created texture : all future texture functions will modify this texture...
	glBindTexture(GL_TEXTURE_2D, (RetinaManager::texId));
	// Give an empty image to OpenGL (the last "0")
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize.w, texSize.h, 0, GL_RGBA,
	GL_UNSIGNED_BYTE, 0);
	// Linear filtering...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	glGenFramebuffers(1, &(RetinaManager::FBOId));
	glBindFramebuffer(GL_FRAMEBUFFER, RetinaManager::FBOId);
	// Create Depth Buffer
	glGenRenderbuffers(1, &(RetinaManager::depthBufferId));
	glBindRenderbuffer(GL_RENDERBUFFER, RetinaManager::depthBufferId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texSize.w, texSize.h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RetinaManager::depthBufferId);
	// Create vertexshader buffer
	glGenBuffers(1, &(RetinaManager::vertexbuffer));
	glGenBuffers(1, &(RetinaManager::colorbuffer));

	// Create VertexArray
	glGenVertexArrays(1, &(RetinaManager::VertexArrayID));
	glBindVertexArray(RetinaManager::VertexArrayID);

	// Create and compile GLSL program from the shaders
	RetinaManager::programID = LoadShaders("src/shader/vertexshader.txt", "src/shader/fragmentshader.txt");
	RetinaManager::MatrixID = glGetUniformLocation(RetinaManager::programID, "transMatrix");

	// Set the texture as our colour attachment #0...
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texId, 0);
	// Set the list of draw buffers...
	GLenum GLDrawBuffers2[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, GLDrawBuffers2); // "1" is the size of DrawBuffers
	// Check if everything is OK...
	GLenum Check2 = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (Check2 != GL_FRAMEBUFFER_COMPLETE) {
		printf("There is a problem with the FBO.\n");
		exit(EXIT_FAILURE);
	}
	// ****************** /Initialize GLEW *********************************

	// ******************* OVR eye configurations ************************
	RetinaManager::eyeFov[0] = RetinaManager::hmdDesc.DefaultEyeFov[0];
	RetinaManager::eyeFov[1] = RetinaManager::hmdDesc.DefaultEyeFov[1];

	RetinaManager::Cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	RetinaManager::Cfg.OGL.Header.Multisample = (1);
	RetinaManager::Cfg.OGL.Header.RTSize.w = clientSize.w;
	RetinaManager::Cfg.OGL.Header.RTSize.h = clientSize.h;

	int distortionCaps2 = ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp;
	ovrHmd_ConfigureRendering(RetinaManager::hmd, &(RetinaManager::Cfg.Config), distortionCaps2, RetinaManager::eyeFov,
			RetinaManager::eyeRenderDesc);

	RetinaManager::eyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
	RetinaManager::eyeTexture[0].OGL.Header.TextureSize.w = texSize.w;
	RetinaManager::eyeTexture[0].OGL.Header.TextureSize.h = texSize.h;
	RetinaManager::eyeTexture[0].OGL.Header.RenderViewport.Pos.x = 0;
	RetinaManager::eyeTexture[0].OGL.Header.RenderViewport.Pos.y = 0;
	RetinaManager::eyeTexture[0].OGL.Header.RenderViewport.Size.w = texSize.w / 2;
	RetinaManager::eyeTexture[0].OGL.Header.RenderViewport.Size.h = texSize.h;
	RetinaManager::eyeTexture[0].OGL.TexId = RetinaManager::texId;

	RetinaManager::eyeTexture[1] = RetinaManager::eyeTexture[0];
	RetinaManager::eyeTexture[1].OGL.Header.RenderViewport.Pos.x = (texSize.w + 1) / 2;
	// ******************* /OVR eye configurations ************************
	return 0;
}

int RetinaManager::Initialize(int initModeViaKeyboard) {
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
	} else {	//ELSE - paramManager.mode must be initialized ELSEWHERE!
		DEBUG_MSG(
				"Initialized with mode " << ERROR_MODE << ",because boolean initViaKeyboard was set to false in init");
		RetinaManager::setMode(ERROR_MODE);
	}
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
	// ********************* Initialize GL *****************************************
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	GLFWmonitor *monitor;
	if (count > 1 && RetinaManager::useOculus == true) { // This means, there is a second monitor
		monitor = monitors[count - 1]; // this will make Fullscreen paramManager.mode on last monitor. This should be the Occulus!
		DEBUG_MSG("'Last' monitor is " << glfwGetMonitorName(monitor) <<". This should be Oculus Rift");
	} else {
		monitor = NULL;
	}

	initGL(clientSize, texSize, monitor);
	// ********************* /Initialize GL ****************************************

	RetinaManager::control = Stop;
	isInitialized = true;
	return 1;
}

int RetinaManager::setMode(int mode) {
	DEBUG_MSG("Changing mode from mode " << RetinaManager::paramManager.getMode() << " to " << mode);
	RetinaManager::setControl(STOP);
	switch (mode) {
		case 0: {
			//Initialize eDVS
			RetinaManager::streamHandle[0] = edvs_open("/dev/ttyUSB0?baudrate=4000000");
			RetinaManager::streamHandle[1] = edvs_open("/dev/ttyUSB1?baudrate=4000000");
			if (RetinaManager::streamHandle[0] != 0 && RetinaManager::streamHandle[1] != 0) {
				RetinaManager::events[0] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
				RetinaManager::events[1] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
			} else {
				mode = ERROR_MODE;
			}
			break;
		}

		case 1: {
			//Initialize eDVS
			RetinaManager::streamHandle[0] = edvs_open("/dev/ttyUSB0?baudrate=4000000");
			RetinaManager::streamHandle[1] = edvs_open("/dev/ttyUSB1?baudrate=4000000");
			if (RetinaManager::streamHandle[0] != 0 && RetinaManager::streamHandle[1] != 0) {
				RetinaManager::events[0] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
				RetinaManager::events[1] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
			} else {
				mode = ERROR_MODE;
			}
			break;
		}
		case 2: {
			// Nothing needed here.
			break;
		}
		case 3: {
			DEBUG_MSG("Not implemented yet");
			mode = ERROR_MODE;
			break;
		}
		case 4: {
			RetinaManager::streamHandle[0] = edvs_open("/dev/ttyUSB0?baudrate=4000000");
			RetinaManager::streamHandle[1] = edvs_open("/dev/ttyUSB1?baudrate=4000000");
			if (RetinaManager::streamHandle[0] != 0 && RetinaManager::streamHandle[1] != 0) {
				RetinaManager::events[0] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
				RetinaManager::events[1] = (edvs_event_t*) malloc(RetinaManager::num_max_events * sizeof(edvs_event_t));
			} else {
				mode = ERROR_MODE;
			}
			break;
		}
		default: {
			mode = ERROR_MODE;
		}
	}

	RetinaManager::paramManager.setMode(mode);
	if (mode != ERROR_MODE) {
		DEBUG_MSG("mode was succesfully set to " << RetinaManager::paramManager.getMode());
		return 1;
	} else { // mode == ERROR_MODE
		DEBUG_MSG(
				"mode could not be set correctly --> mode set to error mode: " << RetinaManager::paramManager.getMode());
		return 0;
	}
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

	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_PAUSE) == GLFW_PRESS
			&& getOldKeyState(GLFW_KEY_PAUSE) == GLFW_RELEASE) {
		RetinaManager::setControl(PAUSE);
	}

	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_P) == GLFW_PRESS && getOldKeyState(GLFW_KEY_P) == GLFW_RELEASE) {
		RetinaManager::setControl(PLAY);
	}

	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_X) == GLFW_PRESS && getOldKeyState(GLFW_KEY_X) == GLFW_RELEASE) {
		RetinaManager::setControl(STOP);
	}

	if ((glfwGetKey(RetinaManager::pWindow, GLFW_KEY_A) == GLFW_PRESS)
			& (RetinaManager::paramManager.getUpdateInterval() < 20000000)) {
		RetinaManager::paramManager.incUpdateInterval();
	}

	if ((glfwGetKey(RetinaManager::pWindow, GLFW_KEY_F1) == GLFW_PRESS) && getOldKeyState(GLFW_KEY_F1) == GLFW_RELEASE) {
		RetinaManager::writeSettings();
	}

	if ((glfwGetKey(RetinaManager::pWindow, GLFW_KEY_F2) == GLFW_PRESS) && getOldKeyState(GLFW_KEY_F2) == GLFW_RELEASE) {
		RetinaManager::loadSettings();
	}

	if ((glfwGetKey(RetinaManager::pWindow, GLFW_KEY_O) == GLFW_PRESS) && getOldKeyState(GLFW_KEY_O) == GLFW_RELEASE) {
		RetinaManager::tryToUseOculus();
	}

	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_0) == GLFW_PRESS && getOldKeyState(GLFW_KEY_0) == GLFW_RELEASE) {
		RetinaManager::setMode(0);
	}
	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_1) == GLFW_PRESS && getOldKeyState(GLFW_KEY_1) == GLFW_RELEASE) {
		RetinaManager::setMode(1);
	}
	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_2) == GLFW_PRESS && getOldKeyState(GLFW_KEY_2) == GLFW_RELEASE) {
		RetinaManager::setMode(2);
	}
	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_3) == GLFW_PRESS && getOldKeyState(GLFW_KEY_3) == GLFW_RELEASE) {
		RetinaManager::setMode(3);
	}
	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_4) == GLFW_PRESS && getOldKeyState(GLFW_KEY_4) == GLFW_RELEASE) {
		RetinaManager::setMode(4);
	}
	if (glfwGetKey(RetinaManager::pWindow, GLFW_KEY_T) == GLFW_PRESS && getOldKeyState(GLFW_KEY_T) == GLFW_RELEASE) {
		RetinaManager::setFile("edvs2");
	}

	setKeyState(GLFW_KEY_0, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_0));
	setKeyState(GLFW_KEY_1, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_1));
	setKeyState(GLFW_KEY_2, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_2));
	setKeyState(GLFW_KEY_3, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_3));
	setKeyState(GLFW_KEY_4, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_4));

	setKeyState(GLFW_KEY_P, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_P));
	setKeyState(GLFW_KEY_PAUSE, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_PAUSE));
	setKeyState(GLFW_KEY_X, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_X));

	setKeyState(GLFW_KEY_F1, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_F1));
	setKeyState(GLFW_KEY_F2, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_F2));

	setKeyState(GLFW_KEY_O, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_O));

	setKeyState(GLFW_KEY_T, glfwGetKey(RetinaManager::pWindow, GLFW_KEY_T));
}

void RetinaManager::setRedGreen(char *colorVal) {
	glm::vec3 midColor;
	glm::vec3 onColor;
	glm::vec3 offColor;
	if (strcmp(colorVal, "on") == 0) {
		midColor = glm::vec3(0.5f, 0.5f, 0.5f);
		onColor = glm::vec3(1.0f, 0.0f, 0.0f);
		offColor = glm::vec3(0.0f, 1.0f, 0.0f);
	} else {
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

	strcpy(RetinaManager::edvsFileName, filename);
	// Cast from char * to string, then concatenating, then cast back to char* (yes, strcat would have also worked)
	edvsFileNameS_left = (std::string) EDVS_DATA_FOLDER_NAME + (std::string) filename
			+ (std::string) FILENAME_EXTENSION_LEFT;
	edvsFileNameS_right = (std::string) EDVS_DATA_FOLDER_NAME + (std::string) filename
			+ (std::string) FILENAME_EXTENSION_RIGHT;

	edvsFileName_left = (char *) edvsFileNameS_left.c_str();
	edvsFileName_right = (char *) edvsFileNameS_right.c_str();

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
		RetinaManager::edvsFile[1] = NULL;
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
		default: {
			DEBUG_MSG(
					"Tried to use function SetFile, when being in an invalid mode: " << RetinaManager::paramManager.getMode());
			return 0;
		}
	}

	this->CreateEDVSGL();
	if (RetinaManager::edvsFile[0] != NULL && RetinaManager::edvsFile[1] != NULL) {
		return 1;
	} else {
		return 0;
	}
}

void RetinaManager::setControl(char *control) {
	if (strcmp(control, PLAY) == 0) {
		if (this->control == Stop) {
			// Reinitialize eDVSGL objects (for next Play)
			if (this->setFile(RetinaManager::edvsFileName) == 1) {
				this->control = Play;
			}
		}
	} else if (strcmp(control, PAUSE) == 0) {
		this->control = Pause;
	} else if (strcmp(control, STOP) == 0) {
		this->control = Stop;
		this->recordCounter = 0;
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

int RetinaManager::tryToUseOculus() {
	int count;
	GLFWmonitor** monitors;
	GLFWmonitor *monitor;
	ovrSizei clientSize;

	RetinaManager::setControl(STOP);
	monitors = glfwGetMonitors(&count);
	if (count > 1) { // This means, there is a second monitor
		// The last monitor should be Oculus Rift!
		DEBUG_MSG("Second Monitor detected --> Oculus Rift");
		DEBUG_MSG(glfwGetMonitorName(monitor));
		monitor = monitors[count - 1];
		this->useOculus = true;
	} else {
		DEBUG_MSG("Only 1 monitor detected");
		monitor = NULL;
		this->useOculus = false;
	}
	clientSize.w = RetinaManager::hmdDesc.Resolution.w;
	clientSize.h = RetinaManager::hmdDesc.Resolution.h;

	//FIXME: The next 2 lines work "sometimes", e.g. in 90% of times. Why not always? Couldn't find it out yet.
	glfwDestroyWindow(RetinaManager::pWindow);
	RetinaManager::Initialize(0);

	return this->useOculus;
}

FileAndWindowStateType RetinaManager::getFileState() {
	FileAndWindowStateType tempState = Default;
	if (RetinaManager::eDVS[0]->eof() || RetinaManager::eDVS[1]->eof()) {
		tempState = tempState | EndOfFile;
	} else {
		tempState = tempState & ~EndOfFile;
	}
	if (RetinaManager::paramManager.getMode() == 4 && RetinaManager::isTimeElapsed()) {
		tempState = tempState | RecordTimeElapsed;
	} else {
		tempState = tempState & ~RecordTimeElapsed;
	}

	return tempState;
}
