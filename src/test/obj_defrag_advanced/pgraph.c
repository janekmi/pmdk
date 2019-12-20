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

#define MAX_GRAPH_COPIES 10
#define PATTERN 'g'

static size_t
pnode_size(unsigned edges_num, size_t pattern_size)
{
	size_t node_size = sizeof(struct pnode);
	node_size += sizeof(PMEMoid) * edges_num;
	node_size += pattern_size;
	return node_size;
}

static void
pnode_init(PMEMobjpool *pop, PMEMoid pnode_oid, struct vnode *vnode, PMEMoid pnodes[])
{
	struct pnode *pnode = pmemobj_direct(pnode_oid);
	pnode->node_id = vnode->node_id;
	pnode->size = vnode->psize;

	/* set edges */
	pnode->edges_num = vnode->edges_num;
	for (unsigned i = 0; i < vnode->edges_num; ++i)
		pnode->edges[i] = pnodes[vnode->edges[i]];

	/* initialize pattern */
	pnode->pattern_size = vnode->pattern_size;
	void *pattern = (void *)pnode->edges[pnode->edges_num];
	pmemobj_memset(pop, pattern, PATTERN, pnode->pattern_size, PMEMOBJ_F_MEM_NOFLUSH);

	/* persist the whole node state */
	pmemobj_persist(pop, (const void *)pnode, pnode->size);
}

/*
 * order_shuffle -- XXX
 */
static void
order_shuffle(unsigned *order, unsigned num)
{
	for (unsigned i = 0; i < num; ++i) {
		unsigned j = rand() % num;
		unsigned temp = order[j];
		order[j] = order[i];
		order[i] = temp;
	}
}

/*
 * order_new -- XXX
 */
static unsigned *
order_new(struct vgraph *vgraph)
{
	unsigned *order = malloc(sizeof(unsigned) * vgraph->nodes_num);

	/* initialize id list */
	for (unsigned i = 0; i < vgraph->nodes_num; ++i)
		order[i] = i;

	order_shuffle(order, vgraph->nodes_num);

	return order;
}

static PMEMoid *
pgraph_copy_new(PMEMobjpool *pop, struct vgraph *vgraph)
{
	PMEMoid *nodes = malloc(sizeof(PMEMoid) * vgraph->nodes_num);
	unsigned *order = order_new(vgraph);

	int ret;
	for (unsigned i = 0; i < vgraph->nodes_num; ++i) {
		struct vnode *vnode = vgraph->node[order[i]];
		PMEMoid *node = *nodes[order[i]];
		ret = pmemobj_alloc(pop, nodes, vnode->psize, 0, NULL, NULL);
		UT_ASSERTeq(ret, 0);
	}

	free(order);

	return nodes;
}

/*
 * pgraph_copy_delete -- XXX
 */
static void
pgraph_copy_delete(PMEMoid *nodes, unsigned num)
{
	for (unsigned i = 0; i < num; ++i) {
		if (nodes[i] == OID_NULL)
			continue;

		pmemobj_free(&nodes[i]);
	}

	free(nodes);
}

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

	/* calculate size of pnodes */
	for (unsigned i = 0; i < vgraph->node; ++i) {
		struct vnode *vnode = &vgraph->node[i];
		vnode->psize = pnode_size(vnode->edges, vnode->pattern_size);
	}

	/* prepare multiple copies of the nodes */
	unsigned copies_num = rand() % MAX_GRAPH_COPIES + 1; /* XXX */
	PMEMoid **copies = malloc(sizeof(PMEMoid *) * copies_num);
	for (unsigned i = 0; i < copies_num; ++i)
		copies[i] = pgraph_copy_new(pop, vgraph);

	/* peek exactly the one copy of each node */
	for (unsigned i = 0; i < pgraph->nodes_num; ++i) {
		unsigned copy_id = rand() % copies_num; /* XXX */
		pgraph->nodes[i] = copies[copy_id][i];
		copies[copy_id][i] = OID_NULL;
	}

	/* free unused copies of the nodes */
	for (unsigned i = 0; i < copies_num; ++i)
		pgraph_copy_delete(copies[i], vgraph->nodes_num);

	free(copies);

	/* initialize pnodes */
	for (unsigned i = 0; i < pgraph->nodes; ++i)
		pnode_init(pop, pgraph->nodes[i], &vgraph->node[i], pgraph->nodes);
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
