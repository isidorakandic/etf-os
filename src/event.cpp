#include "event.h"
#include "kernelEv.h"
#include "system.h"

Event::Event(IVTNo ivtNo) {
	kernelEvID = -3;
	systemCallData newData;
	newData.objectID = kernelEvID;
	newData.name = EVENTCONSTRUCTOR;
	newData.ivtNo = ivtNo;
	newData.myEv = this;
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
	// new KernelEv(ivtNo, this);
}

Event::~Event() {
	systemCallData newData;
	newData.objectID = kernelEvID;
	newData.name = EVENTDESTRUCTOR;
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
	// delete allEvents[kernelEvID];
}

void Event::signal() {
	systemCallData newData;
	newData.objectID = kernelEvID;
	newData.name = EVENTSIGNAL;
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
	// allEvents[kernelEvID]->signal();
}

void Event::wait() {
	systemCallData newData;
	newData.objectID = kernelEvID;
	newData.name = EVENTWAIT;
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
	// allEvents[kernelEvID]->wait();
}
