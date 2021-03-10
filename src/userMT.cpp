#include "userMT.h"
#include "system.h" // jer je tu deklarisano userMain

int userMT::result() {
	return res;
}
void userMT::run() { // izvrsava funkciju userMain()
	res = userMain(argc, argv);
}
