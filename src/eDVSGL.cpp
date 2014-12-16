//
//  eDVSGL.cpp
//  RetinaRift
//
//  Created by HiT on 07.08.14.
//  Copyright (c) 2014 H.Zhu. All rights reserved.
//

#include "eDVSGL.h"

bool eDVSGL::eof() {
	return m_eof;
}

unsigned long eDVSGL::getTime() {

	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);

}

void eDVSGL::setGL(GLFWwindow* window, GLuint vertexbuffer, GLuint colorbuffer,
		GLuint programID) {

	m_vertexbuffer = vertexbuffer;
	m_colorbuffer = colorbuffer;
	m_programID = programID;
	m_window = window;

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3),
			&m_vertices[0], GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(glm::vec3),
			&m_colors[0],
			GL_STREAM_DRAW);

}

void eDVSGL::initialize(glm::vec3 midColor, glm::vec3 onColor,
		glm::vec3 offColor) {

	m_eof = 0;
	m_midColor = midColor;
	m_onColor = onColor;
	m_offColor = offColor;
	m_initEvent = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 128 * 128; i++) {
		m_colors.push_back(midColor);
	}

	for (int i = 0; i < 128 * 128; i++) {
		m_events.push_back(m_initEvent);
	}

	for (int i = 0; i < 128 * 128; i++) {
		glm::vec3 vertex;

		int x = i / 128;
		int y = i % 128;

		vertex.z = 0;
		vertex.x = ((float) x / 64) - 1;
		vertex.y = ((float) y / 64) - 1;

		m_vertices.push_back(vertex);
	}

}

void eDVSGL::updateEvent(edvs_event_t * event, int eventNum,
		float updateInterval) {

	int i;
	unsigned long start = 0;
	for (i = 0; i < eventNum; i++) {

		if (i==0){ start = getTime();}
		if (i==eventNum-1){printf ("%lu \n", getTime()-start);}

		m_tempEvent.x = event[i].x;
		m_tempEvent.y = 127 - event[i].y;
		m_tempEvent.z = event[i].parity;
		m_tempEvent.w = event[i].t;

		m_updateInterval = updateInterval;
		m_vertexIndex = 128 * m_tempEvent.x + m_tempEvent.y;

		if (m_tempEvent.w - m_events[m_vertexIndex].w > updateInterval) {

			//set the color with fading
			m_colors[m_vertexIndex] = m_tempEvent.z ? m_onColor : m_offColor;

			//update the events array
			m_events[m_vertexIndex] = m_tempEvent;
		}
	}
}

void eDVSGL::updateEvent(FILE * file, int updateInterval, int displayInterval) {

	int b, c;
	unsigned long a, d;

	unsigned long accumulation_max_time = displayInterval;
	unsigned long accumulation_time = 0;

	while (1) {

		if (EOF == fscanf(file, "%lu %i %i %lu;", &a, &b, &c, &d)) {
			printf("eof! \n");
			m_eof = 1;
			break;
		} else{
			m_eof = 0;
		}

		accumulation_time = d - m_accumulateBegin;

		m_tempEvent.x = a;
		m_tempEvent.y = b;
		m_tempEvent.z = c;
		m_tempEvent.w = d;

		if (accumulation_time > accumulation_max_time) {
			m_accumulateBegin = d;
			break;
		}

		m_updateInterval = updateInterval;
		m_vertexIndex = 128 * m_tempEvent.x + m_tempEvent.y;

		if (m_tempEvent.w - m_events[m_vertexIndex].w > updateInterval) {
			//set the color with fading
			m_colors[m_vertexIndex] = m_tempEvent.z ? m_onColor : m_offColor;

			//update the events array
			m_events[m_vertexIndex] = m_tempEvent;
		}

	}

}

float eDVSGL::decayComponent(float current, float target, float decay) {

	if (current - decay >= target) {
		return current - decay;
	}
	else if (current + decay <= target) {
		return current + decay;
	}
	else {
		return target;
	}
	return target;
}

glm::vec3 eDVSGL::decayColor(glm::vec3 color, glm::vec3 target, float decay) {

	glm::vec3 outcolor;
	outcolor.x = decayComponent(color.x, target.x, decay);
	outcolor.y = decayComponent(color.y, target.y, decay);
	outcolor.z = decayComponent(color.z, target.z, decay);
	return outcolor;
}

void eDVSGL::draw(float cDecay) {
	glm::vec3 OnToMid = m_onColor - m_midColor;
	glm::vec3 OffToMid = m_offColor - m_midColor;

	for (int i = 0; i < 128 * 128; i++) {
		m_colors[i] = decayColor(m_colors[i], m_midColor, cDecay);
	}

	// Bind the color buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(glm::vec3),
			&m_colors[0],
			GL_STREAM_DRAW);

// Use the shader
	glUseProgram(m_programID);

// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
	glVertexAttribPointer(0, // attribute. No particular reason for 0, but must match the layout in the shader.
			3, // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0, // stride
			(void*) 0 // array buffer offset
			);

// 2nd attribute buffer : colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m_colorbuffer);
	glVertexAttribPointer(1, // attribute. No particular reason for 1, but must match the layout in the shader.
			3, // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0, // stride
			(void*) 0 // array buffer offset
			);

// Draw the points !
	glPointSize(5.0f);
	glDrawArrays(GL_POINTS, 0, (GLsizei) m_vertices.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

}
