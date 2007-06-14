/*
 * Copyright (c) 1999 Silicon Graphics, Inc.  All Rights Reserved.
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

#ident "$Id: match_inst.c,v 1.1 2000/12/05 03:42:25 max Exp $"

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include "pmapi.h"	/* _pmCtime only */
#include "impl.h"	/* _pmCtime only */
#include "dstruct.h"
#include "fun.h"
#include "show.h"

/*
 * x-arg1 is the bexp, x->arg2 is the regex
 */
void
cndMatch_inst(Expr *x)
{
    Expr        *arg1 = x->arg1;
    Expr        *arg2 = x->arg2;
    Truth	*ip1;
    Truth	*op;
    int		n;
    int         i;
    int		sts;
    int		mi;
    Metric	*m;

    EVALARG(arg1)
    ROTATE(x)


#if PCP_DEBUG
    if (pmDebug & DBG_TRACE_APPL2) {
	fprintf(stderr, "match_inst(" PRINTF_P_PFX "%p): regex handle=" PRINTF_P_PFX "%p desire %s\n",
	    x, arg2->ring, x->op == CND_MATCH ? "match" : "nomatch");
	dumpExpr(x);
    }
#endif

    if (x->tspan > 0) {

	mi = 0;
	m = &arg1->metrics[mi++];
	i = 0;
	ip1 = (Truth *)(&arg1->smpls[0])->ptr;
	op = (Truth *)(&x->smpls[0])->ptr;

	for (n = 0; n < x->tspan; n++) {

	    if (!arg2->valid || !arg1->valid) {
		*op++ = DUNNO;
	    }
	    else if (x->e_idom <= 0) {
		*op++ = FALSE;
	    }
	    else {
		while (i >= m->m_idom) {
		    /*
		     * no more values, next metric
		     */
		    m = &arg1->metrics[mi++];
		    i = 0;
		}

		if (m->inames == NULL) {
		    *op++ = FALSE;
		}
		else {
		    sts = regexec((regex_t *)arg2->ring, m->inames[i], 0, NULL, 0);
#if PCP_DEBUG
		    if (pmDebug & DBG_TRACE_APPL2) {
			if (x->op == CND_MATCH && sts != REG_NOMATCH) {
			    fprintf(stderr, "match_inst: inst=\"%s\" match && %s\n",
				m->inames[i],
				*ip1 == TRUE ? "true" :
				    (*ip1 == FALSE ? "false" :
					(*ip1 == DUNNO ? "unknown" : "bogus" )));

			}
			else if (x->op == CND_NOMATCH && sts == REG_NOMATCH) {
			    fprintf(stderr, "match_inst: inst=\"%s\" nomatch && %s\n",
				m->inames[i],
				*ip1 == TRUE ? "true" :
				    (*ip1 == FALSE ? "false" :
					(*ip1 == DUNNO ? "unknown" : "bogus" )));
			}
		    }
#endif
		    if ((x->op == CND_MATCH && sts != REG_NOMATCH) ||
			(x->op == CND_NOMATCH && sts == REG_NOMATCH))
			*op++ = *ip1 && TRUE;
		    else
			*op++ = *ip1 && FALSE;
		}
		i++;
	    }
	    ip1++;
	}
	x->valid++;
    }
    else
	x->valid = 0;

    x->smpls[0].stamp = arg1->smpls[0].stamp;

#if PCP_DEBUG
    if (pmDebug & DBG_TRACE_APPL2) {
	fprintf(stderr, "cndMatch_inst(" PRINTF_P_PFX "%p) ...\n", x);
	dumpExpr(x);
    }
#endif
}

