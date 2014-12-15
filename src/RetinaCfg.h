#ifndef RETINACFG_H_
#define RETINACFG_H_

enum RetinaReturnType {
	ContinueRunning 	= 0,
	EndOfFile 			= 1 << 0,
	RecordTimeElapsed 	= 1 << 1,
	CloseWindowRequest 	= 1 << 2,
};
inline RetinaReturnType operator |(RetinaReturnType a, RetinaReturnType b) {
	return static_cast<RetinaReturnType>(static_cast<int>(a) | static_cast<int>(b));
}

inline RetinaReturnType operator &(RetinaReturnType a, RetinaReturnType b) {
	return static_cast<RetinaReturnType>(static_cast<int>(a) & static_cast<int>(b));
}

#endif
