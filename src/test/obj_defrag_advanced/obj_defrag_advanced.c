/*
 * Copyright 2019, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * obj_defrag_advanced.c -- test for defragmentation feature
 */

#include "unittest.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vgraph.h"
#include "pgraph.h"

int
main(int argc, char *argv[])
{
	START(argc, argv, "obj_defrag_advanced");

	const char *path = argv[1];
	PMEMobjpool *pop = NULL;

	pop = pmemobj_create(path, POBJ_LAYOUT_NAME(basic),
		0, S_IWUSR | S_IRUSR);
	if (pop == NULL) {
		UT_FATAL("!pmemobj_create: %s", path);
	}

	srand((unsigned)time(NULL));

	struct vgraph *vgraph = vgraph_new();
	struct pgraph *pgraph = pgraph_new(pop, vgraph);
	vgraph_delete(vgraph);
	pgraph_print(pgraph);

	/*
	 * mix
	 * add edge
	 * dump
	 * defrag
	 * dump
	 */

	pmemobj_close(pop);

	DONE(NULL);
}
