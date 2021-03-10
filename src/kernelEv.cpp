#include "kernelEv.h"
#include "system.h"
#include "schedule.h"

KernelEv::KernelEv(IVTNo ivtNo, Event* myEv) {
	value = 0; // inicijalno, dogadjaj se nije dogodio
	blocked = 0; // na pocetku nit nije blokirana
	myThread = (PCB*) running;
	ivtNum = ivtNo;
	myEvent = myEv;
	allEntries[ivtNum]->setEvent(this); // vezujemo kernelEvent sa IVTEntry objektom
	myEvent->kernelEvID = addEvent(this); // ubacimo sami sebe u niz svih eventova
}

KernelEv::~KernelEv() {
}

void KernelEv::signal() {
	if (blocked == 1) {
		// odblokiram nit i stavim je u red spremnih
		myThread->state = Ready;
		Scheduler::put(myThread); // stavljamo u scheduler ali ne menjamo kontekst
		blocked = 0; // nit vise nije blokirana!
	} else {
		value = 1; // ako nit nije blokirana samo zapamtim da se dogadjaj desio
		// nije bitno da je value bilo 0 ili 1 pre ovoga
	}
}

void KernelEv::wait() {
	if (running != myThread)
		return;
	if (value == 0) {
		blocked = 1; // nit se blokirala!
		running->state = Blocked;
		// dispatch();
	} else {
		value = 0; // nit je prosla ali brisemo fleg da se dogadjaj desio
	}
}
