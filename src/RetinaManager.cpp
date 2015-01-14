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
#define FPS 60

// For printing messages during debugging
#define DEBUG //Uncomment this in order to stop debug messages
#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

// ********************************************************************************************************
// AFAIK GLFW_BUTTON_UP or anything similar for detecting the edge does not exist... Only GLFW_PRESS exists,
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

// Constructor: Only the isInitialized variable is set, the rest is done in the init function, because the object
// may be re-initialized
RetinaManager::RetinaManager() {
	this->useOculus = false;
	isInitialized = false;
}

RetinaManager::~RetinaManager() {

}

// This function returns true when the time specified in recordPlayTime has elapsed.
// Note the the variable recordCounter is incremented during each frame in the function measureFPS, which is called from render()
int RetinaManager::isTimeElapsed() {
	if (this->recordCounter > this->recordPlayTime * FPS) {
		return true;
	}
	return false;
}

/* ******************** Measure FPS ********************************************
This function counts the number of frames during each second and displays it in the window
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

		if(pWindow != NULL){
			glfwSetWindowTitle(pWindow, str);
		}
		// counter zero, count until next second is finished
		nbFrames = 0;
		initTime += 1.0;
	}
	return d;
}

// This function loads settings from a textfile like factors for the zoom (translateback offset), eye distance(viewport offset)
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

// This function write settings to a textfile like factors for the zoom (translateback offset), eye distance(viewport offset)
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

// This function writes the recorded events to a file. INPUTS:
// events: pointer to a structure which stores the events
// file: pointer to the file
// eventNum: amount of events
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


// This function updates the events to be displayed. In each frame this function should be called to retrieve new event data.
// depending on the mode the source for new data is either the eDVS sensor or a file
void RetinaManager::UpdateEvents() {
	if (this->paramManager.getMode() == 0 || this->paramManager.getMode() == 1
			|| this->paramManager.getMode() == 3) {
		this->ReadEventsFromSensor();
	}

	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
		if (this->paramManager.getMode() == 1 || this->paramManager.getMode() == 3) {
			this->writeEventsToFile(this->edvsFile[eyeIndex], this->events[eyeIndex],
					this->eventNum[eyeIndex]);
		}

		switch (this->paramManager.getMode()) {
			// update from edvs sensor
			case 0: {
				this->eDVS[eyeIndex]->updateEvent(this->events[eyeIndex],
						this->eventNum[eyeIndex], this->paramManager.getUpdateInterval());
				break;
			}

			case 1: {
			// update from edvs sensor
				this->eDVS[eyeIndex]->updateEvent(this->events[eyeIndex],
						this->eventNum[eyeIndex], this->paramManager.getUpdateInterval());
				break;
			}
			case 2: {
				// update from file
				if (!this->eDVS[eyeIndex]->eof()) {
					this->eDVS[eyeIndex]->updateEvent(this->edvsFile[eyeIndex],
							this->paramManager.getUpdateInterval(),
							this->paramManager.getDisplayInterval());
				} else {
					fclose(this->edvsFile[eyeIndex]);
					this->edvsFile[eyeIndex] = NULL;
				}
				break;

			}
			// update from edvs sensor
			case 3: {
				this->eDVS[eyeIndex]->updateEvent(this->events[eyeIndex],
						this->eventNum[eyeIndex], this->paramManager.getUpdateInterval());
				break;
			}
		}
	}
}

/* *************************************************************************
 * This function reads Events from edvs-sensors and stores them into the structure "events"
 ***************************************************************************/
void RetinaManager::ReadEventsFromSensor() {
	// Obtain event stream from eDVS
	this->eventNum[0] = edvs_read(this->streamHandle[0], this->events[0],
			this->num_max_events);
	this->eventNum[1] = edvs_read(this->streamHandle[1], this->events[1],
			this->num_max_events);
}

