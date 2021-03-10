#include "system.h"
#include "thread.h"
#include "schedule.h"
#include "pcb.h"
#include "idle.h"
#include "kernlSem.h"
#include "kernelEv.h"
#include "kernlThr.h"
#include "event.h"
#include <stdio.h>
#include <dos.h>

// zabranjuje promenu konteksta bez zabrane prekida
#define lock lockFlag=0;

// dozvoljava promenu konteksta
#define unlock lockFlag=1;

// definicija extern promenljivih
volatile Time cnt = defaultTimeSlice;
volatile int dispatchWanted = 0;
volatile unsigned int lockFlag = 1;

// pomocne promenljive timer-a
volatile unsigned int tmpss = 0;
volatile unsigned int tmpsp = 0;
volatile unsigned int tmpbp = 0;
// pomocne promenljive za azuriranje sleep niti u timer-u
volatile sleepElem* tmp = 0;
volatile sleepElem* prev = 0;

// pomocne promenljive sys_call-a
volatile unsigned int tmpss1 = 0;
volatile unsigned int tmpsp1 = 0;
volatile unsigned int tmpbp1 = 0;

// globalne promenljive koje koriste metode init i restore
pInterrupt oldTimerRoutine = 0;
kernlThr* kernelThread = 0; // objekat kernel thread-a
volatile Idle* idleThread;
PCB* mainPCB = 0;

// pokazivac na tekucu (running) nit i kernel nit
volatile PCB* runningKernelThread = 0; // pokazuje na kontekst kernel niti (pcb)
volatile PCB* running = 0;

// promenjive za odrzavanje evidencije o svim nitima
PCB** allPCB = 0; // interrupt ga ne menja pa nije volatile
int PCBnum = 0;
ID nextPCBid = 0; // indeksiranje niti pocinje od 0

// promenjive za odrzavanje evidencije o svim semaforima
KernelSem** allSem = 0;
int semNum = 0;
ID nextSemId = 0;

// promenjive za odrzavanje evidencije o svim eventovima
KernelEv** allEvents = 0;
int evNum = 0;
ID nextEventId = 0;

// pomocna metoda za dodavanje pcb u niz allPCB
ID addPCB(PCB* newPCB) {
	if (nextPCBid == PCBnum) {
		PCB** allPCBtmp = new PCB*[PCBnum + initPCBnum];
		for (int i = 0; i < PCBnum; i++) {
			allPCBtmp[i] = allPCB[i];
		}
		delete[] allPCB;
		allPCB = allPCBtmp;
		PCBnum += initPCBnum;
	}
	allPCB[nextPCBid] = newPCB;
	return nextPCBid++;
}

// pomocna metoda za dodavanje kernel semafora u niz allSem
ID addSem(KernelSem* newSem) {
	if (nextSemId == semNum) {
		KernelSem** allSemtmp = new KernelSem*[semNum + initSemNum];
		for (int i = 0; i < semNum; i++) {
			allSemtmp[i] = allSem[i];
		}
		delete[] allSem;
		allSem = allSemtmp;
		semNum += initSemNum;
	}
	allSem[nextSemId] = newSem;
	return nextSemId++;
}

// pomocna metoda za dodavanje kernel eventa u niz allEvents
ID addEvent(KernelEv* newEvent) {
	if (nextEventId == evNum) {
		KernelEv** allEvtmp = new KernelEv*[evNum + initEvNum];
		for (int i = 0; i < evNum; i++) {
			allEvtmp[i] = allEvents[i];
		}
		delete[] allEvents;
		allEvents = allEvtmp;
		evNum += initEvNum;
	}
	allEvents[nextEventId] = newEvent;
	return nextEventId++;
}

