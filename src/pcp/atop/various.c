/*
** ATOP - System & Process Monitor
**
** The program 'atop' offers the possibility to view the activity of
** the system on system-level as well as process-level.
**
** This source-file contains various functions to a.o. format the
** time-of-day, the cpu-time consumption and the memory-occupation. 
**
** Copyright (C) 2000-2010 Gerlof Langeveld
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
*/

#include <pcp/pmapi.h>
#include <pcp/impl.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

#include "atop.h"
#include "hostmetrics.h"

/*
** Function convtime() converts a value (number of seconds since
** 1-1-1970) to an ascii-string in the format hh:mm:ss, stored in
** chartim (9 bytes long).
*/
char *
convtime(double timed, char *chartim)
{
	time_t		utime = (time_t) timed;
	struct tm 	tt;

	pmLocaltime(&utime, &tt);

	sprintf(chartim, "%02d:%02d:%02d", tt.tm_hour, tt.tm_min, tt.tm_sec);

	return chartim;
}

/*
** Function convdate() converts a value (number of seconds since
** 1-1-1970) to an ascii-string in the format yyyy/mm/dd, stored in
** chardat (11 bytes long).
*/
char *
convdate(double timed, char *chardat)
{
	time_t		utime = (time_t) timed;
	struct tm 	tt;

	pmLocaltime(&utime, &tt);

	sprintf(chardat, "%04d/%02d/%02d",
		tt.tm_year+1900, tt.tm_mon+1, tt.tm_mday);

	return chardat;
}


/*
** Convert a hh:mm string into a number of seconds since 00:00
**
** Return-value:	0 - Wrong input-format
**			1 - Success
*/
int
hhmm2secs(char *itim, unsigned int *otim)
{
	register int	i;
	int		hours, minutes;

	/*
	** check string syntax
	*/
	for (i=0; *(itim+i); i++)
		if ( !isdigit(*(itim+i)) && *(itim+i) != ':' )
			return(0);

	sscanf(itim, "%d:%d", &hours, &minutes);

	if ( hours < 0 || hours > 23 || minutes < 0 || minutes > 59 )
		return(0);

	*otim = (hours * 3600) + (minutes * 60);

	if (*otim >= SECSDAY)
		*otim = SECSDAY-1;

	return(1);
}


/*
** Function val2valstr() converts a value to an ascii-string of a fixed
** number of positions; if the value does not fit, it will be formatted to
** exponent-notation (as short as possible, so not via the standard printf-
** formatters %f or %e). The offered string should have a length of width+1.
** The value might even be printed as an average for the interval-time.
*/
char *
val2valstr(count_t value, char *strvalue, int width, int avg, int nsecs)
{
	count_t	maxval, remain = 0;
	int	exp     = 0;
	char	*suffix = "";

	if (avg)
	{
		value  = (value + (nsecs/2)) / nsecs;     /* rounded value */
		width  = width - 2;	/* subtract two positions for '/s' */
		suffix = "/s";
	}

	maxval = pow(10.0, width) - 1;

	if (value < maxval)
	{
		sprintf(strvalue, "%*lld%s", width, value, suffix);
	}
	else
	{
		if (width < 3)
		{
			/*
			** cannot avoid ignoring width
			*/
			sprintf(strvalue, "%lld%s", value, suffix);
		}
		else
		{
			/*
			** calculate new maximum value for the string,
			** calculating space for 'e' (exponent) + one digit
			*/
			width -= 2;
			maxval = pow(10.0, width) - 1;

			while (value > maxval)
			{
				exp++;
				remain = value % 10;
				value /= 10;
			}

			if (remain >= 5)
				value++;

			sprintf(strvalue, "%*llde%d%s",
					width, value, exp, suffix);
		}
	}

	return strvalue;
}

#define DAYSECS 	(24*60*60)
#define HOURSECS	(60*60)
#define MINSECS 	(60)

/*
** Function val2elapstr() converts a value (number of seconds)
** to an ascii-string of up to max 13 positions in NNNdNNhNNmNNs
** stored in strvalue (at least 14 positions).
** returnvalue: number of bytes stored
*/
int
val2elapstr(int value, char *strvalue)
{
        char	*p=strvalue, doshow=0;

        if (value > DAYSECS) 
        {
                p+=sprintf(p, "%dd", value/DAYSECS);
                value %= DAYSECS;
		doshow = 1;
        }

        if (value > HOURSECS || doshow) 
        {
                p+=sprintf(p, "%dh", value/HOURSECS);
                value %= HOURSECS;
		doshow = 1;
        }

        if (value > MINSECS || doshow) 
        {
                p+=sprintf(p, "%dm", value/MINSECS);
                value %= MINSECS;
		doshow = 1;
        }

        if (value || doshow) 
        {
                p+=sprintf(p, "%ds", value);
		doshow = 1;
        }

        return p-strvalue;
}


