/* ****************************************************************************
 *
 * © Copyright 2007,2008 Codefidence Ltd. 
 * All rights reserved unless explictly stated otherwise.
 *
 * This file is part of CoProc.
 *
 * CoProc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

#include "coproc.h"

#define ONLY_MSPACES 1
#define USE_DL_PREFIX 1
#define HAVE_MORECORE 0
#define HAVE_MMAP 0
#define HAVE_MREMAP 0
#define USE_LOCKS 1

#include "malloc.c"

extern char *program_invocation_name;

struct coproc_global {
	size_t size;
	void * pshared;
	mspace msp;
	size_t title_len;
};

static struct coproc_global global;

int coproc_init(size_t shm_max_size) {

	char *last_argv;
	int i, environsize;
	extern char **environ;
	char **tmp_env;

	if(setpgrp()) return -1;

	global.size = shm_max_size + TOP_FOOT_SIZE;

	global.pshared = mmap(NULL, global.size, PROT_READ | PROT_WRITE, \
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if(MAP_FAILED == global.pshared) {
		global.pshared = NULL;
		return -1;
	}

	global.msp = create_mspace_with_base(global.pshared, global.size, 1);

	if(!global.msp) {
		munmap(global.pshared, global.size);
		global.pshared = NULL;
		return -1;
	}

	for(i = 0; environ[i] != NULL; i++) {
                last_argv = environ[i] + strlen(environ[i]);
        }

	for(i = environsize = 0; environ[i] != NULL; i++) {
		environsize += strlen(environ[i]) + 1;
	}
  
  	if((tmp_env = (char **) malloc((i + 1) * sizeof(char *))) != NULL ) {

    		for(i = 0; environ[i] != NULL; i++) {
      			if((tmp_env[i] = malloc(strlen(environ[i]) + 1)) != NULL)
				strcpy(tmp_env[i], environ[i]);
    		}

    		tmp_env[i] = NULL;
		environ = tmp_env;
  	}

	global.title_len = last_argv - program_invocation_name - 1;

	return 0;

}

static inline int set_limit(int resource, rlim_t value) {

	struct rlimit limit = {
		.rlim_cur = value,
		.rlim_max = value
	};

	return setrlimit(resource, &limit);
}


pid_t coproc_create(char * coproc_name, struct coproc_attributes * attrib, \
	int flags, int (* start_routine)(void *), void * arg) {

	pid_t pid;
	pid_t * child_pid;
	int clone_flags = SIGCHLD;

#define COND_SET_FLAG(flg, cflg) if(flags & flg) clone_flags |= cflg

	COND_SET_FLAG(COPROC_SHARE_FS, (CLONE_FS | CLONE_FILES));
	COND_SET_FLAG(COPROC_SHARE_SYSVSEM, CLONE_SYSVSEM);
	COND_SET_FLAG(COPROC_NOTIFY_PARENT, CLONE_PARENT);

#undef COND_SET_FLAG

	if(flags & COPROC_DETACHED) {

		pid_t detached_pid;
		pid_t tmp;

		child_pid = coproc_alloc(sizeof(pid_t));

		if(!child_pid) return -1;

		detached_pid = syscall(SYS_clone, clone_flags, NULL, NULL, NULL);

		if(-1 == detached_pid) return -1;
		
		if(detached_pid) {
			int dummy;
	
			waitpid(detached_pid, &dummy, 0);
			tmp = *child_pid;
			coproc_free(child_pid);
			return tmp;
		}
	} 

	pid = syscall(SYS_clone, clone_flags, NULL, NULL, NULL );

	if(-1 == pid) { 
		if(child_pid) *child_pid = -1;
		return -1;
	}

	if(pid) {
		/* Parent */

		if(flags & COPROC_DETACHED) {
			*child_pid = pid;
			_exit(0);
		} else {
			return pid;
		}

	} else {
		/* Child */

		strncpy(program_invocation_name, coproc_name, global.title_len);
		program_invocation_name[global.title_len + 1] = 0;

#define COND_SET_ATTR(flg, res, val) if((flags & flg) && \
	(!attrib || set_limit(res, attrib->val))) return -1

		COND_SET_ATTR(COPROC_SET_AS_SIZE, RLIMIT_AS, address_space_size);
		COND_SET_ATTR(COPROC_SET_CORE_SIZE, RLIMIT_CORE, core_file_size);
		COND_SET_ATTR(COPROC_SET_CPU_TIME, RLIMIT_CPU, cpu_time);
		COND_SET_ATTR(COPROC_SET_STACK_SIZE, RLIMIT_STACK, stack_size);

#undef COND_SET_ATTR

		if(flags & COPROC_SET_SCHED) {

			struct sched_param param = {
				.sched_priority = attrib->scheduling_param
			};

			if((!attrib) || sched_setscheduler(0, \
				attrib->scheduling_policy, &param)) {
					return -1;
			}
		}
	
		if(flags & COPROC_SET_CPU_AFFINITY) {
		
			if((!attrib) || sched_setaffinity(0,  \
				sizeof(cpu_set_t), &attrib->cpu_affinity_mask)) { 
					return -1;
			}
		}

	exit(start_routine(arg));

	}

}

int coproc_kill(int sig) {

	return kill(0, sig);
}

int coproc_exit(void) {

	coproc_kill(SIGTERM);
	exit(0);

}

int coproc_is_alive(pid_t pid) {

	return kill(pid, 0);
}

void * coproc_alloc(size_t size) {

	return mspace_malloc(global.msp, size);
}

void coproc_free(void *ptr) {

	return mspace_free(global.msp, ptr);
}

int coproc_join(pid_t pid, int * status) {

	return waitpid(pid, status, __WALL);
}
