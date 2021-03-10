#ifndef _semaphor_h_
#define _semaphor_h_

#include "thread.h"

class KernelSem;

class Semaphore {
public:
	Semaphore(int init = 1);
	virtual ~Semaphore();
	virtual int wait(int toBlock);
	virtual void signal();
	int val() const; // Returns the current value of the semaphore
protected:
	friend class KernelSem; // ovo sam dodala da bi u konstruktoru kernelSem mogla da se postavi vrednost za ID nakon sto se ubaci u niz
private:
	ID kernelSemID;
};

#endif
