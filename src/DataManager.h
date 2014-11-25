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


// Auskommentiert, da das so wohl scheiße ist, da die input der main char * sind... das dann hin und herzu casten
// ist nicht ganz easy und wohl fehleranfällig und unnötig und bla...
/*
	// Make enumeration for all Experiments
	enum experiment {
	    exp1,
	    exp2,
	    exp3,
	    exp4,
	};
	// Map those enums to the string of the Filename, EXcluding "_left.txt" / "_right.txt"
	static inline const char *stringFromExperiment(enum experiment e)
	{
	    static const char *strings[] = {
	    		"exp1",
				"exp2",
				"exp3",
				"exp4",
	    };
	    return strings[e];
	}
*/
};

#endif /* DATAMANAGER_H_ */
