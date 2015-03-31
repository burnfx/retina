/*
 * RetinaManager.h
 *
 *  Created on: 10.12.2014
 *      Author: richi-ubuntu
 */
#ifndef RETINAMANAGER_H_
#define RETINAMANAGER_H_

#include <OVR.h>
#include "eDVSGL.h"
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include "stdio.h"
#include "ParameterManager.h"
#include "shader.hpp"
#include "main.h"



/************************Modes supported***************************
 0: read from device, no recording
 1: read from device, recording
 2: read from file, no recording
 3: Exeriment: Read Experiment File - Not Implemented yet
 ******************************************************************/

class RetinaManager {
private:
	int recordCounter = 0;
	int recordPlayTime = 10;
	int firstFrame = 0;

	bool useOculus;
	bool isInitialized = false;

	char *edvsFileName_left;
	char *edvsFileName_right;
	char edvsFileName[100];

	const size_t num_max_events = 1024;

	double initTime = glfwGetTime();
	int nbFrames;
	GLuint FBOId;
	ovrHmd hmd;

	edvs_stream_handle streamHandle[2];
	edvs_event_t* events[2];


	ovrHmdDesc hmdDesc;
	ovrGLConfig Cfg;
	ovrFovPort eyeFov[2];
	ovrGLTexture eyeTexture[2];
	ovrEyeRenderDesc eyeRenderDesc[2];
	ssize_t eventNum[2];

	GLuint texId;
	GLuint depthBufferId;

	GLuint VertexArrayID;
	GLuint vertexbuffer;
	GLuint colorbuffer;
	GLuint programID;
	GLuint MatrixID;

	eDVSGL *eDVS[2];
	FILE *edvsFile[2];


	ParameterManager paramManager;
	GLFWwindow *pWindow;
	RetinaControlType control;

	RetinaServerInterface *pRetInterface;


	void WriteEventsToFile(int eyeIndex);

	void ReadEventsFromSensor();
	int isTimeElapsed();
	glm::mat4 CalcTransMatrix(ovrEyeType Eye);

	void loadSettings();
	void writeSettings();
	void writeEventsToFile(FILE * file, edvs_event_t* event, int eventNum);
	double measureFPS();
	void CreateEDVSGL();
	void displayBlackBackground();
public:
	void renderOvrEyes();
	void UpdateEvents();
	FileAndWindowStateType getFileAndWindowState();
	RetinaManager();
	virtual ~RetinaManager();
	int setMode(int mode);
	void TerminateWindow();
	void render();
	int Initialize(int initModeViaKeyboard, RetinaServerInterface *retInterface);
	int initGL(ovrSizei clientSize,ovrSizei texSize, GLFWmonitor *monitor);

	void KeyControl();

	const ovrGLConfig& getCfg() const {
		return Cfg;
	}

	void setCfg(const ovrGLConfig& cfg) {
		Cfg = cfg;
	}

	const ovrFovPort *getEyeFov() const{
		return eyeFov;
	}

	const ovrEyeRenderDesc *getEyeRenderDesc() const{
		return eyeRenderDesc;
	}

	ovrHmd getHmd() const {
		return hmd;
	}

	void setHmd(ovrHmd hmd) {
		this->hmd = hmd;
	}

	ParameterManager& getParamManager(){
		return paramManager;
	}

	void setParamManager(ParameterManager& paramManager) {
		this->paramManager = paramManager;
	}

	GLFWwindow*& getWindow() {
		return pWindow;
	}

	void setWindow(GLFWwindow*& window) {
		pWindow = window;
	}

	char* getEdvsFileNameLeft() const {
		return edvsFileName_left;
	}

	void setEdvsFileNameLeft(char* edvsFileNameLeft) {
		edvsFileName_left = edvsFileNameLeft;
	}

	char* getEdvsFileNameRight() const {
		return edvsFileName_right;
	}

	void setEdvsFileNameRight(char* edvsFileNameRight) {
		edvsFileName_right = edvsFileNameRight;
	}


	void setMode(char *mode){
		this->setMode(atoi(mode));
	}
	void setcDecay(char *cDecay){
		this->getParamManager().setDecay(atof(cDecay));
	}
	void setUpdateInterval(char *updateInterval){
		this->getParamManager().setUpdateInterval(atoi(updateInterval));
	}
	void setDisplayInterval(char *displayInterval){
		this->getParamManager().setDisplayInterval(atoi(displayInterval));
	}
	void setTranslateBack_Offset(char *translate_back){
		this->getParamManager().setTranslateBackOffset(atof(translate_back));
	}
	void setViewport_Offset(char *viewport){
		this->getParamManager().setViewportOffset(atof(viewport));
	}

	void setRedGreen(char *colorVal);
	int setFile(char *filename);

	void setControl(char *control);


	RetinaControlType getControl() const {
		return control;
	}


	FileAndWindowStateType getFileState();

	bool isUsingOculus() const {
		return useOculus;
	}

	int tryToUseOculus();

	void setTime(char *time){
		this->recordPlayTime = atoi(time);
	}
	float getTime(){
		return this->recordPlayTime;
	}
	void setFirstFrame(int val);
	int isFirstFrame();
	void measureStartTime();

	void destroyWindow(){glfwDestroyWindow(pWindow);}
};


#endif /* RETINAMANAGER_H_ */
