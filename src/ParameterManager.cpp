/*
 * ParameterManager.cpp
 *
 *  Created on: 10.11.2014
 *      Author: richi-ubuntu
 */

#include "ParameterManager.h"
#include <glm/glm.hpp>






ParameterManager::ParameterManager() {
	updateInterval = 1000;
	displayInterval = 10000;
	translateBack_Offset = -1.5;
	viewport_Offset = 20.0;
	cDecay = 0.1;

	delta_translateBack_Offset = 0.01;
	delta_viewport_Offset = 0.5;
	delta_updateInterval = 100;
	delta_cDecay = 0.0002;
	delta_displayInterval = 1000;

	midColor = glm::vec3(0.5f, 0.5f, 0.5f);
	onColor = glm::vec3(1.0f, 1.0f, 1.0f);
	offColor = glm::vec3(0.0f, 0.0f, 0.0f);

	mode = -1;
}

ParameterManager::~ParameterManager() {
	// TODO Auto-generated destructor stub
}


void ParameterManager::setColors(glm::vec3 midColor, glm::vec3 onColor, glm::vec3 offColor){
	this->midColor = midColor;
	this->onColor = onColor;
	this->offColor = offColor;
}



void ParameterManager::incTranslateBackOffset(){
	translateBack_Offset += delta_translateBack_Offset;
}

void ParameterManager::incViewportOffset(){
	viewport_Offset += delta_viewport_Offset;
}

void ParameterManager::incUpdateInterval(){
	updateInterval += delta_updateInterval;
}

void ParameterManager::incDecay(){
	cDecay += delta_cDecay;
}





void ParameterManager::decTranslateBackOffset(){
	translateBack_Offset -= delta_translateBack_Offset;
}

void ParameterManager::decViewportOffset(){
	viewport_Offset -= delta_viewport_Offset;
}

void ParameterManager::decUpdateInterval(){
	updateInterval -= delta_updateInterval;
}

void ParameterManager::decDecay(){
	cDecay -= delta_cDecay;
}


