#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "coproc.h"

extern char ** environ;

int cfunc(void * arg) {

	char * p = (void *)arg;

	printf("child got: %c\n", *p);

	sleep(12);

	*p = 'C';

	printf("child set: %c %s\n", *p, getenv("HOME"));

	return 0;
}


int main (int argc, char ** argv) {

	int pid, ret;
	char * test_mem;
	
	coproc_init(1024 * 1024);

	test_mem = coproc_alloc(1024);

        if(!test_mem)  abort();

        *test_mem = 'B';
	printf("PPID: %d\n", getpid());

	pid = coproc_create("very_very_very long test_coproc name", NULL, COPROC_SHARE_FS | COPROC_SHARE_SYSVSEM  | COPROC_DETACHED, \
		cfunc, test_mem);

	if(pid < 0) abort();

	*test_mem = 'A';

	printf("PID: %d\n", pid);

	ret = coproc_join(pid, &ret);

	

	printf("Got: %d, %c, %s %s %s\n", ret, *test_mem, strerror(errno), getenv("HOME"), environ[0]);

	coproc_free(test_mem);

	return 0;
}
