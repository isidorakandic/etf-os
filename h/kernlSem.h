#ifndef _kernelSem_h_
#define _kernelSem_h_

#include "semaphor.h"
#include "pcb.h"

class KernelSem {
public:
	KernelSem(int init = 1, Semaphore* sem);
	virtual ~KernelSem();
	virtual int wait(int toBlock);
	virtual void signal();
	int val() const; // Returns the current value of the semaphore
	Semaphore* mySem; // sta ce mi mySem?
private:
	int value;
	waitElem* blockedList; // lista niti koje su blokirane na semaforu
};

#endif