// This function calculates the transformation Matrix, needed for the Oculus Rift display
glm::mat4 RetinaManager::CalcTransMatrix(ovrEyeType Eye) {
	glm::mat4 projMat;
	glm::mat4 modelViewMat;
	// Get Projection and ModelView matrices from the device
	OVR::Matrix4f projectionMatrix = ovrMatrix4f_Projection(this->eyeRenderDesc[Eye].Fov, 0.3f, 1000.0f, true);

	// Convert the matrices into OpenGl form
	memcpy(glm::value_ptr(projMat), &(projectionMatrix.Transposed().M[0][0]), sizeof(projectionMatrix));
	modelViewMat = glm::mat4(1.0); //Identity matrix for model-view
	// Adjust IPD and the distance from FOV
	glm::mat4 translateIPD = glm::translate(glm::mat4(1.0),
			glm::vec3(this->eyeRenderDesc[Eye].ViewAdjust.x, this->eyeRenderDesc[Eye].ViewAdjust.y,
					this->eyeRenderDesc[Eye].ViewAdjust.z));

	glm::mat4 translateBack = glm::translate(glm::mat4(1.0),
			glm::vec3(0, 0, this->paramManager.getTranslateBackOffset()));

	// Calc and Return the transformed Mat
	return projMat * modelViewMat * translateBack * translateIPD;;
}


void RetinaManager::renderOvrEyes() {
	ovrHmd_BeginFrame((this->hmd), 0);
	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
		ovrEyeType Eye = this->hmdDesc.EyeRenderOrder[eyeIndex];
		ovrPosef eyePose = ovrHmd_BeginEyeRender(this->hmd, Eye);

		glViewport(
				this->eyeTexture[Eye].OGL.Header.RenderViewport.Pos.x
						+ getSignForEye(eyeIndex) * this->paramManager.getViewportOffset(),
				this->eyeTexture[Eye].OGL.Header.RenderViewport.Pos.y,
				this->eyeTexture[Eye].OGL.Header.RenderViewport.Size.w,
				this->eyeTexture[Eye].OGL.Header.RenderViewport.Size.h);

		glm::mat4 transMatrix = this->CalcTransMatrix(Eye);

		// Use the shader
		glUseProgram(this->programID);
		glUniformMatrix4fv(this->MatrixID, 1, GL_FALSE, &transMatrix[0][0]);

		// Draw the events
		this->eDVS[eyeIndex]->draw(this->paramManager.getDecay());

		ovrHmd_EndEyeRender((this->hmd), Eye, eyePose, &(this->eyeTexture[Eye]).Texture);
	}
	ovrHmd_EndFrame((this->hmd));
}

// This performs one rendering step and measures the FPS.
// This function should be called in every step of the render loop
void RetinaManager::render() {

	this->measureFPS();

	// Bind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, this->FBOId);
	// Clear the background
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// OVR RENDERING
	this->renderOvrEyes();

	// Swap buffers
	glfwSwapBuffers(this->pWindow);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Clean up
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	return;
}

