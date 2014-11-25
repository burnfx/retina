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
#include <vector>
#include <tgmath.h>
#include <assert.h>

using namespace std;

#define JUST_FOR_DEBUG true


DataManager::DataManager() {
	// TODO Auto-generated constructor stub

}

DataManager::~DataManager() {
	// TODO Auto-generated destructor stub
}

void DataManager::ReduceDataFromFile(char *wFileName, char *rFileName, float ratio){
	std::vector<string> AllLines;
    ofstream writefile;
	ifstream readfile;
	readfile.open (rFileName);
	writefile.open (wFileName);
	string myString;
#ifdef JUST_FOR_DEBUG
	int drops[2] = {0, 0};
#endif
	// Read all lines
	while(!readfile.eof())
	{
		getline(readfile,myString);
		AllLines.push_back(myString);
	}
	// calc how many random lines should be kept
	int N = AllLines.size();
	int M = round(ratio*N);
	// calc array of random numbers of size n_keep(each differing)
	// --> Knuth algorithm
	int rand_array[M];
	int in, im;
	im = 0;
	for (in = 0; in < N && im < M; ++in) {
		int rn = N - in;
		int rm = M - im;
		if (rand() % rn < rm){
			/* Take it */
			rand_array[im++] = in;
		}
	}
	// assert stops program and gives message, if m!=M
	assert(im == M);

	// Write those lines into new file
	int k = 0;
	for (int i = 0; i < N; i++){
		if (rand_array[k] == i){
			//write ...
			writefile << AllLines.at(i) << "\n";
			k++;
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
#ifdef JUST_FOR_DEBUG
		printf("non drops: %i, drops: %i, ratio(soll): %f, ratio(is): %f\n",
				drops[0],drops[1], ratio, float(drops[0])/float(drops[0]+drops[1]));
#endif
}
