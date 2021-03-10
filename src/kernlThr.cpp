#include "kernlThr.h"
#include "system.h"
#include "pcb.h"
#include "kernlSem.h"
#include "kernelEv.h"
#include <stdio.h>

void kernlThr::run() {
	// ne treba while(1) petlja jer se pri promeni konteksta pc uvek postavlja na pocetak run metode
	switch (data->name) {
	case START:
		allPCB[data->objectID]->start();
		break;
	case WAITTOCOMPLETE:
		allPCB[data->objectID]->waitToComplete();
		break;
	case THREADDESTRUCTOR:
		if (allPCB[data->objectID] != 0) { // za svaki slucaj proverimo
			delete allPCB[data->objectID]; // ovo ce pozvati destruktor pcb-a za ovu nit
			allPCB[data->objectID] = 0; // brisemo iz niza ovaj pcb
		}
		break;
	case SLEEP:
		PCB::sleep(data->timeToSleep);
		break;
	case THREADCONSTRUCTOR:
		new PCB(data->stackSize, data->timeSlice, data->myThread);
		break;
	case DISPATCH:
		dispatchWanted = 1; // samo postavimo fleg pa se promena vrsi u kernelToUser
		break;
	case CLEARWAITLIST:
		allPCB[data->objectID]->clearWaitList();
		break;
	case SEMAPHORECONSTRUCTOR:
		new KernelSem(data->init, data->mySem);
		break;
	case SEMAPHOREDESTRUCTOR:
		if (allSem[data->objectID] != 0) {
			delete allSem[data->objectID];
			allSem[data->objectID] = 0;
		}
		break;
	case SEMWAIT:
		data->retValue = allSem[data->objectID]->wait(data->toBlock);
		break;
	case SEMSIGNAL:
		allSem[data->objectID]->signal();
		break;
	case VAL:
		data->retValue = allSem[data->objectID]->val();
		break;
	case EVENTCONSTRUCTOR:
		new KernelEv(data->ivtNo, data->myEv);
		break;
	case EVENTDESTRUCTOR:
		if (allEvents[data->objectID] != 0) {
			delete allEvents[data->objectID];
			allEvents[data->objectID] = 0;
		}
		break;
	case EVENTWAIT:
		allEvents[data->objectID]->wait();
		break;
	case EVENTSIGNAL:
		allEvents[data->objectID]->signal();
		break;
	}
	kernelToUser(); // promena konteksta
}