/*
** Function val2cpustr() converts a value (number of milliseconds)
** to an ascii-string of 6 positions in milliseconds or minute-seconds or
** hours-minutes, stored in strvalue (at least 7 positions).
*/
#define	MAXMSEC		(count_t)100000
#define	MAXSEC		(count_t)6000
#define	MAXMIN		(count_t)6000

char *
val2cpustr(count_t value, char *strvalue)
{
	if (value < MAXMSEC)
	{
		sprintf(strvalue, "%2lld.%02llds", value/1000, value%1000/10);
	}
	else
	{
	        /*
       	 	** millisecs irrelevant; round to seconds
       	 	*/
        	value = (value + 500) / 1000;

        	if (value < MAXSEC) 
        	{
               	 	sprintf(strvalue, "%2lldm%02llds", value/60, value%60);
		}
		else
		{
			/*
			** seconds irrelevant; round to minutes
			*/
			value = (value + 30) / 60;

			if (value < MAXMIN) 
			{
				sprintf(strvalue, "%2lldh%02lldm",
							value/60, value%60);
			}
			else
			{
				/*
				** minutes irrelevant; round to hours
				*/
				value = (value + 30) / 60;

				sprintf(strvalue, "%2lldd%02lldh",
						value/24, value%24);
			}
		}
	}

	return strvalue;
}

/*
** Function val2Hzstr() converts a value (in MHz) 
** to an ascii-string.
** The result-string is placed in the area pointed to strvalue,
** which should be able to contain at least 8 positions.
*/
char *
val2Hzstr(count_t value, char *strvalue)
{
        if (value < 1000)
        {
                sprintf(strvalue, "%4lldMHz", value);
        }
        else
        {
                double fval=value/1000.0;      // fval is double in GHz
                char prefix='G';

                if (fval >= 1000.0)            // prepare for the future
                {
                        prefix='T';        
                        fval /= 1000.0;
                }
                sprintf(strvalue, "%4.2f%cHz", fval, prefix);
        }
	return strvalue;
}


/*
** Function val2memstr() converts a value (number of bytes)
** to an ascii-string in a specific format (indicated by pformat).
** The result-string is placed in the area pointed to strvalue,
** which should be able to contain at least 7 positions.
*/
#define	ONEKBYTE	1024
#define	ONEMBYTE	1048576
#define	ONEGBYTE	1073741824L
#define	ONETBYTE	1099511627776LL

#define	MAXBYTE		1024
#define	MAXKBYTE	ONEKBYTE*99999L
#define	MAXMBYTE	ONEMBYTE*999L
#define	MAXGBYTE	ONEGBYTE*999LL

char *
val2memstr(count_t value, char *strvalue, int pformat, int avgval, int nsecs)
{
	char 	aformat;	/* advised format		*/
	count_t	verifyval;
	char	*suffix = "";
	int	basewidth = 6;


	/*
	** notice that the value can be negative, in which case the
	** modulo-value should be evaluated and an extra position should
	** be reserved for the sign
	*/
	if (value < 0)
		verifyval = -value * 10;
	else
		verifyval =  value;

	/*
	** verify if printed value is required per second (average) or total
	*/
	if (avgval)
	{
		value     /= nsecs;
		verifyval *= 100;
		basewidth -= 2;
		suffix     = "/s";
	}
	
	/*
	** determine which format will be used on bases of the value itself
	*/
	if (verifyval <= MAXBYTE)	/* bytes ? */
		aformat = ANYFORMAT;
	else
		if (verifyval <= MAXKBYTE)	/* kbytes ? */
			aformat = KBFORMAT;
		else
			if (verifyval <= MAXMBYTE)	/* mbytes ? */
				aformat = MBFORMAT;
			else
				if (verifyval <= MAXGBYTE)	/* mbytes ? */
					aformat = GBFORMAT;
				else
					aformat = TBFORMAT;

	/*
	** check if this is also the preferred format
	*/
	if (aformat <= pformat)
		aformat = pformat;

	switch (aformat)
	{
	   case	ANYFORMAT:
		sprintf(strvalue, "%*lld%s",
				basewidth, value, suffix);
		break;

	   case	KBFORMAT:
		sprintf(strvalue, "%*lldK%s",
				basewidth-1, value/ONEKBYTE, suffix);
		break;

	   case	MBFORMAT:
		sprintf(strvalue, "%*.1lfM%s",
			basewidth-1, (double)((double)value/ONEMBYTE), suffix); 
		break;

	   case	GBFORMAT:
		sprintf(strvalue, "%*.1lfG%s",
			basewidth-1, (double)((double)value/ONEGBYTE), suffix);
		break;

	   case	TBFORMAT:
		sprintf(strvalue, "%*.1lfT%s",
			basewidth-1, (double)((double)value/ONETBYTE), suffix);
		break;

	   default:
		sprintf(strvalue, "!TILT!");
	}

	return strvalue;
}


