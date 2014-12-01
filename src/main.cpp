#include <iostream>
#include <sstream>
#include "edvs.h"
#include "eDVSGL.h"
#include <OVR.h>
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include "shader.hpp"
#include "ParameterManager.h"
#include "DataManager.h"
#include <string.h>
#include <stdio.h>

// Defines
//*********************************************************************
//#define SHOW_ON_OCULUS false
#define OCULUS_DK_VERSION_DEBUG 2
#define DEFAULT_EDVSDATA_LEFT_FILENAME "edvsdata/edvs_left.txt"
#define DEFAULT_EDVSDATA_RIGHT_FILENAME "edvsdata/edvs_right.txt"
#define DEFAULT_MODE 2
#define MODE_INDICATOR "-mode"
#define FILENAME_INDICATOR"-filename"
#define EDVS_DATA_FOLDER_NAME "edvsdata/"
#define FILENAME_EXTENSION_RIGHT "_right.txt"
#define FILENAME_EXTENSION_LEFT "_left.txt"
//**********************************************************************

eDVSGL eDVS1;
eDVSGL eDVS2;

GLFWwindow *window;
ovrHmdDesc hmdDesc;
ovrHmd hmd;

ovrGLConfig Cfg;
ovrFovPort eyeFov[2];
ovrGLTexture eyeTexture[2];
ovrEyeRenderDesc eyeRenderDesc[2];

const size_t num_max_events = 1024;
double initTime = glfwGetTime();
int nbFrames = 0;

ParameterManager paramManager;
DataManager *dataManager;

/************************Modes supported***************************
 0: read from device, no recording
 1: read from device, recording
 2: read from file, no recording
 3: Exeriment: Read Experiment File - Not Implemented yet
 ******************************************************************/
int mode = -1;

FILE * edvsLeft;
FILE * edvsRight;
edvs_stream_handle a;
edvs_stream_handle b;
edvs_event_t* events_a;
edvs_event_t* events_b;
ssize_t eventNum1;
ssize_t eventNum2;

unsigned long getTime() {

	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	unsigned long time = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
	return time;
}

void loadSettings() {
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
			"viewport: %f \n ", &temp_updateInterval, &temp_cDecay,
			&temp_translateBack_Offset, &temp_Viewport_Offset);

	paramManager.setUpdateInterval(temp_updateInterval);
	paramManager.setTranslateBackOffset(temp_translateBack_Offset);
	paramManager.setViewportOffset(temp_Viewport_Offset);
	paramManager.setDecay(temp_cDecay);

	fclose(settings);

}

void writeSettings() {

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
			"viewport: %f \n ", temp_updateInterval, temp_cDecay,
			temp_translateBack_Offset, temp_Viewport_Offset);

	fclose(settings);

}

void writeEvents(FILE * file, edvs_event_t* event, int eventNum) {

	glm::vec4 tempEvent;
	int i;
	for (i = 0; i < eventNum; i++) {

		tempEvent.x = event[i].x;
		tempEvent.y = 127 - event[i].y;
		tempEvent.z = event[i].parity;
		tempEvent.w = event[i].t;

		if (file == NULL) {
			printf("not able to open the file !\n");

		}

		fprintf(file, "%i %i %i %lu;\n", (int) tempEvent.x, (int) tempEvent.y,
				(int) tempEvent.z, (unsigned long) tempEvent.w);
	}
}

