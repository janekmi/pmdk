// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2017-2019, Intel Corporation */

/*
 * obj_action.c -- test the action API
 */

#include <stdlib.h>
#include "unittest.h"

#define LAYOUT_NAME "obj_action"

struct root {
	PMEMoid oid;
};

#define MAX_ACTS 100
#define SLOTS_NUM 50
#define SLOTS_SIZE (sizeof(uint64_t) * SLOTS_NUM)

int
main(int argc, char *argv[])
{
	START(argc, argv, "obj_action");

	if (argc < 2)
		UT_FATAL("usage: %s filename", argv[0]);

	const char *path = argv[1];

	PMEMobjpool *pop = pmemobj_create(path, LAYOUT_NAME, PMEMOBJ_MIN_POOL,
				S_IWUSR | S_IRUSR);
	if (pop == NULL)
		UT_FATAL("!pmemobj_create: %s", path);

	PMEMoid root = pmemobj_root(pop, sizeof(struct root));
	struct root *rootp = (struct root *)pmemobj_direct(root);

	struct pobj_action actions[MAX_ACTS];
	unsigned actnum = 0;

	rootp->oid = pmemobj_reserve(pop, &actions[actnum++], SLOTS_SIZE, 0);
	uint64_t *slots = (uint64_t *)pmemobj_direct(rootp->oid);
	for (unsigned i = 0; i < SLOTS_NUM; ++i) {
		pmemobj_set_value(pop, &actions[actnum++], &slots[i], 0);
	}
	pmemobj_publish(pop, actions, actnum);

	actnum = 0; /* reuse the action array */
	for (unsigned i = 0; i < SLOTS_NUM; ++i) {
		pmemobj_set_value(pop, &actions[actnum++], &slots[i], 1);
	}
	pmemobj_publish(pop, actions, actnum);

	pmemobj_persist(pop, rootp, sizeof(*rootp));

	pmemobj_close(pop);

	DONE(NULL);
}
