/*
 * Linux Filesystem Cluster
 *
 * Copyright (c) 2000,2004 Silicon Graphics, Inc.  All Rights Reserved.
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

#ident "$Id: filesys.h,v 1.5 2004/06/24 06:15:36 kenmcd Exp $"

#include <sys/vfs.h>

typedef struct {
    int		  id;
    char	  *device;
    char	  *path;
    int		  fetched;
    int		  valid;
    int		  seen;
    struct statfs stats;
} filesys_entry_t;

typedef struct {
    int		  nmounts;
    filesys_entry_t *mounts;
    pmdaIndom 	*indom;
} filesys_t;

extern int refresh_filesys(filesys_t *);
