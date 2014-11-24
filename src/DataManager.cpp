/*
 * DataManager.cpp
 *
 *  Created on: 24.11.2014
 *      Author: richi-ubuntu
 */

#include "DataManager.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

#define JUST_FOR_DEBUG true


DataManager::DataManager() {
	// TODO Auto-generated constructor stub

}

DataManager::~DataManager() {
	// TODO Auto-generated destructor stub
}

void DataManager::ReduceDataFromFile(char *wFileName, char *rFileName, float ratio){
/*
 * FILE *fid = fopen(fileName, "r");

	if (fid == NULL) {
		printf("not able to open the file !\n");
	}
	int temp_updateInterval;
	float temp_translateBack_Offset;
	float temp_Viewport_Offset;
	float temp_cDecay;
	fscanf(fid, "updateInterval: %i \n"
			"cDecay: %f \n"
			"translateBack: %f \n"
			"viewport: %f \n ", &temp_updateInterval, &temp_cDecay,
			&temp_translateBack_Offset, &temp_Viewport_Offset);
*/

    string myString;
    ofstream writefile;
	ifstream readfile;
	readfile.open (rFileName);
	writefile.open (wFileName);
#ifdef JUST_FOR_DEBUG
	int drops[2] = {0, 0};
#endif
		while(!readfile.eof()) // To get you all the lines.
		{
			double rnum = (double)(std::rand())/RAND_MAX;
			getline(readfile,myString);
			if(rnum<= ratio){
				writefile << myString << "\n";
				#ifdef JUST_FOR_DEBUG
					drops[0] ++;
				#endif
			}
#ifdef JUST_FOR_DEBUG
			else {
				drops[1] ++;
			}
#endif
		}
		readfile.close();
		writefile.close();
#ifdef JUST_FOR_DEBUG
		printf("non drops: %i, drops: %i, ratio(soll): %f, ratio(is): %f\n",
				drops[0],drops[1], ratio, float(drops[0])/float(drops[0]+drops[1]));
#endif
}
