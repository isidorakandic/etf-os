#include "semaphor.h"
#include "kernlSem.h"
#include "system.h"
#include <stdio.h>
#include "kernlThr.h"

Semaphore::Semaphore(int init) {
	kernelSemID = -3; // inicijalizujem da ne prijavljivalo gresku. sistemski poziv dodeljuje pravi ID
	systemCallData newData;
	newData.objectID = kernelSemID;
	newData.name = SEMAPHORECONSTRUCTOR;
	newData.init = init;
	newData.mySem = this;
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
	// new KernelSem(init, this);
}

Semaphore::~Semaphore() {
	systemCallData newData;
	newData.objectID = kernelSemID;
	newData.name = SEMAPHOREDESTRUCTOR;
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
	// delete allSem[kernelSemID];
}

int Semaphore::wait(int toBlock) {
	systemCallData newData;
	newData.objectID = kernelSemID;
	newData.name = SEMWAIT;
	newData.toBlock = toBlock;
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
	return newData.retValue; // kernel thread ce upisati povratnu vrednost u ovo polje preko data pokazivaca
	//return allSem[kernelSemID]->wait(toBlock);
}

void Semaphore::signal() {
	systemCallData newData;
	newData.objectID = kernelSemID;
	newData.name = SEMSIGNAL;
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
	// allSem[kernelSemID]->signal();
}

int Semaphore::val() const {
	systemCallData newData;
	newData.objectID = kernelSemID;
	newData.name = VAL;
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
	return newData.retValue;
	// return allSem[kernelSemID]->val();
}
