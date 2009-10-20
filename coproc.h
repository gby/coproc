/******************************************************************************
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

#ifndef _COPROC_H_
#define _COPROC_H_

#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/wait.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/prctl.h>

struct coproc_attributes {

	rlim_t  address_space_size; 	/* The maximum size of the process 
						address space in bytes */
	rlim_t  core_file_size;		/* Maximum size of core file */
	rlim_t  cpu_time;		/* CPU time limit in seconds */
	rlim_t  stack_size;		/* The maximum size of the process 
						stack, in bytes */
	int 	  scheduling_policy; 	/* Scheduling policy. */
	int 	  scheduling_param;	/* Scheduling priority or nice level */
	cpu_set_t cpu_affinity_mask;	/* The CPU mask of the co-process */
};

#define	COPROC_NOTIFY_PARENT 	(1)
#define COPROC_SHARE_FS 	(1<<1)
#define COPROC_DETACHED         (1<<2)
#define	COPROC_SHARE_SYSVSEM	(1<<3)
#define	COPROC_SET_AS_SIZE	(1<<4)
#define	COPROC_SET_CORE_SIZE	(1<<5)
#define	COPROC_SET_CPU_TIME	(1<<6)
#define	COPROC_SET_STACK_SIZE	(1<<6)
#define	COPROC_SET_SCHED	(1<<7)
#define	COPROC_SET_CPU_AFFINITY (1<<8)

/******************************************************************************
 * coproc_init
 *
 * Init coproc library 
 *
 * Call must be done prior * to the first use of any of the 
 * other coproc library APIs 
 *
 * shm_max_size:   the maximum number, in bytes of allocatable shared memory.
 */

int coproc_init(size_t shm_max_size);

/******************************************************************************
 * coproc_create
 * 
 * Create a new co-process 
 *
 * coproc_name:   a string with the new co-process name.
 *
 * attrib:	  the address of a coproc_attributes structure with the
 * 			attributes for the newly created co-process or
 * 			NULL for the default attributes.
 *
 * flags:	  a mask of one or more of:	
 *
 * 				COPROC_NOTIFY_PARENT - make the new co-process 
 * 				inherit creator parent as opposed to the default
 * 				of making the creator the parent.
 *
 * 				COPROC_SHARE_FS - new co-process will share
 * 				file system, file descriptors and IO context
 * 				with creator as opposed of getting a copy
 * 				of the creator resources.
 *
 * 				COPROC_SHARE_PTRACE - trace new co-process 
 * 				when creator is traced (debugged).
 *
 * 				COPROC_SHARE_SYSVSEM - new co-process will
 * 				share SystemV semaphores undo values as 
 * 				oppoused to default of getting an empty set.
 *
 * 				COPROC_SET_AS_SIZE - set address space size
 * 				of new co-process according to the limit 
 * 				specified in attributes argument as oppused 
 * 				to inherting from creator.
 *
 * 				COPROC_SET_CORE_SIZE - set the core size
 * 				of the new co-process according to the limit
 * 				specified in the attributes argument as 
 * 				oppused of inhgerting from creator.
 *
 * 				COPROC_SET_CPU_TIME - set the maximum cpu
 * 				time for the new co-proccess according to the
 * 				limit specified in the attributes argument as
 * 				oppused of inhgerting from creator.
 *
 * 				COPROC_SET_STACK_SIZE - set the maximum stack
 * 				size for the new co-proccess according to the
 * 				limit specified in the attributes argument as
 * 				oppused of inhgerting from creator.
 *
 * 				COPROC_SET_SCHED - set the scheduling policy 
 * 				and priority according to the attributes
 * 				argument oppused of inhgerting from creator.
 *
 * 				COPROC_SET_CPU_AFFINITY - set the CPU affinity
 * 				mask of the new co-process according to the 
 * 				attributes argument oppused of inhgerting from 
 * 				creator.
 *
 * start_routine: the address of the function to call in the new co-process.
 * 
 * arg:		  an argument passed to the start_routine when called.
 */

pid_t coproc_create(char * coproc_name, struct coproc_attributes * attrib, \
        int flags, int (* start_routine)(void *), void * arg);

/******************************************************************************
 * coproc_exit
 *
 * Terminate the program and all associated co-processes 
 */

int coproc_exit(void);

/******************************************************************************
 * coproc_alloc
 *
 * Allocate shared memory that is visible from all co-processes
 *
 * size:	the size in bytes of memory to shared by all co-processes to
 * 			to allocate.
 */

void * coproc_alloc(size_t size);

/******************************************************************************
 * coproc_free
 *
 * Free memory previously allocated by a call to coproc_alloc
 *
 * ptr:		an address previosuly provided by coproc_alloc to free.
 */

void coproc_free(void *ptr);

/******************************************************************************
 * coproc_join
 *
 * Synchronize on the termination of a co-processes and read it's return 
 * status
 *
 * pid:		the process ID of the co-process to join.
 *
 * status:	the address of an integer to which the co-process status 
 * 			is copied.
 */

int coproc_join(pid_t pid, int * status);

#endif /* _COPROC_H_ */
