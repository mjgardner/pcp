#include <pcp/pmapi.h>

int
main(int argc, char *argv[])
{
    int			sts;
    char		*err;
    struct timeval	time;

    while (argc > 1) {
	printf("\"%s\"", argv[1]);
	sts = pmParseInterval(argv[1], &time, &err);
	if (sts == -1) {
	    printf(" Error:\n%s", err);
	    free(err);
	}
	else if (sts == 0) {
	    printf(" Time: %lld.%06ld sec\n",
			(long long)time.tv_sec, (long)time.tv_usec);
	}
	else {
	    printf(" Bogus return value: %d\n", sts);
	}
	argc--;
	argv++;
    }

    return 0;
}
