#include "system.h"
#include "userMT.h"
#include <stdio.h>

int main(int argc, char* argv[]) {

	// inicijalizujemo sistem
	system_init();

	// napravim novu nit u okviru koje cu pokrenuti userMain()
	userMT* userMainThread = new userMT(argc, argv);
	userMainThread->start();

	// sacekamo da zavrsi da bi mogli da pokupimo rezultat
	userMainThread->waitToComplete();
	int res = userMainThread->result();

	// restauriramo sistem
	system_restore();

	// vracamo rezutat userMain niti
	return res;
}
