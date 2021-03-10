#include "kernlSem.h"
#include "pcb.h"
#include "schedule.h"
#include "system.h"
#include <stdio.h>

KernelSem::KernelSem(int init, Semaphore* sem) {
	value = init;
	blockedList = 0;
	mySem = sem;
	mySem->kernelSemID = addSem(this);
}

KernelSem::~KernelSem() { // u slucaju da na semaforu i dalje ima blokiranih niti ni jedno resenje nije idealno!
	if (blockedList != 0) { // samo odblokiramo blokirane niti i stavimo ih u scheduler
		//printf("destruktor semafora: blockedList !=0\n");
		waitElem* tmp = blockedList;
		while (blockedList != 0) {
			blockedList->threadPCB->state = Ready;
			Scheduler::put(blockedList->threadPCB); // moze da stavlja u scheduler niti, samo ne moze da menja kontekst
			tmp = blockedList;
			blockedList = blockedList->next;
			delete tmp;
		}
	}
}

int KernelSem::wait(int toBlock) {
	int ret = 0; // slucaj da se nit ne blokira
	if (toBlock != 0) {
		value--;
		if (value < 0) { // nit se blokira
			waitElem* newBlocked = new waitElem();
			newBlocked->next = blockedList;
			newBlocked->threadPCB = (PCB*) running;
			blockedList = newBlocked;
			running->state = Blocked;
			// dispatch(); // u slucaju sistemskih poziva ne menjamo ovde kontekst
			ret = 1; // 1 ako se nit blokirala se vraca
		}
	} else { // toBlock == 0
		if (value <= 0) {
			ret = -1;
		} else {
			value--;
		}
	}
	return ret;
}

void KernelSem::signal() {
	value++;
	if (value <= 0) {
		waitElem* toUnblock = blockedList; // tmp elem
		toUnblock->threadPCB->state = Ready; // prvi iz liste blokiranih stavimo u scheduler
		Scheduler::put(toUnblock->threadPCB);
		blockedList = blockedList->next;
		delete toUnblock;
	}
}

int KernelSem::val() const {
	return value;
}
