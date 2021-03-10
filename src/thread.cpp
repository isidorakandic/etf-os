#include "thread.h"
#include "pcb.h"
#include "system.h"
#include <stdio.h>
#include <dos.h>
#include "kernlThr.h"

Thread::Thread(StackSize stackSize, Time timeSlice) {
	pcbID = -3; // da ne prijavljuje gresku
	if (nextPCBid == 0) { // ako je prva nit u sistemu onda je to kernel thread i ona ne moze da se kreira sistemskim pozivom
		new PCB(stackSize, timeSlice, this); // pravi i stavlja PCB u globalni niz i dodeljuje mu ID
	} else {
		// ako nije kernel thread onda se sistemskim pozivom kreira nova nit
		systemCallData newData;
		newData.name = THREADCONSTRUCTOR;
		newData.stackSize = stackSize;
		newData.timeSlice = timeSlice;
		newData.myThread = this;
		unsigned tcx;
		unsigned tdx;
#ifndef BCC_BLOCK_IGNORE
		tcx = FP_SEG(&newData);
		tdx = FP_OFF(&newData);
		asm {
			push cx
			push dx
			mov cx, tcx
			mov dx, tdx
			int 0x61
			pop dx
			pop cx
		}
#endif
	}
// new PCB(stackSize, timeSlice, this);
}

Thread::~Thread() { // sve izvedene klase kao prvu liniju destruktora imaju poziv waitToComplete()
	if (pcbID == 0) { // brisemo kernel thread i to ne mozemo preko sistemskog poziva
		delete allPCB[pcbID];
		allPCB[pcbID] = 0;
	} else {
		systemCallData newData;
		newData.objectID = pcbID;
		newData.name = THREADDESTRUCTOR;
		unsigned tcx, tdx;
#ifndef BCC_BLOCK_IGNORE
		tcx = FP_SEG(&newData);
		tdx = FP_OFF(&newData);
		asm {
			push cx
			push dx
			mov cx, tcx
			mov dx, tdx
			int 0x61
			pop dx
			pop cx
		}
#endif
	}
// delete allPCB[pcbID];
}

void Thread::start() {
	systemCallData newData;
	unsigned tcx, tdx;
	newData.name = START;
	newData.objectID = pcbID;
#ifndef BCC_BLOCK_IGNORE
	tcx = FP_SEG(&newData);
	tdx = FP_OFF(&newData);
	asm {
		push cx
		push dx
		mov cx, tcx
		mov dx, tdx
		int 0x61
		pop dx
		pop cx
	}
#endif
// allPCB[pcbID]->start();
}

void Thread::waitToComplete() {
	systemCallData newData;
	unsigned tcx, tdx;
	newData.name = WAITTOCOMPLETE;
	newData.objectID = pcbID;
#ifndef BCC_BLOCK_IGNORE
	tcx = FP_SEG(&newData);
	tdx = FP_OFF(&newData);
	asm {
		push cx
		push dx
		mov cx, tcx
		mov dx, tdx
		int 0x61
		pop dx
		pop cx
	}
#endif
// allPCB[pcbID]->waitToComplete();
}

void Thread::sleep(Time timeToSleep) {
	systemCallData newData;
	unsigned tcx, tdx;
	newData.name = SLEEP;
	newData.timeToSleep = timeToSleep;
#ifndef BCC_BLOCK_IGNORE
	tcx = FP_SEG(&newData);
	tdx = FP_OFF(&newData);
	asm {
		push cx
		push dx
		mov cx, tcx
		mov dx, tdx
		int 0x61
		pop dx
		pop cx
	}
#endif
// PCB::sleep(timeToSleep);
}

void Thread::wrapper(Thread* running) {
	running->run();
	systemCallData newData;
	unsigned tcx, tdx;
	newData.name = CLEARWAITLIST;
	newData.objectID = running->pcbID;
#ifndef BCC_BLOCK_IGNORE
	tcx = FP_SEG(&newData);
	tdx = FP_OFF(&newData);
	asm {
		mov cx, tcx
		mov dx, tdx
		int 0x61
	}
#endif
// clearWaitList();
	// ovde nemamo push i pop jer ce se u clearWaitList() state postaviti na Finished i nit je zavrsila sa svojim izvrsavanjem
	// i vise se ne vraca ovde
}

void dispatch() {
	systemCallData newData;
	newData.name = DISPATCH;
	unsigned tcx, tdx;
#ifndef BCC_BLOCK_IGNORE
	tcx = FP_SEG(&newData);
	tdx = FP_OFF(&newData);
	asm {
		push cx
		push dx
		mov cx, tcx
		mov dx, tdx
		int 61h
		pop dx
		pop cx
	}
#endif
}

