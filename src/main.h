#ifndef MAIN_H_
#define MAIN_H_


#include <thread>
#include <pthread.h>
//*************************Defines *******************************
#define MODE_INDICATOR "-mode"
#define FILENAME_INDICATOR "-filename"
#define EDVS_DATA_FOLDER_NAME "edvsdata/"
#define FILENAME_EXTENSION_RIGHT "_right.txt"
#define FILENAME_EXTENSION_LEFT "_left.txt"

#define DEFAULT_MODE 2
#define ERROR_MODE -1

//#define RECORD_DURATION 10 //sec
#define DEFAULT_MODE 2
#define OCULUS_DK_VERSION_DEBUG 2




#define PLAY "play"
#define PAUSE "pause"
#define STOP "stop"


class RetinaServerInterface;
class RetinaManager;

extern RetinaManager *retinaManager;
extern RetinaServerInterface *retInterface;


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
