#ifndef _userMT_h_
#define _userMT_h_

#include "thread.h"
#include <stdio.h>


const Time userMTTimeSlice = 0;

class userMT: public Thread {
public:
	userMT(int main_argc, char* main_argv[]) :
			argc(main_argc), argv(main_argv), res(-1), Thread(defaultStackSize,
					userMTTimeSlice) {
	}
	virtual ~userMT() {
		waitToComplete();
	}
	int result();
protected:
	virtual void run();
private:
	int res;
	int argc;
	char** argv;
};

#endif
