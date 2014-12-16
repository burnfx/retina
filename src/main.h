#ifndef MAIN_H_
#define MAIN_H_


//*************************Defines *******************************
#define MODE_INDICATOR "-mode"
#define FILENAME_INDICATOR "-filename"
#define EDVS_DATA_FOLDER_NAME "edvsdata/"
#define FILENAME_EXTENSION_RIGHT "_right.txt"
#define FILENAME_EXTENSION_LEFT "_left.txt"

#define SHOW_ON_OCULUS
#define DEFAULT_MODE 2


#define PLAY "-play"
#define PAUSE "-pause"
#define STOP "-stop"


class RetinaManager;
extern RetinaManager *retinaManager;



enum RetinaReturnType {
	Play 				= 0,
	EndOfFile 			= 1 << 0,
	RecordTimeElapsed 	= 1 << 1,
	CloseWindowRequest 	= 1 << 2,
	Pause 				= 1 << 3,
	Stop				= 1 << 4,
};
inline RetinaReturnType operator |(RetinaReturnType a, RetinaReturnType b) {
	return static_cast<RetinaReturnType>(static_cast<int>(a) | static_cast<int>(b));
}

inline RetinaReturnType operator &(RetinaReturnType a, RetinaReturnType b) {
	return static_cast<RetinaReturnType>(static_cast<int>(a) & static_cast<int>(b));
}

inline RetinaReturnType operator ==(RetinaReturnType a, RetinaReturnType b) {
	return static_cast<RetinaReturnType>(static_cast<int>(a) & static_cast<int>(b));
}

inline RetinaReturnType operator ~(RetinaReturnType a) {
	return static_cast<RetinaReturnType>(~(static_cast<int>(a)));
}

#endif
