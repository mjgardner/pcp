#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>

#include "zfs_pools.h"

void
zfs_pools_init(zfs_poolstats_t *poolstats, pmdaInstid *pools, pmdaIndom *poolsindom)
{
        DIR *zfs_dp;
        struct dirent *ep;
        int pool_num = 0;
        size_t size;

        // Discover the pools by looking for directories in /proc/spl/kstat/zfs
        if ((zfs_dp = opendir(ZFS_PROC_DIR)) != NULL) {
                while ((ep = readdir(zfs_dp))) {
                        if (ep->d_type == DT_DIR) {
                                if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
                                        continue;
                                else {
                                        size = (pool_num + 1) * sizeof(pmdaInstid);
                                        if ((pools = (pmdaInstid *) realloc(pools, size)) == NULL)
                                                pmNoMem("process", size, PM_FATAL_ERR);
                                        pools[pool_num].i_name = (char *) malloc(strlen(ep->d_name) + 1);
                                        strcpy(pools[pool_num].i_name, ep->d_name);
                                        pools[pool_num].i_name[strlen(ep->d_name)] = '\0';
                                        pools[pool_num].i_inst = pool_num;
                                        pool_num++;
                                }
                        }
                }
                closedir(zfs_dp);
        }
        if (pools == NULL)
                pmNotifyErr(LOG_WARNING, "no ZFS pools found, instance domain is empty.");
	poolsindom->it_indom = ZFS_POOL_INDOM;
        poolsindom->it_set = pools;
        poolsindom->it_numinst = pool_num;
        poolstats = (zfs_poolstats_t *) malloc(pool_num * sizeof(zfs_poolstats_t));
}

void
zfs_pools_clear(zfs_poolstats_t *poolstats, pmdaInstid *pools, pmdaIndom *poolsindom)
{
        int i;

        for (i = 0; i < poolsindom->it_numinst; i++) {
                free(pools[i].i_name);
                pools[i].i_name = NULL;
        }
        if (pools)
                free(pools);
        if (poolstats)
                free(poolstats);
        poolstats = NULL;
	poolsindom->it_indom = ZFS_POOL_INDOM;
        poolsindom->it_set = pools = NULL;
        poolsindom->it_numinst = 0;
}

void
zfs_poolstats_refresh(zfs_poolstats_t *poolstats, pmdaInstid *pools, pmdaIndom *poolsindom)
{
        int i;
        char *line, pool_dir[PATH_MAX], fname[PATH_MAX];
        FILE *fp;
        struct stat sstat;
        regex_t rgx_io;
        size_t nmatch = 1, len;
        regmatch_t pmatch[1];
        
        regcomp(&rgx_io, "^([0-9]+ ){11}[0-9]+$", REG_EXTENDED);
        if ((poolstats = realloc(poolstats, poolsindom->it_numinst * sizeof(zfs_poolstats_t))) == NULL)
                pmNoMem("process", poolsindom->it_numinst * sizeof(zfs_poolstats_t), PM_FATAL_ERR);
        for (i = 0; i < poolsindom->it_numinst; i++) {
                strcpy(pool_dir, ZFS_PROC_DIR);
                strcat(pool_dir, poolsindom->it_set[i].i_name);
                if (stat(pool_dir, &sstat) != 0) {
                        // Pools setup changed, the instance domain must follow
                        regfree(&rgx_io);
                        zfs_pools_clear(poolstats, pools, poolsindom);
                        zfs_pools_init(poolstats, pools, poolsindom);
                        zfs_poolstats_refresh(poolstats, pools, poolsindom);
                        return;
                }
                // Read the state if exists
                /*poolstats[i].state = (char *) malloc(8*sizeof(char));
                strcpy(poolstats[i].state, "UNKNOWN");
                poolstats[i].state[8] = '\0';*/
                poolstats[i].state = "UNKNOWN";
                strcpy(fname, pool_dir);
                strcat(fname, "state");
                fp = fopen(fname, "r");
                if (fp != NULL) {
                        if (getline(&line, &len, fp) != -1) {
                                if (len > 0 && line[len-1] == '\n') line[--len] = '\0';
                                free(poolstats[i].state);
                                poolstats[i].state = (char *) malloc((len+1)*sizeof(char));
                                strcpy(poolstats[i].state, line);
                                poolstats[i].state[len] = '\0';
                        }
                        fclose(fp);
                }
                // Read the IO stats
                strcpy(fname, pool_dir);
                strcat(fname, "io");
                fp = fopen(fname, "r");
                if (fp != NULL) {
                        while (getline(&line, &len, fp) != -1) {
                                if (regexec(&rgx_io, line, nmatch, pmatch, 0) == 0)
                                        sscanf(line, "%" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
						&poolstats[i].nread,
	 					&poolstats[i].nwritten,
	 					&poolstats[i].reads,
	 					&poolstats[i].writes,
	 					&poolstats[i].wtime,
	 					&poolstats[i].wlentime,
	 					&poolstats[i].wupdate,
	 					&poolstats[i].rtime,
	 					&poolstats[i].rlentime,
	 					&poolstats[i].rupdate,
	 					&poolstats[i].wcnt,
	 					&poolstats[i].rcnt);
                        }
                        fclose(fp);
                }
        }
        regfree(&rgx_io);
}