// TODO: change type
// This function tests and returns whether the recording time has elapsed (if being in mode 3) or
// if the display window should close because of any reason.
FileAndWindowStateType RetinaManager::getFileAndWindowState() {
	FileAndWindowStateType tempState = Default;
	glfwPollEvents();
	if (this->paramManager.getMode() == 3 && this->isTimeElapsed()) {
		tempState = tempState | RecordTimeElapsed;
	} else {
		tempState = tempState & ~RecordTimeElapsed;
	}
	if (glfwGetKey(this->pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(pWindow)) {
		tempState = tempState | CloseWindowRequest;
	} else {
		tempState = tempState & ~CloseWindowRequest;
	}

	return tempState;
}

// This function initializes the part of the code, which belongs to the GL libraries
int RetinaManager::initGL(ovrSizei clientSize, ovrSizei texSize, GLFWmonitor *monitor) {
	if (&(this->texId) != NULL) glDeleteTextures(1, &(this->texId));

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


	if (this->isUsingOculus()) {
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
	glGenTextures(1, &(this->texId));
	// "Bind" the newly created texture : all future texture functions will modify this texture...
	glBindTexture(GL_TEXTURE_2D, (this->texId));
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

	glGenFramebuffers(1, &(this->FBOId));
	glBindFramebuffer(GL_FRAMEBUFFER, this->FBOId);
	// Create Depth Buffer
	glGenRenderbuffers(1, &(this->depthBufferId));
	glBindRenderbuffer(GL_RENDERBUFFER, this->depthBufferId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texSize.w, texSize.h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->depthBufferId);
	// Create vertexshader buffer
	glGenBuffers(1, &(this->vertexbuffer));
	glGenBuffers(1, &(this->colorbuffer));

	// Create VertexArray
	glGenVertexArrays(1, &(this->VertexArrayID));
	glBindVertexArray(this->VertexArrayID);

	// Create and compile GLSL program from the shaders
	this->programID = LoadShaders("src/shader/vertexshader.txt", "src/shader/fragmentshader.txt");
	this->MatrixID = glGetUniformLocation(this->programID, "transMatrix");

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
	this->eyeFov[0] = this->hmdDesc.DefaultEyeFov[0];
	this->eyeFov[1] = this->hmdDesc.DefaultEyeFov[1];

	this->Cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	this->Cfg.OGL.Header.Multisample = (1);
	this->Cfg.OGL.Header.RTSize.w = clientSize.w;
	this->Cfg.OGL.Header.RTSize.h = clientSize.h;

	int distortionCaps2 = ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp;
	ovrHmd_ConfigureRendering(this->hmd, &(this->Cfg.Config), distortionCaps2, this->eyeFov,
			this->eyeRenderDesc);

	this->eyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
	this->eyeTexture[0].OGL.Header.TextureSize.w = texSize.w;
	this->eyeTexture[0].OGL.Header.TextureSize.h = texSize.h;
	this->eyeTexture[0].OGL.Header.RenderViewport.Pos.x = 0;
	this->eyeTexture[0].OGL.Header.RenderViewport.Pos.y = 0;
	this->eyeTexture[0].OGL.Header.RenderViewport.Size.w = texSize.w / 2;
	this->eyeTexture[0].OGL.Header.RenderViewport.Size.h = texSize.h;
	this->eyeTexture[0].OGL.TexId = this->texId;

	this->eyeTexture[1] = this->eyeTexture[0];
	this->eyeTexture[1].OGL.Header.RenderViewport.Pos.x = (texSize.w + 1) / 2;
	// ******************* /OVR eye configurations ************************
	return 0;
}

// Initialization function. Needs to be called after instantiating the object.
// The object can be re-initialized at different positions in code
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
		} while (!(tempMode >= 0 && tempMode <= 3 && nTries < 5));
		if (nTries >= 5) tempMode = DEFAULT_MODE;

		this->setMode(tempMode);
	} else {	//ELSE - paramManager.mode must be initialized ELSEWHERE!
		DEBUG_MSG(
				"Initialized with mode " << ERROR_MODE << ",because boolean initViaKeyboard was set to false in init");
		this->setMode(ERROR_MODE);
	}
	// ********************* Initialize OVR ******************************
	// Initializes LibOVR.
	ovr_Initialize();
	this->hmd = ovrHmd_Create(0);

	if (!this->hmd) {
		// If we didn't detect an Hmd, create a simulated one for debugging.
		if (OCULUS_DK_VERSION_DEBUG == 2) {
			this->hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
		} else {
			this->hmd = ovrHmd_CreateDebug(ovrHmd_DK1);
		}
		if (!this->hmd) {   // Failed Hmd creation.
			return 1;
		}
	}
	// Get more details about the HMD
	ovrHmd_GetDesc(this->hmd, &(this->hmdDesc));

	// Start the sensor which provides the Rift’s pose and motion.
	ovrHmd_StartSensor(this->hmd,
			ovrSensorCap_Orientation | ovrSensorCap_YawCorrection | ovrSensorCap_Position | ovrHmdCap_LowPersistence,
			ovrSensorCap_Orientation);

	ovrSizei clientSize;
	clientSize.w = this->hmdDesc.Resolution.w;
	clientSize.h = this->hmdDesc.Resolution.h;

	// Configure Stereo settings.
	ovrSizei texSizeLeft = ovrHmd_GetFovTextureSize(this->hmd, ovrEye_Left,
			this->hmdDesc.DefaultEyeFov[0], 1.0f);
	ovrSizei texSizeRight = ovrHmd_GetFovTextureSize(this->hmd, ovrEye_Right,
			this->hmdDesc.DefaultEyeFov[1], 1.0f);
	ovrSizei texSize;
	texSize.w = texSizeLeft.w + texSizeRight.w;
	texSize.h = (texSizeLeft.h > texSizeRight.h ? texSizeRight.h : texSizeRight.h);
	// ******************** /InitializeT OVR ******************************
	// ********************* Initialize GL *****************************************
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	GLFWmonitor *monitor;
	if (count > 1 && this->useOculus == true) { // This means, there is a second monitor
		monitor = monitors[count - 1]; // this will make Fullscreen paramManager.mode on last monitor. This should be the Occulus!
		DEBUG_MSG("'Last' monitor is " << glfwGetMonitorName(monitor) <<". This should be Oculus Rift");
	} else {
		monitor = NULL;
	}

	initGL(clientSize, texSize, monitor);
	// ********************* /Initialize GL ****************************************

	this->control = Stop;
	isInitialized = true;
	return 1;
}

