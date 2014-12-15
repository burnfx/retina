//
//  eDVSGL.h
//  RetinaRift
//
//  Created by HiT on 07.08.14.
//  Copyright (c) 2014 H.Zhu. All rights reserved.
//

#ifndef __RetinaRift__eDVSGL__
#define __RetinaRift__eDVSGL__

#include <iostream>
#include <vector>
#include <ctime>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//Include GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "edvs.h"

class eDVSGL {

private:

	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec4> m_events;
	std::vector<glm::vec3> m_colors;

	glm::vec3 m_midColor;
	glm::vec3 m_onColor;
	glm::vec3 m_offColor;
	glm::vec4 m_initEvent;
	glm::vec4 m_tempEvent;
	int m_vertexIndex;

	GLuint m_vertexbuffer;
	GLuint m_colorbuffer;
	GLuint m_programID;
	GLFWwindow* m_window;

	int m_updateInterval;
	bool m_eof;
	unsigned long m_accumulateBegin;

	glm::vec3 decayColor(glm::vec3 color, glm::vec3 target, float decay);

	float decayComponent(float current, float target, float decay);

	unsigned long getTime();

public:

	bool eof();

	void setGL(GLFWwindow* window, GLuint vertexBuffer, GLuint colorBuffer,
			GLuint programID);

	void initialize(glm::vec3 midColor, glm::vec3 onColor, glm::vec3 offColor);

	void updateEvent(edvs_event_t* event, int eventNum, float updateInterval);

	void updateEvent(FILE * file, int updateInterval, int displayInterval);

	void draw(float tDecay);
};

#endif /* defined(__RetinaRift__eDVSGL__) */
