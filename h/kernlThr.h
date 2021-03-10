#ifndef _kernlThr_h_
#define _kernlThr_h_

#include "thread.h"
#include "event.h"
#include "semaphor.h"

typedef enum systemCallName {
	START,
	WAITTOCOMPLETE,
	THREADDESTRUCTOR,
	SLEEP,
	THREADCONSTRUCTOR,
	CLEARWAITLIST, // pomocna metoda u pcb
	DISPATCH,
	SEMAPHORECONSTRUCTOR,
	SEMAPHOREDESTRUCTOR,
	SEMWAIT,
	SEMSIGNAL,
	VAL,
	EVENTCONSTRUCTOR,
	EVENTDESTRUCTOR,
	EVENTWAIT,
	EVENTSIGNAL
};

typedef struct systemCallData {
	systemCallName name;
	ID objectID;
	Time timeToSleep;
	StackSize stackSize;
	Time timeSlice;
	Thread* myThread;
	int init;
	Semaphore* mySem;
	int toBlock;
	IVTNo ivtNo;
	Event* myEv;
	int retValue;
};

class kernlThr: public Thread {
public:
	kernlThr() :
			Thread(defaultStackSize, defaultTimeSlice) {
	}
	~kernlThr() {
	}
protected:
	virtual void run();
private:
};

#endif;
