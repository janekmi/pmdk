// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2014-2023, Intel Corporation */

/*
 * manpage.c -- simple example for the libpmemobj man page
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <libpmemobj.h>
#include <assert.h>
#include <signal.h>
#include <sys/wait.h>
#include <inttypes.h>

/* size of the pmemobj pool -- 3 GB */
#define POOL_SIZE ((size_t)(1 << 30) * 3)

/* name of our layout in the pool */
#define LAYOUT_NAME "example_layout"

static size_t
get_curr_allocated(PMEMobjpool *pop)
{
	size_t allocated;
	int ret = pmemobj_ctl_get(pop, "stats.heap.curr_allocated", &allocated);
	assert(ret == 0);
	return allocated;
}

#define ALLOC_SIZE 100
#define ALLOC_NUM 1000
#define ALLOC_FREE 200

enum STATE {
	STATE_INVALID = 0xbadf00d,
	STATE_OPENED,
	STATE_ALLOCATED
};

const char path[] = "/mnt/pmem0/myfile";

#define INVALID_CLASS_ID ((unsigned)-1)

static unsigned
register_alloc_class(PMEMobjpool *pop)
{
	int ret;

	struct pobj_alloc_class_desc class = {
		.unit_size = 300,
		.alignment = 300,
		.units_per_block = 1000,
		.header_type = POBJ_HEADER_NONE,
		.class_id = INVALID_CLASS_ID
	};

	ret = pmemobj_ctl_set(pop, "heap.alloc_class.new.desc", &class);
	assert(ret == 0);
	assert(class.class_id != INVALID_CLASS_ID);

	return class.class_id;
}

static void
alloc_and_free(int seed, int writefs)
{
	PMEMobjpool *pop;
	PMEMoid oids[ALLOC_NUM];
	PMEMoid oid;
	enum STATE state;
	int i;
	int idx;
	int freed;
	int ret = 0;
	unsigned class_id;

	srand(seed);

	/* create the pmemobj pool or open it if it already exists */
	pop = pmemobj_create(path, LAYOUT_NAME, POOL_SIZE, 0666);

	if (pop == NULL)
	    pop = pmemobj_open(path, LAYOUT_NAME);

	if (pop == NULL) {
		perror(path);
		exit(1);
	}

	uint64_t run_allocated, run_active;
	ret = pmemobj_ctl_get(pop, "stats.heap.run_allocated", &run_allocated);
	assert(ret == 0);
	ret = pmemobj_ctl_get(pop, "stats.heap.run_active", &run_active );
	assert(ret == 0);
	printf("stats.heap.run_active/run_allocated: %" PRIu64 "/%" PRIu64 "\n", run_active, run_allocated);

	/* It has to be enabled before any persistent operation happen. */
	/* stats.heap.curr_allocated is a persistent statistic. */
	int enabled = POBJ_STATS_ENABLED_PERSISTENT;
	ret = pmemobj_ctl_set(pop, "stats.enabled", &enabled);
	assert(ret == 0);

	// assert(get_curr_allocated(pop) == 0);
	printf("stats.heap.curr_allocated: %zu\n", get_curr_allocated(pop));

	class_id = register_alloc_class(pop);

	printf("opened\n");
	state = STATE_OPENED;
	write(writefs, &state, sizeof(enum STATE));

	pmemobj_tx_begin(pop, NULL, TX_PARAM_NONE);
	for (i = 0; i < ALLOC_NUM; ++i) {
		oids[i] = pmemobj_tx_xalloc(ALLOC_SIZE, 0, POBJ_CLASS_ID(class_id));
		assert(!OID_IS_NULL(oids[i]));
	}

	// printf("allocated\n");
	// state = STATE_ALLOCATED;
	// write(writefs, &state, sizeof(enum STATE));

	for (i = 0; i < ALLOC_FREE; ++i) {
		idx = rand() % ALLOC_NUM;
		oid = oids[idx];
		if (OID_IS_NULL(oid)) {
			--i;
			continue;
		}
		ret = pmemobj_tx_free(oid);
		assert(ret == 0);
		oids[idx] = OID_NULL;
	}
	freed = 0;
	for (i = 0; i < ALLOC_NUM; ++i) {
		if (OID_IS_NULL(oids[i])) {
			++freed;
		}
	}
	assert(freed == ALLOC_FREE);
	/*
	 * The transaction has to be aborted to make sure the allocations are
	 * reverted from the pool no matter the transaction is interrupted from
	 * the outside on-time or not.
	 */
	pmemobj_tx_abort(-1);

	// assert(get_curr_allocated(pop) == 0);
	printf("stats.heap.curr_allocated: %zu\n", get_curr_allocated(pop));

	pmemobj_close(pop);
	printf("closed normally\n");
}

static int
sequence(int step)
{
	int pfds[2];
	enum STATE state = STATE_INVALID;
	int seed;
	int wstatus;
	int ret;

	ret = pipe(pfds);
	assert(ret == 0);

	seed = rand();

	pid_t ch_pid = fork();
	if (ch_pid < 0) {
		perror("fork");
		return 1;
	} else if (ch_pid > 0) {
		printf("child PID: %d\n", ch_pid);

		read(pfds[0], &state, sizeof(state));
		assert(state == STATE_OPENED);

		ret = kill(ch_pid, SIGKILL);
		assert(ret == 0);
		printf("killed\n");

		if (waitpid(ch_pid, &wstatus, WUNTRACED | WCONTINUED) == -1) {
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
	} else {
		printf("step: %d\n", step);
		alloc_and_free(seed, pfds[1]);
	}

	close(pfds[0]);
	close(pfds[1]);

	return 0;
}

int
main(int argc, char *argv[])
{
	time_t t;
	int ret;
	int i;

	srand((unsigned) time(&t));

	unlink(path);

	for (i = 0; i < 100; ++i) {
		ret = sequence(i);
	}

	return ret;
}
