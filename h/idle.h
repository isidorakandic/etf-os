#ifndef _idle_h_
#define _idle_h_

#include "thread.h"

const int idleTimeSlice = 1;

class Idle: public Thread {
public:
	Idle() :
			Thread(defaultStackSize, idleTimeSlice) {
	}
	~Idle() {
	}
protected:
	virtual void run();
};

#endif;
