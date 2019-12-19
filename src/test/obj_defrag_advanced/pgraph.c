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
 * pgraph.c -- persistent graph representation
 */

#include "vgraph.h"
#include "pgraph.h"

/*
 * pgraph_new -- XXX
 */
struct pgraph *
pgraph_new(PMEMobjpool *pop, struct vgraph *vgraph)
{
	size_t root_size = sizeof(struct pgraph) + sizeof(PMEMoid) * vgraph->nodes_num;
	PMEMoid root_oid = pmemobj_root(pop, root_size);
	struct pgraph *pgraph = pmemobj_direct(root_oid);
	pgraph->nodes_num = vgraph->nodes_num;
	int ret;

	for (unsigned i = 0; i < pgraph->nodes_num; ++i) {
		 ret = pmemobj_alloc(pop, &pgraph->nodes[i], vgraph->node[i].size, 0, NULL, NULL);
		 UT_ASSERTeq(ret, 0);
	}

	return pgraph;
}

/*
 * pgraph_delete -- XXX
 */
void
pgraph_delete(PMEMobjpool *pop, struct pgraph *pgraph)
{
	for (unsigned i = 0; pgraph->nodes_num; ++i) {
		pmemobj_free(&pgraph->nodes[i]);
	}
}

/*
 * pgraph_print -- XXX
 */
void
pgraph_print(struct pgraph *graph)
{

}
