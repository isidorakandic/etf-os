#ifndef _system_h_
#define _system_h_

#include "thread.h"
#include <dos.h>
#include "semaphor.h"
#include "event.h"
#include "kernlThr.h"

extern volatile unsigned int lockFlag; // zabrana promene konteksta bez zabrane prekida

extern volatile unsigned int tmpss;
extern volatile unsigned int tmpsp;
extern volatile unsigned int tmpbp;
extern volatile Time cnt;
extern unsigned oldTimerOFF, oldTimerSEG;
extern volatile PCB* running;
extern volatile int dispatchWanted;

extern PCB** allPCB;
extern int PCBnum;
extern ID nextPCBid;

extern KernelSem** allSem;
extern int semNum;
extern ID nextSemId;

extern KernelEv** allEvents;
extern int evNum;
extern ID nextEventId;

const int initPCBnum = 10;
const int initSemNum = 10;
const int initEvNum = 10;

extern volatile systemCallData* data;

void tick(); // samo deklarisemo ovo negde

int userMain(int argc, char* argv[]); // i ovo isto

void system_init();

ID addPCB(PCB* newPCB);

ID addSem(KernelSem* newSem);

ID addEvent(KernelEv* newEvent);

void system_restore();

void interrupt timer(...);

void interrupt sys_call(...);

// prekidna rutina za promenu konteksta sa kernel na user thread
void interrupt kernelToUser(...);

#endif
