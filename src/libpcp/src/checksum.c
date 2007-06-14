/*
 * Copyright (c) 1995 Silicon Graphics, Inc.  All Rights Reserved.
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
 * __pmCheckSum(FILE *f) - algorithm stolen from sum(1), changed from 16-bit
 * to 32-bit
 */

#include <stdio.h>
#if defined(sgi)
#include <sgidefs.h>
#endif
#include "pmapi.h"
#include "impl.h"

__int32_t
__pmCheckSum(FILE *f)
{
    __int32_t	sum = 0x19700520;
    int		c;

    while ((c = fgetc(f)) != EOF) {
	if (sum & 1)
	    sum = (sum >> 1) + 0x80000000;
	else
	    sum >>= 1;
	sum += c;
    }
    return sum;
}
