#include "pcb.h"
#include "system.h"
#include "schedule.h"
#include <stdio.h>

// definicija sleepList-a
volatile sleepElem* sleepList = 0;

const Time mainTimeSlice = 0;

PCB::PCB(StackSize stackSize, Time timeSlice1, Thread* myThread1) {
	myThread = myThread1;
	waitList = 0;
	stack = new unsigned[stackSize / sizeof(unsigned)];
	// delimo sa ovim jer je stackSize u bajtovima, a unsigned ima 16b
#ifndef BCC_BLOCK_IGNORE
	stack[stackSize-1] = FP_SEG(myThread1);
	stack[stackSize-2] = FP_OFF(myThread1);
	stack[stackSize-6] = FP_SEG(Thread::wrapper);
	stack[stackSize-7] = FP_OFF(Thread::wrapper);
#endif
	stack[stackSize - 5] = initPSW;
	sp = ss = 0;
#ifndef BCC_BLOCK_IGNORE
	ss = FP_SEG(stack+stackSize-16);
	sp = FP_OFF(stack+stackSize-16);
#endif
	bp = sp;
	state = New;
	timeSlice = timeSlice1;
	myThread->pcbID = addPCB(this); // pcb sam sebe doda u niz allPCB
}

PCB::PCB() { // konstruktor za PCB main niti
	// ova polja nam trebaju
	timeSlice = mainTimeSlice; // nit ce se prvi put izvrsavati nesto duze od timeSlice jer ce odbrojavanje poceti tek kad omogucimo prekide
	state = Ready; // ova nit je vec running tako da preskacemo New state
	// ova polja necemo korisiti
	myThread = 0;
	waitList = 0;
	stack = 0; // stek main-a nije u nasoj nadleznosti da dealociramo
	ss = 0; // vrednost ovih polja nam nisu bitne jer ce odgovarajuce vrednosti biti ovde sacuvane pri promeni konteksta
	sp = 0;
	bp = 0;
}

PCB::~PCB() {
	if (stack != 0) { // ako nije pcb main niti u pitanju
		delete[] stack;
		stack = 0;
	}
}

void PCB::start() {
	if (state != New)
		return;
	state = Ready;
	Scheduler::put(this);
}

void PCB::waitToComplete() {
	if (state == Finished || running->myThread->pcbID == myThread->pcbID) {
		// treba obezbediti da se ne radi nista ako je nit vec zavrsila ili ako nit zove sama za sebe
		return;
	}
	waitElem* newElem = new waitElem();
	newElem->threadPCB = (PCB*) running;
	newElem->next = waitList;
	waitList = newElem;
	running->state = Blocked;
	// dispatch();
}

void PCB::sleep(Time timeToSleep) {
	if (timeToSleep == 0)
		return;
	// pravimo novi element liste
	sleepElem* newElem = new sleepElem();
	newElem->threadPCB = (PCB*) running;
	if (sleepList == 0) {
		newElem->t = timeToSleep;
		newElem->next = 0;
		// novi element je nova glava liste
		sleepList = newElem;
	} else {
		if (timeToSleep < sleepList->t) { // ide na pocetak
			newElem->t = timeToSleep;
			newElem->next = (sleepElem*) (sleepList);
			sleepList->t -= newElem->t; // ubacili smo novi element na pocetak i sad umanjimo za njegov time time stare glave liste
			// novi element je nova glava liste
			sleepList = newElem;
		} else { // sleepList != 0 && timeToSleep >= sleepList->t
			newElem->t = timeToSleep - sleepList->t;
			sleepElem* tmp = sleepList->next;
			sleepElem* prev = (sleepElem*) sleepList;
			while (tmp != 0 && tmp->t < newElem->t) {
				newElem->t -= tmp->t;
				prev = tmp;
				tmp = tmp->next;
			}
			prev->next = newElem;
			newElem->next = tmp;
			if (tmp != 0) // ako sledeci postoji umanjimo ga koliko je potrebno
				tmp->t -= newElem->t;
		}
	}

	// blokiramo trenutnu nit
	running->state = Blocked;
	// dispatch();
}

void PCB::clearWaitList() { // ovo se poziva iz Thread::wrapper()
	waitElem* tmp = 0;
	while (running->waitList != 0) {
		running->waitList->threadPCB->state = Ready;
		Scheduler::put(running->waitList->threadPCB);
		tmp = running->waitList;
		running->waitList = running->waitList->next;
		delete tmp; // brisemo objekat waitElem
	}
	running->state = Finished; // kraj izvrsavanja niti
	// dispatch();
}
