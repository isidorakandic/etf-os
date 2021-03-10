#include "event.h"
#include "kernelEv.h"
#include <dos.h>

IVTEntry** allEntries;

IVTEntry::IVTEntry(IVTNo ivtEnt, pInterrupt newRoutine) {
	ivtEntry = ivtEnt;
	oldRoutine = 0; // samo da ne bi prijavljivao gresku
	event = 0; // posebnom metodom uvezujemo event sa objektom IVTEntry
#ifndef BCC_BLOCK_IGNORE
			oldRoutine = getvect(ivtEntry);
			setvect(ivtEntry, newRoutine);
#endif
	if (allEntries == 0) {
		allEntries = new IVTEntry*[numOfEntries];
		for (int i = 0; i < numOfEntries; i++)
			allEntries[i] = 0;
	}
	if (allEntries[ivtEntry] != 0) { // u svaki ulaz, max jedan event
		return;
	}
	allEntries[ivtEntry] = this;
}

IVTEntry::~IVTEntry() {
// vracamo staru rutinu u odgovarajuci ulaz
#ifndef BCC_BLOCK_IGNORE
	setvect(ivtEntry, oldRoutine);
#endif
}

void IVTEntry::signal() {
	if (event == 0)
		return; // dogadjaj nije postavljen!
	event->signal();
}

void IVTEntry::callOldRoutine() {
	(*oldRoutine)();
}

void IVTEntry::setEvent(KernelEv* ev) {
	event = ev;
}
