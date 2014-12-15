/*
 * ParameterManager.h
 *
 *  Created on: 10.11.2014
 *      Author: richi-ubuntu
 */

#ifndef PARAMETERMANAGER_H_
#define PARAMETERMANAGER_H_

#include <glm/glm.hpp>


class ParameterManager {
private:
	int updateInterval;
	int displayInterval;
	float translateBack_Offset;
	float viewport_Offset;
	float cDecay;

	int delta_updateInterval;
	int delta_displayInterval;
	float delta_translateBack_Offset;
	float delta_viewport_Offset;
	float delta_cDecay;

	glm::vec3 midColor;
	glm::vec3 onColor;
	glm::vec3 offColor;

	int mode;
public:

	ParameterManager();
	virtual ~ParameterManager();
	void incTranslateBackOffset();
	void incViewportOffset();
	void incUpdateInterval();
	void incDecay();
	void decTranslateBackOffset();
	void decViewportOffset();
	void decUpdateInterval();
	void decDecay();

	void setColors(glm::vec3 midColor, glm::vec3 onColor, glm::vec3 offColor);

	int getDeltaUpdateInterval() const {
		return delta_updateInterval;
	}

	void setDeltaUpdateInterval(int deltaUpdateInterval) {
		delta_updateInterval = deltaUpdateInterval;
	}

	float getDeltaViewportOffset() const {
		return delta_viewport_Offset;
	}

	void setDeltaViewportOffset(float deltaViewportOffset) {
		delta_viewport_Offset = deltaViewportOffset;
	}

	int getDeltaDisplayInterval() const {
		return delta_displayInterval;
	}

	void setDeltaDisplayInterval(int deltaDisplayInterval) {
		delta_displayInterval = deltaDisplayInterval;
	}

	float getDeltaTranslateBackOffset() const {
		return delta_translateBack_Offset;
	}

	void setDeltaTranslateBackOffset(float deltaTranslateBackOffset) {
		delta_translateBack_Offset = deltaTranslateBackOffset;
	}

	int getDisplayInterval() const {
		return displayInterval;
	}

	void setDisplayInterval(int displayInterval) {
		this->displayInterval = displayInterval;
	}

	float getTranslateBackOffset() const {
		return translateBack_Offset;
	}

	void setTranslateBackOffset(float translateBackOffset) {
		translateBack_Offset = translateBackOffset;
	}

	int getUpdateInterval() const {
		return updateInterval;
	}

	void setUpdateInterval(int updateInterval) {
		this->updateInterval = updateInterval;
	}

	float getViewportOffset() const {
		return viewport_Offset;
	}

	void setViewportOffset(float viewportOffset) {
		viewport_Offset = viewportOffset;
	}

	float getDecay() const {
		return cDecay;
	}

	void setDecay(float decay) {
		cDecay = decay;
	}

	float getDeltaCDecay() const {
		return delta_cDecay;
	}

	void setDeltaCDecay(float deltaCDecay) {
		delta_cDecay = deltaCDecay;
	}

	const glm::vec3& getMidColor() const {
		return midColor;
	}

	void setMidColor(const glm::vec3& midColor) {
		this->midColor = midColor;
	}

	const glm::vec3& getOffColor() const {
		return offColor;
	}

	void setOffColor(const glm::vec3& offColor) {
		this->offColor = offColor;
	}

	const glm::vec3& getOnColor() const {
		return onColor;
	}

	void setOnColor(const glm::vec3& onColor) {
		this->onColor = onColor;
	}

	int getMode() const {
		return mode;
	}

	void setMode(int mode) {
		this->mode = mode;
	}
};

#endif /* PARAMETERMANAGER_H_ */