/*
** Function numeric() checks if the ascii-string contains 
** a numeric (positive) value.
** Returns 1 (true) if so, or 0 (false).
*/
int
numeric(char *ns)
{
	register char *s = ns;

	while (*s)
		if (*s < '0' || *s > '9')
			return(0);		/* false */
		else
			s++;
	return(1);				/* true  */
}

/*
** generic pointer verification after malloc
*/
void
ptrverify(const void *ptr, const char *errormsg, ...)
{
	va_list args;

        va_start(args, errormsg);

	if (!ptr)
	{
		acctswoff();
		netatop_signoff();

		if (vis.show_end)
			(vis.show_end)();

        	va_list args;
		fprintf(stderr, errormsg, args);
        	va_end  (args);

		exit(13);
	}
}

/*
** signal catcher for cleanup before exit
*/
void
cleanstop(exitcode)
{
	acctswoff();
	netatop_signoff();
	(vis.show_end)();
	exit(exitcode);
}

/*
** establish an async timer alarm with microsecond-level precision
*/
void
setalarm(struct timeval *interval)
{
	struct itimerval val;

	val.it_value = *interval;
	val.it_interval.tv_sec = val.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &val, NULL);
}

void
setalarm2(int sec, int usec)
{
	struct timeval interval;

	interval.tv_sec = sec;
	interval.tv_usec = usec;
	setalarm(&interval);
}

static void
setup_step_mode(void)
{
	const int SECONDS_IN_24_DAYS = 2073600;

	if (rawreadflag)
		fetchmode = PM_MODE_INTERP;
	else
		fetchmode = PM_MODE_LIVE;

	if (interval.tv_sec > SECONDS_IN_24_DAYS)
	{
		fetchstep = interval.tv_sec;
		fetchmode |= PM_XTB_SET(PM_TIME_SEC);
	}
	else
	{
		fetchstep = interval.tv_sec * 1e3 + interval.tv_usec / 1e3;
		fetchmode |= PM_XTB_SET(PM_TIME_MSEC);
	}
}

/*
 * Set the origin position and interval for PMAPI context fetching
 */
static int
setup_origin(pmOptions *opts)
{
	int		sts = 0;

	curtime = origin = opts->origin;

	/* initial archive mode, position and delta */
	if (opts->context == PM_CONTEXT_ARCHIVE)
	{
		if (opts->interval.tv_sec || opts->interval.tv_usec)
			interval = opts->interval;

		setup_step_mode();

		if ((sts = pmSetMode(fetchmode, &curtime, fetchstep)) < 0)
		{
			pmprintf(
		"%s: pmSetMode failure: %s\n", pmProgname, pmErrStr(sts));
			opts->flags |= PM_OPTFLAG_RUNTIME_ERR;
			opts->errors++;
		}
	}

	return sts;
}

/*
 * PMAPI context creation and initial command line option handling.
 */
static int
setup_context(pmOptions *opts)
{
	char		*source;
	int		sts, ctx;

	if (opts->context == PM_CONTEXT_ARCHIVE)
		source = opts->archives[0];
	else if (opts->context == PM_CONTEXT_HOST)
		source = opts->hosts[0];
	else if (opts->context == PM_CONTEXT_LOCAL)
		source = NULL;
	else
	{
		opts->context = PM_CONTEXT_HOST;
		source = "local:";
	}

	if ((sts = ctx = pmNewContext(opts->context, source)) < 0)
	{
		if (opts->context == PM_CONTEXT_HOST)
			pmprintf(
		"%s: Cannot connect to pmcd on host \"%s\": %s\n",
				pmProgname, source, pmErrStr(sts));
		else if (opts->context == PM_CONTEXT_LOCAL)
			pmprintf(
		"%s: Cannot make standalone connection on localhost: %s\n",
				pmProgname, pmErrStr(sts));
		else
			pmprintf(
		"%s: Cannot open archive \"%s\": %s\n",
				pmProgname, source, pmErrStr(sts));
	}
	else if ((sts = pmGetContextOptions(ctx, opts)) == 0)
		sts = setup_origin(opts);

	if (sts < 0)
	{
		pmflush();
		cleanstop(1);
	}

	return ctx;
}

