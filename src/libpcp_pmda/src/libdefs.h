/*
 * Copyright (c) 1999-2000 Silicon Graphics, Inc.  All Rights Reserved.
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
 * 
 * Contact information: Silicon Graphics, Inc., 1500 Crittenden Lane,
 * Mountain View, CA 94043, USA, or: http://www.sgi.com
 *
 * $Id: libdefs.h,v 1.2 2000/12/28 09:17:25 max Exp $
 */

#ifndef LIBDEFS_H
#define LIBDEFS_H

#define HAVE_V_TWO(interface) (interface == PMDA_INTERFACE_2 || interface == PMDA_INTERFACE_3)

/*
 * Auxilliary structure used to save data from pmdaDSO or pmdaDaemon and
 * make it available to the other methods, also as private per PMDA data
 * when multiple DSO PMDAs are in use
 */
typedef struct {
    int		pmda_interface;
    pmResult	*res;			/* high-water allocation for */
    int		maxnpmids;		/* pmResult for each PMDA */
} e_ext_t;

#endif /* LIBDEFS_H */
