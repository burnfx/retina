#ifndef MAIN_H_
#define MAIN_H_


//*************************Defines *******************************
#define MODE_INDICATOR "-mode"
#define FILENAME_INDICATOR "-filename"
#define EDVS_DATA_FOLDER_NAME "edvsdata/"
#define FILENAME_EXTENSION_RIGHT "_right.txt"
#define FILENAME_EXTENSION_LEFT "_left.txt"

#define SHOW_ON_OCULUS true
#define DEFAULT_MODE 2


#define PLAY "-play"
#define PAUSE "-pause"
#define STOP "-stop"


class RetinaManager;
extern RetinaManager *retinaManager;



enum RetinaRenderReturnType {
	Default				= 0,
	EndOfFile 			= 1 << 0,
	RecordTimeElapsed 	= 1 << 1,
	CloseWindowRequest 	= 1 << 2,
};
inline RetinaRenderReturnType operator |(RetinaRenderReturnType a, RetinaRenderReturnType b) {
	return static_cast<RetinaRenderReturnType>(static_cast<int>(a) | static_cast<int>(b));
}

inline RetinaRenderReturnType operator &(RetinaRenderReturnType a, RetinaRenderReturnType b) {
	return static_cast<RetinaRenderReturnType>(static_cast<int>(a) & static_cast<int>(b));
}
// Overloaded operator == to the same as & operator. Example:
// If e.g. Stop and EndOfFile is true for RetinaRenderReturnType example, then
// example == Stop and example == EndOfFile both will be true.
inline RetinaRenderReturnType operator ==(RetinaRenderReturnType a, RetinaRenderReturnType b) {
	return static_cast<RetinaRenderReturnType>(static_cast<int>(a) & static_cast<int>(b));
}

inline RetinaRenderReturnType operator ~(RetinaRenderReturnType a) {
	return static_cast<RetinaRenderReturnType>(~(static_cast<int>(a)));
}




enum RetinaStateType {
	Play,
	Pause,
	Stop,
};
#endif