void
setup_globals(pmOptions *opts)
{
	pmID		pmids[HOST_NMETRICS];
	pmDesc		descs[HOST_NMETRICS];
	pmResult	*result;

	setup_context(opts);
	setup_metrics(hostmetrics, &pmids[0], &descs[0], HOST_NMETRICS);
	fetch_metrics("host", HOST_NMETRICS, pmids, &result);

	if (HOST_NMETRICS != result->numpmid)
	{
		fprintf(stderr,
			"%s: pmFetch failed to fetch initial metric value(s)\n",
			pmProgname);
		cleanstop(1);
	}

	hertz = extract_integer(result, descs, HOST_HERTZ);
	pagesize = extract_integer(result, descs, HOST_PAGESIZE);
	extract_string(result, descs, HOST_RELEASE, sysname.release, sizeof(sysname.release));
	extract_string(result, descs, HOST_VERSION, sysname.version, sizeof(sysname.version));
	extract_string(result, descs, HOST_MACHINE, sysname.machine, sizeof(sysname.machine));
	extract_string(result, descs, HOST_NODENAME, sysname.nodename, sizeof(sysname.nodename));
	nodenamelen = strlen(sysname.nodename);

	pmFreeResult(result);
}

/*
** extract values from a pmResult structure using given offset(s)
** "value" is always a macro identifier from a metric map file.
*/
int
extract_integer_index(pmResult *result, pmDesc *descs, int value, int i)
{
	pmAtomValue atom = { 0 };
	pmValueSet *values = result->vset[value];

	if (values->numval <= 0 || values->numval <= i)
		return -1;
	pmExtractValue(values->valfmt, &values->vlist[i],
			descs[value].type, &atom, PM_TYPE_32);
	return atom.l;
}

int
extract_integer(pmResult *result, pmDesc *descs, int value)
{
	return extract_integer_index(result, descs, value, 0);
}

int
extract_integer_inst(pmResult *result, pmDesc *descs, int value, int inst)
{
	pmAtomValue atom = { 0 };
	pmValueSet *values = result->vset[value];
	int i;

	for (i = 0; i < values->numval; i++)
	{
		if (values->vlist[i].inst != inst)
			continue;
		pmExtractValue(values->valfmt, &values->vlist[i],
			descs[value].type, &atom, PM_TYPE_32);
		break;
	}
	if (values->numval <= 0 || i == values->numval)
		return -1;
	return atom.l;
}

count_t
extract_count_t(pmResult *result, pmDesc *descs, int value)
{
	return extract_count_t_index(result, descs, value, 0);
}

count_t
extract_count_t_index(pmResult *result, pmDesc *descs, int value, int i)
{
	pmAtomValue atom = { 0 };
	pmValueSet *values = result->vset[value];

	if (values->numval <= 0 || values->numval <= i)
		return -1;

	pmExtractValue(values->valfmt, &values->vlist[i],
			descs[value].type, &atom, PM_TYPE_64);
	return atom.ll;
}

count_t
extract_count_t_inst(pmResult *result, pmDesc *descs, int value, int inst)
{
	pmAtomValue atom = { 0 };
	pmValueSet *values = result->vset[value];
	int i;

	for (i = 0; i < values->numval; i++)
	{
		if (values->vlist[i].inst != inst)
			continue;
		pmExtractValue(values->valfmt, &values->vlist[i],
			descs[value].type, &atom, PM_TYPE_64);
		break;
	}
	if (values->numval <= 0 || i == values->numval)
		return -1;
	return atom.ll;
}

char *
extract_string_index(pmResult *result, pmDesc *descs, int value, char *buffer, int buflen, int i)
{
	pmAtomValue atom = { 0 };
	pmValueSet *values = result->vset[value];

	if (values->numval <= 0 || values->numval <= i)
		return NULL;

	pmExtractValue(values->valfmt, &values->vlist[i],
			descs[value].type, &atom, PM_TYPE_STRING);
	strncpy(buffer, atom.cp, buflen);
	free(atom.cp);
	if (buflen > 1)	/* might be a single character - e.g. process state */
	    buffer[buflen-1] = '\0';
	return buffer;
}

char *
extract_string(pmResult *result, pmDesc *descs, int value, char *buffer, int buflen)
{
	return extract_string_index(result, descs, value, buffer, buflen, 0);
}

