/*
 * DataManager.h
 *
 *  Created on: 24.11.2014
 *      Author: richi-ubuntu
 */

#ifndef DATAMANAGER_H_
#define DATAMANAGER_H_

class DataManager {
public:
	DataManager();
	virtual ~DataManager();

	void ReduceDataFromFile(char *wFileName, char *rFileName, float ratio);
};

#endif /* DATAMANAGER_H_ */