void system_init() {

	// inicijalizujemo ulaz za prekidnu rutinu za sistemske pozive i prekidnu rutinu za promenu konteksta sa kernel niti na user nit
#ifndef BCC_BLOCK_IGNORE
	setvect(0x61, sys_call);
	setvect(0x62, kernelToUser);
#endif

	// pravimo kontekst za trenutnu nit (koja izvrsava main)
	mainPCB = new PCB;
	running = mainPCB;

	// pravimo niz PCB-ova
	allPCB = new PCB*[initPCBnum];
	nextPCBid = 0;
	PCBnum = initPCBnum;

	// pravimo kernel nit - prva nit koja se pravi u sistemu i njen pcb je allPCB[0]
	kernelThread = new kernlThr; // nigde je ne startujemo jer ona ne ide u scheduler, bas kao i idle
	runningKernelThread = allPCB[0];

	// pravimo niz Semafora
	allSem = new KernelSem*[initSemNum];
	nextSemId = 0;
	semNum = initSemNum;

	// pravimo niz Eventova
	allEvents = new KernelEv*[initEvNum];
	nextEventId = 0;
	evNum = initEvNum;

	// pravimo idle nit - allPCB[1]
	idleThread = new Idle;

	// inicijalizujemo prekidnu rutinu za timer
#ifndef BCC_BLOCK_IGNORE
	oldTimerRoutine = getvect(0x8);
	setvect(0x8,timer);
	setvect(0x60, oldTimerRoutine);
#endif

}

void system_restore() {

	// brisemo sve niti u sistemu (ako nisu vec obrisane)
	for (int i = 2; i < nextPCBid; i++) { // allPCB[0] je kernel nit, a allPCB[1] je idle
		if (allPCB[i] != 0) {
			delete allPCB[i]->myThread;
		}
	}

	// brisemo sve semafore u sistemu (ako nisu vec obrisani)
	for (int j = 0; j < nextSemId; j++) {
		if (allSem[j] != 0) {
			delete allSem[j]->mySem;
		}
	}

	// brisemo sve eventove u sistemu (ako nisu vec obrisani)
	for (int k = 0; k < nextEventId; k++) {
		if (allEvents[k] != 0) {
			delete allEvents[k]->myEvent;
		}
	}

	// brisem alocirane promenljive
	delete idleThread;
	delete mainPCB;
	delete[] allPCB;
	delete[] allSem;
	delete[] allEvents;
	delete[] allEvents;
	delete kernelThread; // na kraju brisemo kernel nit nakon sto su izvrseni svi sistemski pozivi

	// vracamo staru prekidnu rutinu timera u odgovarajuci ulaz
#ifndef BCC_BLOCK_IGNORE
	setvect(0x8,oldTimerRoutine);
#endif
}

sleepElem* sleepTmp = 0;

void interrupt timer(...) {
	if (!dispatchWanted) { // ukoliko smo u prekidnu rutinu usli jer je proslo 55ms od prethodne:
		tick();
		// ako je running->timeSlice = 0 onda ce cnt biti < 0 i nikad nece ispuniti uslov za promenu konteksta cnt==0
		// sto je upravo ono sto smo hteli jer se onda promena desava samo za dispatchWanted
		cnt--;
		// azuriranje uspavanih niti
		if (sleepList != 0) {
			sleepList->t--;
			while (sleepList != 0 && sleepList->t == 0) {
				sleepList->threadPCB->state = Ready;
				Scheduler::put(sleepList->threadPCB);
				sleepTmp = (sleepElem*) sleepList;
				sleepList = sleepList->next;
				delete sleepTmp;
			}
		}
	}
	if (cnt == 0 || dispatchWanted) {
		if (lockFlag) {
			// izvrsava se promena konteksta
			// prvo cuvamo kontekst running niti
#ifndef BCC_BLOCK_IGNORE
			asm {
				mov tmpss, ss
				mov tmpsp, sp
				mov tmpbp, bp
			}
#endif
			running->ss = tmpss;
			running->sp = tmpsp;
			running->bp = tmpbp;
			if (running->state == Ready) { // ako ima i dalje da se izvrsava (idle nece ici jer je state new)
				Scheduler::put((PCB*) running);
			}
			running = Scheduler::get();
			if (running == 0) { // ako nema ni jedne spremne niti izvrsava se idle
				running = allPCB[1]; // allPCB[1] je idle nit; 0 je kernel nit, 1 je idle
			}
			// u registre procesora upisujemo kontekst nove running niti
			tmpss = running->ss;
			tmpsp = running->sp;
			tmpbp = running->bp;
			cnt = running->timeSlice;
#ifndef BCC_BLOCK_IGNORE
			asm {
				mov ss, tmpss
				mov sp, tmpsp
				mov bp, tmpbp
			}
#endif
			dispatchWanted = 0; // izvrsili smo promenu konteksta pa brisemo eventualni zahtev za njom
		} else {
			dispatchWanted = 1; // desila se potreba za promenom konteksta ali nije moguce sad je realizovati
			// zbog lockFlag-a i zato samo pamtimo da je to trebalo da uradimo tako sto postavimo dispatchWanted=1
		}
	}
	if (!dispatchWanted) { // pozivamo staru prekidnu rutinu
#ifndef BCC_BLOCK_IGNORE
	asm int 0x60
#endif
	}
}