char *
extract_string_inst(pmResult *result, pmDesc *descs, int value, char *buffer, int buflen, int inst)
{
	pmAtomValue atom = { 0 };
	pmValueSet *values = result->vset[value];
	int i;

	for (i = 0; i < values->numval; i++)
	{
		if (values->vlist[i].inst != inst)
			continue;
		pmExtractValue(values->valfmt, &values->vlist[i],
			descs[value].type, &atom, PM_TYPE_STRING);
		break;
	}
	if (values->numval <= 0 || values->numval == i)
		return NULL;
	strncpy(buffer, atom.cp, buflen);
	free(atom.cp);
	if (buflen > 1)	/* might be a single character - e.g. process state */
	    buffer[buflen-1] = '\0';
	return buffer;
}

float
extract_float_inst(pmResult *result, pmDesc *descs, int value, int inst)
{
	pmAtomValue atom = { 0 };
	pmValueSet *values = result->vset[value];
	int i;

	for (i = 0; i < values->numval; i++)
	{
		if (values->vlist[i].inst != inst)
			continue;
		pmExtractValue(values->valfmt, &values->vlist[i],
			descs[value].type, &atom, PM_TYPE_FLOAT);
		break;
	}
	if (values->numval <= 0 || i == values->numval)
		return -1;
	return atom.f;
}


void
setup_metrics(char **metrics, pmID *pmidlist, pmDesc *desclist, int nmetrics)
{
	int	i, sts;

	if ((sts = pmLookupName(nmetrics, metrics, pmidlist)) < 0)
	{
		fprintf(stderr, "%s: pmLookupName: %s\n",
			pmProgname, pmErrStr(sts));
		cleanstop(1);
	}
	if (nmetrics != sts)
	{
		for (i=0; i < nmetrics; i++)
		{
			if (pmidlist[i] != PM_ID_NULL)
				continue;
			if (pmDebug & DBG_TRACE_APPL0)
				fprintf(stderr,
					"%s: pmLookupName failed for %s\n",
					pmProgname, metrics[i]);
		}
	}

	for (i=0; i < nmetrics; i++)
	{
		if (pmidlist[i] == PM_ID_NULL)
		{
			desclist[i].pmid = PM_ID_NULL;
			continue;
		}
		if ((sts = pmLookupDesc(pmidlist[i], &desclist[i])) < 0)
		{
			if (pmDebug & DBG_TRACE_APPL0)
				fprintf(stderr,
					"%s: pmLookupDesc failed for %s: %s\n",
					pmProgname, metrics[i], pmErrStr(sts));
			pmidlist[i] = desclist[i].pmid = PM_ID_NULL;
		}
	}
}

int
fetch_metrics(const char *purpose, int nmetrics, pmID *pmids, pmResult **result)
{
	int	sts;

	pmSetMode(fetchmode, &curtime, fetchstep);
	if ((sts = pmFetch(nmetrics, pmids, result)) < 0)
	{
		if (sts != PM_ERR_EOL)
			fprintf(stderr, "%s: %s pmFetch: %s\n",
				pmProgname, purpose, pmErrStr(sts));
		cleanstop(1);
	}
	if (pmDebug & DBG_TRACE_APPL1)
	{
		pmResult	*rp = *result;
		struct tm	tmp;
		time_t		sec;

		sec = (time_t)rp->timestamp.tv_sec;
		pmLocaltime(&sec, &tmp);

		fprintf(stderr, "%s: got %d %s metrics @%02d:%02d:%02d.%03d\n",
				pmProgname, rp->numpmid, purpose,
				tmp.tm_hour, tmp.tm_min, tmp.tm_sec,
				(int)(rp->timestamp.tv_usec / 1000));
	}
	return sts;
}

int
get_instances(const char *purpose, int value, pmDesc *descs, int **pids, char ***insts)
{
	int	sts;

	if (descs[value].pmid == PM_ID_NULL)
	{
		/* we have no descriptor for this metric, thus no instances */
		*insts = NULL;
		*pids = NULL;
		return 0;
	}

	if (!rawreadflag)
	{
		if ((sts = pmGetInDom(descs[value].indom, pids, insts)) < 0)
		{
			fprintf(stderr, "%s: pmGetInDom %s indom: %s\n",
				pmProgname, purpose, pmErrStr(sts));
			cleanstop(1);
		}
		return sts;
	}

	if ((sts = pmGetInDomArchive(descs[value].indom, pids, insts)) < 0)
	{
		fprintf(stderr, "%s: pmGetInDomArchive %s indom: %s\n",
			pmProgname, purpose, pmErrStr(sts));
		cleanstop(1);
	}
	return sts;
}

void
rawwrite(pmOptions *opts)
{
	/* NYI - see pmRecordSetup(3) */
}
