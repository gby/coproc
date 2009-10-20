
/* ****************************************************************************
 *
 * © Copyright 2007,2008,2009 Codefidence Ltd. 
 * All rights reserved unless explictly stated otherwise.
 *
 * This file is part of CoProc.
 *
 * CoProc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation, either version 3 of 
 * the License, or (at your option) any later version.
 * 
 * CoProc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CoProc.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

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

	pid = coproc_create("very_very_very long test_coproc name", NULL, \
			COPROC_SHARE_FS | COPROC_SHARE_SYSVSEM  | COPROC_DETACHED, \
			cfunc, test_mem);

	if(pid < 0) abort();

	*test_mem = 'A';

	printf("PID: %d\n", pid);

	ret = coproc_join(pid, &ret);

	

	printf("Got: %d, %c, %s %s %s\n", ret, *test_mem, strerror(errno), getenv("HOME"), environ[0]);

	coproc_free(test_mem);

	return 0;
}
