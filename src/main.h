#ifndef MAIN_H_
#define MAIN_H_


#include <mutex>
#include <thread>
//*************************Defines *******************************
#define MODE_INDICATOR "-mode"
#define FILENAME_INDICATOR "-filename"
#define EDVS_DATA_FOLDER_NAME "edvsdata/"
#define FILENAME_EXTENSION_RIGHT "_right.txt"
#define FILENAME_EXTENSION_LEFT "_left.txt"

#define DEFAULT_MODE 2

//#define RECORD_DURATION 10 //sec
#define DEFAULT_MODE 2
#define OCULUS_DK_VERSION_DEBUG 2

#define DEFAULT_EDVSDATA_LEFT_FILENAME "edvsdata/edvs_left.txt"
#define DEFAULT_EDVSDATA_RIGHT_FILENAME "edvsdata/edvs_right.txt"

#define PLAY "play"
#define PAUSE "pause"
#define STOP "stop"



class RetinaManager;
extern RetinaManager *retinaManager;

extern std::mutex myMutex;


enum FileAndWindowStateType {
	Default				= 0,
	EndOfFile 			= 1 << 0, // == 1 == 0001
	RecordTimeElapsed 	= 1 << 1, // == 2 == 0010
	CloseWindowRequest 	= 1 << 2, // == 4 == 0100
};
inline FileAndWindowStateType operator |(FileAndWindowStateType a, FileAndWindowStateType b) {
	return static_cast<FileAndWindowStateType>(static_cast<int>(a) | static_cast<int>(b));
}

inline FileAndWindowStateType operator &(FileAndWindowStateType a, FileAndWindowStateType b) {
	return static_cast<FileAndWindowStateType>(static_cast<int>(a) & static_cast<int>(b));
}
// Overloaded operator == to the same as & operator. Example:
// If e.g. Stop and EndOfFile is true for FileAndWindowStateType example, then
// example == Stop and example == EndOfFile both will be true.
inline FileAndWindowStateType operator ==(FileAndWindowStateType a, FileAndWindowStateType b) {
	return static_cast<FileAndWindowStateType>(static_cast<int>(a) & static_cast<int>(b));
}

inline FileAndWindowStateType operator ~(FileAndWindowStateType a) {
	return static_cast<FileAndWindowStateType>(~(static_cast<int>(a)));
}




enum RetinaControlType {
	Play,
	Pause,
	Stop,
};
#endif
