#ifndef _pcb_h_
#define _pcb_h_

#include "thread.h"

typedef enum ThreadState {
	New, Ready, Blocked, Finished
};

typedef struct waitElem {
	PCB* threadPCB;
	waitElem* next;
};

typedef struct sleepElem {
	PCB* threadPCB;
	Time t;
	sleepElem* next;
};

const int initPSW = 0x200;
extern volatile sleepElem* sleepList;

class PCB {
public:
	void start();
	void waitToComplete();
	virtual ~PCB();
	static void sleep(Time timeToSleep);
	PCB(StackSize stackSize, Time timeSlice, Thread* myThread1);
	PCB(); // kontruktor za PCB main niti sistema
	// public jer im pristupa interrupt ili system restore
	ThreadState state;
	unsigned ss;
	unsigned sp;
	unsigned bp;
	Time timeSlice;
	Thread* myThread;
	void clearWaitList(); // pomocna metoda za deblokiranje niti koje su cekale da data nit zavrsi izvrsavanje
private:
	waitElem* waitList;
	unsigned* stack; // pamtimo zbog dealokacije
};

#endif
