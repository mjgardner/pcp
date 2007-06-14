/*
 * Copyright (c) International Business Machines Corp., 2002
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * Contact information: Silicon Graphics, Inc., 1500 Crittenden Lane,
 * Mountain View, CA 94043, USA, or: http://www.sgi.com
 */

/*
 * This code contributed by Mike Mason (mmlnx@us.ibm.com)
 * $Id: sem_limits.h,v 1.3 2004/06/24 06:15:36 kenmcd Exp $
 */

#ifdef _SEM_SEMUN_UNDEFINED
/* glibc 2.1 no longer defines semun, instead it defines
 * _SEM_SEMUN_UNDEFINED so users can define semun on their own.
 */
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};
#endif

typedef struct {
	unsigned int semmap; /* # of entries in semaphore map */
	unsigned int semmni; /* max # of semaphore identifiers */
	unsigned int semmns; /* max # of semaphores in system */  
	unsigned int semmnu; /* num of undo structures system wide */
	unsigned int semmsl; /* max num of semaphores per id */
	unsigned int semopm; /* max num of ops per semop call */
	unsigned int semume; /* max num of undo entries per process */
	unsigned int semusz; /* sizeof struct sem_undo */
	unsigned int semvmx; /* semaphore maximum value */
	unsigned int semaem; /* adjust on exit max value */
} sem_limits_t;

extern int refresh_sem_limits(sem_limits_t*);