static void WindowSizeCallback(GLFWwindow* p_Window, int p_Width,
		int p_Height) {
	if (glfwGetKey(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE){
		Cfg.OGL.Header.RTSize.w = p_Width;
		Cfg.OGL.Header.RTSize.h = p_Height;

		int distortionCaps = ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp;
		ovrHmd_ConfigureRendering(hmd, &Cfg.Config, distortionCaps, eyeFov,
				eyeRenderDesc);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}
}

void kControl() {

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		paramManager.incTranslateBackOffset();
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		paramManager.decTranslateBackOffset();
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		paramManager.decViewportOffset();
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		paramManager.incViewportOffset();
	}

	if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			& (paramManager.getDecay() < 0.5)) {
		paramManager.incDecay();
	}

	if ((glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			& (paramManager.getDecay() > 0)) {
		paramManager.decDecay();
	}

	if ((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			& (paramManager.getUpdateInterval() > 1000)) {
		paramManager.decUpdateInterval();
	}

	if ((glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			& (paramManager.getUpdateInterval() < 20000000)) {
		paramManager.incUpdateInterval();
	}

	if ((glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)) {
		writeSettings();
	}

	if ((glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)) {
		loadSettings();
	}

	if ((glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)) {
		if (mode == 0 && mode != 1) {
			mode = 1;
			edvsLeft = fopen(DEFAULT_EDVSDATA_LEFT_FILENAME, "w");
			edvsRight = fopen(DEFAULT_EDVSDATA_RIGHT_FILENAME, "w");
		}
	}

	if ((glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS)) {
		if (mode == 1) {
			mode = 0;
			fclose(edvsLeft);
			fclose(edvsRight);
		}
	}
}

int Initialize(ovrHmd *hmd,int *mode, GLFWwindow **window, ovrHmdDesc *hmdDesc, eDVSGL *eDVS1, eDVSGL *eDVS2,
		GLuint *FBOId,ovrGLTexture *eyeTexture,GLuint *programID,GLuint *MatrixID,GLuint *vertexbuffer,
		GLuint *VertexArrayID, ParameterManager *paramManager, int initModeViaKeyboard){
	if (initModeViaKeyboard){
		int nTries = 0;
		printf("Please choose the mode: \n"
				"0: Read from devices, no recording \n"
				"1: Read from devices, recording \n"
				"2: Read from files \n"
				"3: Experiment - Not implemented yet \n"
				"Mode:");
		do{
			scanf("%i", mode);
			nTries += 1;
		}while(!(*mode>=0 && *mode<=3 && nTries < 5));

		if(nTries >= 5){
			*mode = DEFAULT_MODE;
		}
	} //ELSE - mode must be initialized ELSEWHERE!

	// Initializes LibOVR.
	ovr_Initialize();
	*hmd = ovrHmd_Create(0);

	if (!*hmd) {
		// If we didn't detect an Hmd, create a simulated one for debugging.
		if (OCULUS_DK_VERSION_DEBUG == 2){
			*hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
		} else{
			*hmd = ovrHmd_CreateDebug(ovrHmd_DK1);
		}
		if (!*hmd) {   // Failed Hmd creation.
			return 1;
		}
	}

	// Get more details about the HMD
	ovrHmd_GetDesc(*hmd, hmdDesc);


	// Start the sensor which provides the Rift’s pose and motion.
	ovrHmd_StartSensor(*hmd,ovrSensorCap_Orientation |
			ovrSensorCap_YawCorrection | ovrSensorCap_Position |
					ovrHmdCap_LowPersistence,ovrSensorCap_Orientation);

	// Initialize GLFW
	if (!glfwInit()) {
		fprintf( stderr, "Failed to initialize GLFW\n");
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	ovrSizei clientSize;
	clientSize.w = hmdDesc->Resolution.w;
	clientSize.h = hmdDesc->Resolution.h;

	// Open a window and create its OpenGL context
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	GLFWmonitor *monitor;
	if (count > 1){ // This means, there is a second monitor
		monitor = monitors[count-1]; // this will make Fullscreen mode on last monitor. This should be the Occulus!
		// printf(glfwGetMonitorName(monitor));
	} else {
		monitor = NULL;
	}
	// Note: The second last param specifies wether to open a new window (NULL) or create the window in full screen mode
	// at the specified window.
#ifdef SHOW_ON_OCULUS
	*window = glfwCreateWindow(clientSize.w, clientSize.h, "eDVS: FPS=0", monitor, NULL);
#else
	*window = glfwCreateWindow(clientSize.w, clientSize.h, "eDVS: FPS=0", NULL, NULL);
#endif
	if (window == NULL) {
		fprintf( stderr,
				"Failed to open GLFW window. Make sure your graphic card supports OpenGL 3.3+. \n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(*window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(*window, GLFW_STICKY_KEYS, GL_TRUE);


	// Print OpenGL version information
	int Major = glfwGetWindowAttrib(*window, GLFW_CONTEXT_VERSION_MAJOR);
	int Minor = glfwGetWindowAttrib(*window, GLFW_CONTEXT_VERSION_MINOR);
	int Profile = glfwGetWindowAttrib(*window, GLFW_OPENGL_PROFILE);
	printf("OpenGL: %d.%d ", Major, Minor);
	if (Profile == GLFW_OPENGL_COMPAT_PROFILE)
		printf("GLFW_OPENGL_COMPAT_PROFILE\n");
	else
		printf("GLFW_OPENGL_CORE_PROFILE\n");
	printf("Vendor: %s\n", (char*) glGetString(GL_VENDOR));
	printf("Renderer: %s\n", (char*) glGetString(GL_RENDERER));

	// Configure Stereo settings.
	ovrSizei texSizeLeft = ovrHmd_GetFovTextureSize(*hmd, ovrEye_Left,
			hmdDesc->DefaultEyeFov[0], 1.0f);
	ovrSizei texSizeRight = ovrHmd_GetFovTextureSize(*hmd, ovrEye_Right,
			hmdDesc->DefaultEyeFov[1], 1.0f);
	ovrSizei texSize;
	texSize.w = texSizeLeft.w + texSizeRight.w;
	texSize.h = (
			texSizeLeft.h > texSizeRight.h ? texSizeRight.h : texSizeRight.h);

	// Create FBO

	glGenFramebuffers(1, FBOId);
	glBindFramebuffer(GL_FRAMEBUFFER, *FBOId);

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

	// Create Depth Buffer...
	GLuint depthBufferId;
	glGenRenderbuffers(1, &depthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texSize.w,
			texSize.h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
	GL_RENDERBUFFER, depthBufferId);

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

	// Oculus Rift eye configurations...
	eyeFov[0] = hmdDesc->DefaultEyeFov[0];
	eyeFov[1] = hmdDesc->DefaultEyeFov[1];

	Cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	Cfg.OGL.Header.Multisample = (1);
	Cfg.OGL.Header.RTSize.w = clientSize.w;
	Cfg.OGL.Header.RTSize.h = clientSize.h;

	int distortionCaps = ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp;
	ovrHmd_ConfigureRendering(*hmd, &Cfg.Config, distortionCaps, eyeFov,
			eyeRenderDesc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);


	eyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
	eyeTexture[0].OGL.Header.TextureSize.w = texSize.w;
	eyeTexture[0].OGL.Header.TextureSize.h = texSize.h;
	eyeTexture[0].OGL.Header.RenderViewport.Pos.x = 0;
	eyeTexture[0].OGL.Header.RenderViewport.Pos.y = 0;
	eyeTexture[0].OGL.Header.RenderViewport.Size.w = texSize.w / 2;
	eyeTexture[0].OGL.Header.RenderViewport.Size.h = texSize.h;
	eyeTexture[0].OGL.TexId = texId;

	eyeTexture[1] = eyeTexture[0];
	eyeTexture[1].OGL.Header.RenderViewport.Pos.x = (texSize.w + 1) / 2;

	glfwSetWindowSizeCallback(*window, WindowSizeCallback);

	// Create vertexshader buffer
	glGenBuffers(1, vertexbuffer);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);

	// Create VertexArray
	glGenVertexArrays(1, VertexArrayID);
	glBindVertexArray(*VertexArrayID);

	// Create and compile GLSL program from the shaders
	*programID = LoadShaders("src/shader/vertexshader.txt",
			"src/shader/fragmentshader.txt");
	*MatrixID = glGetUniformLocation(*programID, "transMatrix");

	// Initialize Visualization object

	eDVS1->initialize(paramManager->getMidColor(), paramManager->getOnColor(), paramManager->getOffColor());
	eDVS1->setGL(*window, *vertexbuffer, colorbuffer, *programID);

	eDVS2->initialize(paramManager->getMidColor(), paramManager->getOnColor(), paramManager->getOffColor());
	eDVS2->setGL(*window, *vertexbuffer, colorbuffer, *programID);

	//  Mode Initialization
	switch (*mode) {
		case 0: {

			//Initialize eDVS

			a = edvs_open("/dev/ttyUSB0?baudrate=4000000");
			b = edvs_open("/dev/ttyUSB1?baudrate=4000000");

			events_a = (edvs_event_t*) malloc(
					num_max_events * sizeof(edvs_event_t));
			events_b = (edvs_event_t*) malloc(
					num_max_events * sizeof(edvs_event_t));
			break;
		}

		case 1: {

			//Initialize eDVS

			a = edvs_open("/dev/ttyUSB0?baudrate=4000000");
			b = edvs_open("/dev/ttyUSB1?baudrate=4000000");

			events_a = (edvs_event_t*) malloc(
					num_max_events * sizeof(edvs_event_t));
			events_b = (edvs_event_t*) malloc(
					num_max_events * sizeof(edvs_event_t));
			edvsLeft = fopen(DEFAULT_EDVSDATA_LEFT_FILENAME, "w");
			edvsRight = fopen(DEFAULT_EDVSDATA_RIGHT_FILENAME, "w");
			break;
		}
		case 2: {
			edvsLeft = fopen(DEFAULT_EDVSDATA_LEFT_FILENAME, "r");
			edvsRight = fopen(DEFAULT_EDVSDATA_RIGHT_FILENAME, "r");
			break;
		}
		case 3: {
			printf("Not implemented yet");
		}
	}
	return 1;
}

	/* Measure FPS
	 * FPS target in ms: 1000ms/FPS
	 * 60FPS = 16.666667ms ## 75 FPS = 13.333333ms, etc.
	 */
	void measureFPS() {
		double currentTime = glfwGetTime();
		nbFrames++;

		if ( currentTime - initTime >= 1.0 ) {
			// must convert to const char* for glfwSetWindowTitle
			// If you want to show ms instead of fps change to "(double)1000/(double)(nbFrames);"
			double d = (double)(nbFrames);
			std::stringstream ss;
			ss << d;
			const char* str = ss.str().c_str();

			glfwSetWindowTitle(window, str);

			// counter zero, count until next second is finished
			nbFrames = 0;
			initTime += 1.0;
		}
	}
/* *****************************************************************
// This function reads in all the input params of the main function,
 * which is all input given in argv. The input should be given by
 * a name specifying the name of the variable, followed by the true
 * variable.
 * This function only checks for inputs mode and filename. The
 * correct spelling is defined in the corresponding #defines
********************************************************************/
int initMainArguments(int argc, char *argv[], int *mode, char *edvsDataFileNameLeft, char *edvsDataFileNameRight){
	if (argc < 2){
		return true;
	}
	char *cMode = NULL;
	char *edvsDataFileName = NULL;
	int initModeViaKeyboard;
	// 1. parse mode and filename if it is given
	for(int i = 0; i<argc;i++){
		if (strcmp(argv[i],MODE_INDICATOR)){
			edvsDataFileName = argv[i+1];
		} else if (strcmp(argv[i],FILENAME_INDICATOR)){
			cMode = argv[i+1];
		}
	}
	if (cMode != NULL){
		// 2. check wether mode is in the correct format, must be string of ONE digit.
		if(sizeof(cMode) != sizeof("0")){ //checks for != 0 indirectly aswell..
			printf("Wrong mode given: %s",cMode);
			initModeViaKeyboard = true;
		} else{ //mode is 1 digit (entered correctly)
			*mode = atoi(cMode);
			initModeViaKeyboard = false;
		}
	}
	if(edvsDataFileName!=NULL){
		edvsDataFileNameLeft = strcat(EDVS_DATA_FOLDER_NAME,edvsDataFileName);
		edvsDataFileNameLeft = strcat(edvsDataFileName,FILENAME_EXTENSION_LEFT);
		edvsDataFileNameRight = strcat(EDVS_DATA_FOLDER_NAME,edvsDataFileName);
		edvsDataFileNameRight = strcat(edvsDataFileName,FILENAME_EXTENSION_RIGHT);
	} else{
		printf("No Filename given!");
		edvsDataFileNameLeft = DEFAULT_EDVSDATA_LEFT_FILENAME;
		edvsDataFileNameRight = DEFAULT_EDVSDATA_RIGHT_FILENAME;
	}

	return initModeViaKeyboard;
}


int main(int argc, char *argv[]) {
	printf("narg = %i\n",argc);
	for(int i = 0; i<argc;i++){
		printf("%s\n",argv[i]);
	};
	printf("\n\n");
	GLuint VertexArrayID;
	GLuint vertexbuffer;
	GLuint programID;
	GLuint MatrixID;
	GLuint FBOId;
	glm::mat4 transMatrix;
	int initModeViaKeyboard;
	char *edvsDataFileNameLeft;
	char *edvsDataFileNameRight;

	// *** NOTE: The following function and reading params from main has not been tested yet.***
	initModeViaKeyboard = initMainArguments(argc, argv,&mode,edvsDataFileNameLeft,edvsDataFileNameRight);


	Initialize(&hmd, &mode, &window,&hmdDesc,&eDVS1,&eDVS2,&FBOId, eyeTexture,
			&programID, &MatrixID,&vertexbuffer,&VertexArrayID,&paramManager,initModeViaKeyboard);

	//TEST
	//dataManager = new DataManager;
	//dataManager->ReduceDataFromFile("testDrop.txt",DEFAULT_EDVSDATA_LEFT_FILENAME,0.8);
	//dataManager->ReduceDataFromFile("testDrop2.txt",DEFAULT_EDVSDATA_RIGHT_FILENAME,0.4);
	//TEST

	// Recording mode: type any key into the console to start the recording --> Control when to start recording
	if(mode == 1){
		int dummy;
		printf("Type any key to start recording: \n");
		scanf("%i", &dummy);
	}
	//Rendering Loop
	do {
		measureFPS();

		// Activate keyboard controlling
		kControl();

		// Mode Initialization
		switch (mode) {
			case 0: {
				// Obtain event stream from eDVS
				eventNum1 = edvs_read(a, events_a, num_max_events);
				eventNum2 = edvs_read(b, events_b, num_max_events);
				break;
			}
			case 1: {
				// Obtain event stream from eDVS
				eventNum1 = edvs_read(a, events_a, num_max_events);
				eventNum2 = edvs_read(b, events_b, num_max_events);
				break;
			}
		}

		// Begin OVR rendering
		ovrHmd_BeginFrame(hmd, 0);

		// Bind the FBO...
		glBindFramebuffer(GL_FRAMEBUFFER, FBOId);

		// Clear the background
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
			ovrEyeType Eye = hmdDesc.EyeRenderOrder[eyeIndex];
			ovrPosef eyePose = ovrHmd_BeginEyeRender(hmd, Eye);

			if (eyeIndex == 0) {

				glViewport(
						eyeTexture[Eye].OGL.Header.RenderViewport.Pos.x
								+ paramManager.getViewportOffset(),
						eyeTexture[Eye].OGL.Header.RenderViewport.Pos.y,
						eyeTexture[Eye].OGL.Header.RenderViewport.Size.w,
						eyeTexture[Eye].OGL.Header.RenderViewport.Size.h);
			}

			else {
				glViewport(
						eyeTexture[Eye].OGL.Header.RenderViewport.Pos.x
								- paramManager.getViewportOffset(),
						eyeTexture[Eye].OGL.Header.RenderViewport.Pos.y,
						eyeTexture[Eye].OGL.Header.RenderViewport.Size.w,
						eyeTexture[Eye].OGL.Header.RenderViewport.Size.h);
			}

			// Get Projection and ModelView matrices from the device
			OVR::Matrix4f projectionMatrix = ovrMatrix4f_Projection(
					eyeRenderDesc[Eye].Fov, 0.3f, 1000.0f, true);

			/****** Model-View Matrix doesn't have to be changed for Oculus Retina Display *******
			 OVR::Quatf Orientation = OVR::Quatf(eyePose.Orientation);
			 OVR::Matrix4f modelViewMatrix = OVR::Matrix4f(Orientation.Inverted());
			 *************************************************************************************/

			// Convert the matrices into OpenGl form
			glm::mat4 projMat;
			glm::mat4 modelViewMat;
			memcpy(glm::value_ptr(projMat),
					&(projectionMatrix.Transposed().M[0][0]),
					sizeof(projectionMatrix));
			//memcpy(glm::value_ptr(modelViewMat), &(modelViewMatrix.Transposed().M[0][0]), sizeof(modelViewMatrix));
			modelViewMat = glm::mat4(1.0); //Identity matrix for model-view

			// Adjust IPD and the distance from FOV
			glm::mat4 translateIPD = glm::translate(glm::mat4(1.0),
					glm::vec3(eyeRenderDesc[Eye].ViewAdjust.x,
							eyeRenderDesc[Eye].ViewAdjust.y,
							eyeRenderDesc[Eye].ViewAdjust.z));

			glm::mat4 translateBack = glm::translate(glm::mat4(1.0),
					glm::vec3(0, 0, paramManager.getTranslateBackOffset()));

			// Calculate the transformation matrix for the shader
			modelViewMat = modelViewMat * translateBack * translateIPD;
			transMatrix = projMat * modelViewMat;

			// Use the shader
			glUseProgram(programID);
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &transMatrix[0][0]);

			if (eyeIndex == 0) {
				// Read events in different modes
				switch (mode) {
				case 0: {
					eDVS1.updateEvent(events_a, eventNum1, paramManager.getUpdateInterval());
					break;
				}
				case 1: {
					writeEvents(edvsLeft, events_a, eventNum1);
					eDVS1.updateEvent(events_a, eventNum1, paramManager.getUpdateInterval());
					break;
				}
				case 2: {
					if (!eDVS1.eof()) {
						eDVS1.updateEvent(edvsLeft, paramManager.getUpdateInterval(),
								paramManager.getDisplayInterval());
					}
					break;

				}
				}
				// Draw the events
				eDVS1.draw(paramManager.getDecay());
			}

			else {

				// Read events in different modes
				switch (mode) {
				case 0: {
					eDVS2.updateEvent(events_b, eventNum2, paramManager.getUpdateInterval());
					break;
				}
				case 1: {
					writeEvents(edvsRight, events_b, eventNum2);
					eDVS2.updateEvent(events_b, eventNum2, paramManager.getUpdateInterval());
					break;
				}
				case 2: {
					if (!eDVS2.eof()) {
						eDVS2.updateEvent(edvsRight, paramManager.getDisplayInterval(), paramManager.getDisplayInterval());
					}
					break;
				}
				case 3:{
					printf("Not Implemented yet");
					break;
				}
				}
				// Draw the events
				eDVS2.draw(paramManager.getDecay());

			}

			ovrHmd_EndEyeRender(hmd, Eye, eyePose, &eyeTexture[Eye].Texture);

		}

		// Swap buffers
		glfwSwapBuffers(window);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// End OVR rendering
		ovrHmd_EndFrame(hmd);

		// Clean up
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
			&& glfwWindowShouldClose(window) == 0 && !eDVS1.eof() && !eDVS2.eof());

	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);

	// Shutdown Oculusrift
	ovrHmd_Destroy(hmd);
	ovr_Shutdown();

	//Cleanup eDVS
	edvs_close(a);
	edvs_close(b);
	free(events_a);
	free(events_b);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