volatile systemCallData* data = 0;
volatile unsigned tmpcx, tmpdx;

void interrupt sys_call(...) {
	// dozvoljavamo prekide ali ne i promenu konteksta
#ifndef BCC_BLOCK_IGNORE
	asm sti
#endif
	lock

	// dohvatamo parametre sistemskog poziva
#ifndef BCC_BLOCK_IGNORE
	asm {
		mov tmpcx, cx
		mov tmpdx, dx
	}
	data = (systemCallData*)MK_FP(tmpcx, tmpdx);
#endif

	// cuvamo kontekst running niti
#ifndef BCC_BLOCK_IGNORE
	asm {
		mov tmpss1, ss
		mov tmpsp1, sp
		mov tmpbp1, bp
	}
#endif
	running->ss = tmpss1;
	running->sp = tmpsp1;
	running->bp = tmpbp1;

	// prelazimo na kontekst kernel niti
	tmpss1 = runningKernelThread->ss;
	tmpsp1 = runningKernelThread->sp;
	tmpbp1 = runningKernelThread->bp;

#ifndef BCC_BLOCK_IGNORE
	asm {
		mov ss, tmpss1
		mov sp, tmpsp1
		mov bp, tmpbp1
	}
#endif
}

void interrupt kernelToUser(...) {
	// dozvoljamo prekide ali ne i promenu konteksta, lock je i dalje postavljeno!
#ifndef BCC_BLOCK_IGNORE
	asm sti
#endif

	// nema potrebe da cuvamo kontekst kernel niti jer njeno izvrsavanje uvek treba da se nastavi na pocetku run metode
	// tako da se samo:

	// vracamo se na user nit
	if (running->state != Ready || dispatchWanted == 1) {
		// u slucaju da je nit blokirana u toku sistemskog poziva ili je zavrsila svoje izvrsavanje
		// ili je zatrazena promena konteksta
		// ne vracamo kontekst niti koja je bila running vec nekoj drugoj iz schedulera
		if (dispatchWanted == 1)
			dispatchWanted = 0; // resetujemo fleg
		if (running->state == Ready) // ako smo usli ovde zbog dispatchWanted==1 a running je Ready ono treba da ide u scheduler
			Scheduler::put((PCB*) running);
		running = Scheduler::get();
		if (running == 0) {
			running = allPCB[1]; // allPCB[1] je idle nit; 0 je kernel nit, 1 je idle
		}
		cnt = running->timeSlice; // ako ne vrsimo promenu konteksta vec se vracamo na nit koja je pozvala sistemski poziv
		// onda ne treba da resetujemo cnt, sistemski poziv se racuna u vreme koje je running imala procesor i treba da odbrojava
		// dok mi obradjujemo taj poziv
	}

	// restauriramo kontekst user niti - to je running
	tmpss1 = running->ss;
	tmpsp1 = running->sp;
	tmpbp1 = running->bp;
#ifndef BCC_BLOCK_IGNORE
	asm {
		mov ss, tmpss1
		mov sp, tmpsp1
		mov bp, tmpbp1
	}
#endif
	unlock
	// unlock koji odgovara lock-u sa pocetka system_call
}
