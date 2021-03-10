#ifndef _kernelEv_h_
#define _kernelEv_h_

#include "event.h"
#include "pcb.h"

class KernelEv {
public:
	KernelEv(IVTNo ivtNo, Event* myEv);
	~KernelEv();
	void wait();
	void signal();
	Event* myEvent;
private:
	int value;
	PCB* myThread;
	int blocked; // govori nam da li je nasa nit trenutno blokirana ili ne
	IVTNo ivtNum;
};

#endif
