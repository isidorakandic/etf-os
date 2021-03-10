#ifndef _event_h_
#define _event_h_

// ovde su definicije klasa Event i IVTEntry

#include "thread.h" // zbog id polja

typedef unsigned char IVTNo;

#define PREPAREENTRY(numEntry, callOld)\
	void interrupt inter##numEntry(...);\
	IVTEntry newEntry##numEntry(numEntry, inter##numEntry);\
	void interrupt inter##numEntry(...){\
		newEntry##numEntry.signal();\
		if (callOld == 1) {\
			newEntry##numEntry.callOldRoutine();\
		}\
	}

class KernelEv;

class Event {
public:
	Event(IVTNo ivtNo);
	~Event();
	void wait();
protected:
	friend class KernelEv;
	void signal(); // can call KernelEv
private:
	ID kernelEvID;
};

typedef void interrupt (*pInterrupt)(...);
const int numOfEntries = 256;

class IVTEntry {
public:
	IVTEntry(IVTNo ivtEnt, pInterrupt newRoutine);
	~IVTEntry();
	void callOldRoutine();
	void signal();
	void setEvent(KernelEv* ev);
private:
	pInterrupt oldRoutine;
	IVTNo ivtEntry;
	KernelEv* event;
};

extern IVTEntry** allEntries;

#endif