// This function changes the mode to the one specified as input parameter. When given a wrong mode number or
// neccessary objects like streamHandle are invalid (set to 0), mode will be set to ERROR_MODE
int RetinaManager::setMode(int mode) {
	DEBUG_MSG("Changing mode from mode " << this->paramManager.getMode() << " to " << mode);
	this->setControl(STOP);
	switch (mode) {
		case 0: {
			//Initialize eDVS
			this->streamHandle[0] = edvs_open("/dev/ttyUSB0?baudrate=4000000");
			this->streamHandle[1] = edvs_open("/dev/ttyUSB1?baudrate=4000000");
			if (this->streamHandle[0] != 0 && this->streamHandle[1] != 0) {
				this->events[0] = (edvs_event_t*) malloc(this->num_max_events * sizeof(edvs_event_t));
				this->events[1] = (edvs_event_t*) malloc(this->num_max_events * sizeof(edvs_event_t));
			} else {
				mode = ERROR_MODE;
			}
			break;
		}

		case 1: {
			//Initialize eDVS
			this->streamHandle[0] = edvs_open("/dev/ttyUSB0?baudrate=4000000");
			this->streamHandle[1] = edvs_open("/dev/ttyUSB1?baudrate=4000000");
			if (this->streamHandle[0] != 0 && this->streamHandle[1] != 0) {
				this->events[0] = (edvs_event_t*) malloc(this->num_max_events * sizeof(edvs_event_t));
				this->events[1] = (edvs_event_t*) malloc(this->num_max_events * sizeof(edvs_event_t));
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
			this->streamHandle[0] = edvs_open("/dev/ttyUSB0?baudrate=4000000");
			this->streamHandle[1] = edvs_open("/dev/ttyUSB1?baudrate=4000000");
			if (this->streamHandle[0] != 0 && this->streamHandle[1] != 0) {
				this->events[0] = (edvs_event_t*) malloc(this->num_max_events * sizeof(edvs_event_t));
				this->events[1] = (edvs_event_t*) malloc(this->num_max_events * sizeof(edvs_event_t));
			} else {
				mode = ERROR_MODE;
			}
			break;
		}
		default: {
			mode = ERROR_MODE;
		}
	}

	this->paramManager.setMode(mode);
	if (mode != ERROR_MODE) {
		DEBUG_MSG("mode was succesfully set to " << this->paramManager.getMode());
		return 1;
	} else { // mode == ERROR_MODE
		DEBUG_MSG(
				"mode could not be set correctly --> mode set to error mode: " << this->paramManager.getMode());
		return 0;
	}
}

// This function terminated and closes the display window, after buffers have been freed.
void RetinaManager::TerminateWindow() {
	glDeleteBuffers(1, &(this->vertexbuffer));
	glDeleteVertexArrays(1, &(this->VertexArrayID));
	glDeleteProgram(this->programID);

	// Shutdown Oculusrift
	ovrHmd_Destroy(this->hmd);
	ovr_Shutdown();

	//Cleanup eDVS
	edvs_close(this->streamHandle[0]);
	edvs_close(this->streamHandle[1]);
	free(this->events[0]);
	free(this->events[1]);

	// Close OpenGL pWindow and terminate GLFW
	glfwTerminate();
}

// This function reads in specific keyboard inputs which are used frequently for testing/debugging or other reasons
// Note that if the program shall only respond to edges (press once or release once), the old keystate
// must be read and a new keyState must be set. This is done for keys like "0","1" for the modes or "T", which
// sets an arbitrary test file. Adapt the name of this file if necessary (Note that the actual filenames must
// follow with "_left.txt" and "_right.txt" in order to work properly. e.g. file "X" will must actually be "X_left.txt" and "X_right.txt"
void RetinaManager::KeyControl() {

	if (glfwGetKey(this->pWindow, GLFW_KEY_UP) == GLFW_PRESS) {
		this->paramManager.incTranslateBackOffset();
	}

	if (glfwGetKey(this->pWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
		this->paramManager.decTranslateBackOffset();
	}

	if (glfwGetKey(this->pWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		this->paramManager.decViewportOffset();
	}

	if (glfwGetKey(this->pWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
		this->paramManager.incViewportOffset();
	}

	if ((glfwGetKey(this->pWindow, GLFW_KEY_W) == GLFW_PRESS)
			& (this->paramManager.getDecay() < 0.5)) {
		this->paramManager.incDecay();
	}

	if ((glfwGetKey(this->pWindow, GLFW_KEY_S) == GLFW_PRESS) & (this->paramManager.getDecay() > 0)) {
		this->paramManager.decDecay();
	}

	if ((glfwGetKey(this->pWindow, GLFW_KEY_D) == GLFW_PRESS)
			& (this->paramManager.getUpdateInterval() > 1000)) {
		this->paramManager.decUpdateInterval();
	}

	if (glfwGetKey(this->pWindow, GLFW_KEY_PAUSE) == GLFW_PRESS
			&& getOldKeyState(GLFW_KEY_PAUSE) == GLFW_RELEASE) {
		this->setControl(PAUSE);
	}

	if (glfwGetKey(this->pWindow, GLFW_KEY_P) == GLFW_PRESS && getOldKeyState(GLFW_KEY_P) == GLFW_RELEASE) {
		this->setControl(PLAY);
	}

	if (glfwGetKey(this->pWindow, GLFW_KEY_X) == GLFW_PRESS && getOldKeyState(GLFW_KEY_X) == GLFW_RELEASE) {
		this->setControl(STOP);
	}

	if ((glfwGetKey(this->pWindow, GLFW_KEY_A) == GLFW_PRESS)
			& (this->paramManager.getUpdateInterval() < 20000000)) {
		this->paramManager.incUpdateInterval();
	}

	if ((glfwGetKey(this->pWindow, GLFW_KEY_F1) == GLFW_PRESS) && getOldKeyState(GLFW_KEY_F1) == GLFW_RELEASE) {
		this->writeSettings();
	}

	if ((glfwGetKey(this->pWindow, GLFW_KEY_F2) == GLFW_PRESS) && getOldKeyState(GLFW_KEY_F2) == GLFW_RELEASE) {
		this->loadSettings();
	}

	if ((glfwGetKey(this->pWindow, GLFW_KEY_O) == GLFW_PRESS) && getOldKeyState(GLFW_KEY_O) == GLFW_RELEASE) {
		this->tryToUseOculus();
	}

	if (glfwGetKey(this->pWindow, GLFW_KEY_0) == GLFW_PRESS && getOldKeyState(GLFW_KEY_0) == GLFW_RELEASE) {
		this->setMode(0);
	}
	if (glfwGetKey(this->pWindow, GLFW_KEY_1) == GLFW_PRESS && getOldKeyState(GLFW_KEY_1) == GLFW_RELEASE) {
		this->setMode(1);
	}
	if (glfwGetKey(this->pWindow, GLFW_KEY_2) == GLFW_PRESS && getOldKeyState(GLFW_KEY_2) == GLFW_RELEASE) {
		this->setMode(2);
	}
	if (glfwGetKey(this->pWindow, GLFW_KEY_3) == GLFW_PRESS && getOldKeyState(GLFW_KEY_3) == GLFW_RELEASE) {
		this->setMode(3);
	}
	if (glfwGetKey(this->pWindow, GLFW_KEY_T) == GLFW_PRESS && getOldKeyState(GLFW_KEY_T) == GLFW_RELEASE) {
		this->setFile("1/e0d30l1m1r1h444");
	}

	// atm, GLFW does not store input key values --> input key changes (edges) must be detect manually.
	// --> Store key state manually
	setKeyState(GLFW_KEY_0, glfwGetKey(this->pWindow, GLFW_KEY_0));
	setKeyState(GLFW_KEY_1, glfwGetKey(this->pWindow, GLFW_KEY_1));
	setKeyState(GLFW_KEY_2, glfwGetKey(this->pWindow, GLFW_KEY_2));
	setKeyState(GLFW_KEY_3, glfwGetKey(this->pWindow, GLFW_KEY_3));

	setKeyState(GLFW_KEY_P, glfwGetKey(this->pWindow, GLFW_KEY_P));
	setKeyState(GLFW_KEY_PAUSE, glfwGetKey(this->pWindow, GLFW_KEY_PAUSE));
	setKeyState(GLFW_KEY_X, glfwGetKey(this->pWindow, GLFW_KEY_X));

	setKeyState(GLFW_KEY_F1, glfwGetKey(this->pWindow, GLFW_KEY_F1));
	setKeyState(GLFW_KEY_F2, glfwGetKey(this->pWindow, GLFW_KEY_F2));

	setKeyState(GLFW_KEY_O, glfwGetKey(this->pWindow, GLFW_KEY_O));

	setKeyState(GLFW_KEY_T, glfwGetKey(this->pWindow, GLFW_KEY_T));
}

// This function changes the display colors either to black/grey/white or to red/grey/green
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

// this function changes the filename. Depending on the mode, the filename specifies the file to be displayed in the window
// or a filename for the video track to be recorded. Note: this function appends "_left.txt" and "_right.txt" to the actual filenames
int RetinaManager::setFile(char *filename) {

	std::string edvsFileNameS_left;
	std::string edvsFileNameS_right;
	char *edvsFileName_left;
	char *edvsFileName_right;

	strcpy(this->edvsFileName, filename);
	// Cast from char * to string, then concatenating, then cast back to char* (yes, strcat would have also worked)
	edvsFileNameS_left = (std::string) EDVS_DATA_FOLDER_NAME + (std::string) filename
			+ (std::string) FILENAME_EXTENSION_LEFT;
	edvsFileNameS_right = (std::string) EDVS_DATA_FOLDER_NAME + (std::string) filename
			+ (std::string) FILENAME_EXTENSION_RIGHT;

	edvsFileName_left = (char *) edvsFileNameS_left.c_str();
	edvsFileName_right = (char *) edvsFileNameS_right.c_str();

	// set corresponding left and right filename
	this->setEdvsFileNameRight(edvsFileName_right);
	this->setEdvsFileNameLeft(edvsFileName_left);

	// Close old file
	if (this->edvsFile[0] != NULL) {
		fclose(this->edvsFile[0]);
		this->edvsFile[0] = NULL;
	}
	if (this->edvsFile[1] != NULL) {
		fclose(this->edvsFile[1]);
		this->edvsFile[1] = NULL;
	}
	switch (this->paramManager.getMode()) {
		case 1: {
			this->edvsFile[0] = fopen(edvsFileName_left, "w");
			this->edvsFile[1] = fopen(edvsFileName_right, "w");
			break;
		}
		case 2: {
			this->edvsFile[0] = fopen(edvsFileName_left, "r");
			this->edvsFile[1] = fopen(edvsFileName_right, "r");
			break;
		}
		case 3: {
			this->edvsFile[0] = fopen(edvsFileName_left, "w");
			this->edvsFile[1] = fopen(edvsFileName_right, "w");
			break;
		}
		default: {
			DEBUG_MSG(
					"Tried to use function SetFile, when being in an invalid mode: " << this->paramManager.getMode());
			return 0;
		}
	}

	this->CreateEDVSGL();
	if (this->edvsFile[0] != NULL && this->edvsFile[1] != NULL) {
		return 1;
	} else {
		DEBUG_MSG("edvsFile is a nullpointer");
		return 0;
	}
}

// This function switches the display state between Play/Pause/Stop
// Note: When control play is received, while being in mode stop, the setFile function is called with the currently store filename.
// The return value of setFile is evaluated, depending on the return value setting the control to Play is successful or not (debug msg sent)
void RetinaManager::setControl(char *control) {
	if (strcmp(control, PLAY) == 0) {
		if (this->control == Stop) {
			// Reinitialize eDVSGL objects (for next Play)
			if (this->setFile(this->edvsFileName) == 1) {
				this->control = Play;
			} else{
				DEBUG_MSG("Couldn't start play, because setFile returned 0. Maybe wrong filename.");
			}
		} else{
			if (this->edvsFile[0] != NULL && this->edvsFile[1] != NULL) {
				this->control = Play;
			} else {
				DEBUG_MSG("edvsFile is a nullpointer");
				this->control = Stop;
			}
		}
	} else if (strcmp(control, PAUSE) == 0) {
		this->control = Pause;
	} else if (strcmp(control, STOP) == 0) {
		this->control = Stop;
		this->recordCounter = 0;
	}
}

//This function creates the eDVSGL objects and initializes them
void RetinaManager::CreateEDVSGL() {
	if (this->eDVS[0] != NULL) {
		delete (this->eDVS[0]);
	}
	this->eDVS[0] = new eDVSGL;
	this->eDVS[0]->initialize(this->paramManager.getMidColor(),
			this->paramManager.getOnColor(), paramManager.getOffColor());
	this->eDVS[0]->setGL(pWindow, this->vertexbuffer, this->colorbuffer,
			this->programID);

	if (this->eDVS[1] != NULL) {
		delete (this->eDVS[1]);
	}
	this->eDVS[1] = new eDVSGL;
	this->eDVS[1]->initialize(this->paramManager.getMidColor(),
			this->paramManager.getOnColor(), paramManager.getOffColor());
	this->eDVS[1]->setGL(pWindow, this->vertexbuffer, this->colorbuffer,
			this->programID);
}

// This function switches the display monitor to the oculus rift, if it is connected.
// Note: This function actually just declares the last connected monitor as the occulus rift,
// as it is not possible to identify the oculus rift afaik (not even with function glfwGetMonitorName)
int RetinaManager::tryToUseOculus() {
	int count;
	GLFWmonitor** monitors;
	GLFWmonitor *monitor;
	ovrSizei clientSize;

	this->setControl(STOP);
	if(!useOculus){
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
		clientSize.w = this->hmdDesc.Resolution.w;
		clientSize.h = this->hmdDesc.Resolution.h;

		//FIXME: The next 2 lines work "sometimes", e.g. in 90% of times. Why not always? Couldn't find it out yet.
		glfwDestroyWindow(this->pWindow);
		this->Initialize(0);
	} else{
		this->useOculus = false;
		clientSize.w = this->hmdDesc.Resolution.w;
		clientSize.h = this->hmdDesc.Resolution.h;

		//FIXME: The next 2 lines work "sometimes", e.g. in 90% of times. Why not always? Couldn't find it out yet.
		glfwDestroyWindow(this->pWindow);
		this->Initialize(0);
	}
	this->setMode(2);
	return this->useOculus;
}

// this function returns whether the files with edvs data are at end of while or whether Record time has elapsed in mode 3
FileAndWindowStateType RetinaManager::getFileState() {
	FileAndWindowStateType tempState = Default;
	if (this->eDVS[0]->eof() || this->eDVS[1]->eof()) {
		tempState = tempState | EndOfFile;
	} else {
		tempState = tempState & ~EndOfFile;
	}
	if (this->paramManager.getMode() == 3 && this->isTimeElapsed()) {
		tempState = tempState | RecordTimeElapsed;
	} else {
		tempState = tempState & ~RecordTimeElapsed;
	}

	return tempState;
}
