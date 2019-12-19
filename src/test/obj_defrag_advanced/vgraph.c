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
 * vgraph.c -- volatile graph representation
 */

#include "vgraph.h"

#define MAX_NODES 50
#define MAX_EDGES 10

/*
 * rand_nonzero -- XXX
 */
static unsigned
rand_nonzero(int max)
{
	int ret;
	do {
		ret = rand() % max;
	} while (ret == 0);

	return (unsigned)ret;
}

/*
 * vnode_new -- XXX
 */
static void
vnode_new(struct vnode *node, unsigned v)
{
	unsigned edges_num = rand_nonzero(MAX_EDGES);
	node->node_id = v;
	node->edges_num = edges_num;
	node->edges = (unsigned *)malloc(sizeof(int) * edges_num);
}

static void
vnode_delete(struct vnode *node)
{
	free(node->edges);
}

static struct vnode *
vgraph_get_node(struct vgraph *graph, unsigned id_node)
{
	struct node *node;

	node = &graph->node[id_node];
	return node;
}

static void
vgraph_add_edges(struct vgraph *graph)
{
	unsigned nodes_count = 0;
	unsigned edges_count = 0;
	struct vnode *node;
	for (nodes_count = 0; nodes_count < graph->nodes_num; nodes_count++) {
		node = vgraph_get_node(graph, nodes_count);
		unsigned edges_num = node->edges_num;
		for (edges_count = 0; edges_count < edges_num; edges_count++) {
			unsigned node_link =
					(unsigned)rand() % graph->nodes_num;
			node->edges[edges_count] = node_link;
		}
	}
}

struct vgraph *
vgraph_new()
{
	unsigned nodes_num = rand_nonzero(MAX_NODES);

	struct vgraph *graph =
			malloc(sizeof(struct vgraph) +
					sizeof(struct vnode) * nodes_num);
	graph->nodes_num = nodes_num;

	for (unsigned i = 0; i < nodes_num; i++) {
		vnode_new(&graph->node[i], i);
	}

	vgraph_add_edges(graph);

	return graph;
}

void
vgraph_delete(struct vgraph *graph)
{
	for (unsigned i = 0; i < graph->nodes_num; i++)
		vnode_delete(&graph->node[i], i);

	free(graph);
}

void
vgraph_print(struct vgraph *graph)
{
	struct vnode *node;
	for (unsigned i = 0; i < graph->nodes_num; i++) {
		node = get_node(graph, i);
		unsigned edges_num = node ->edges_num;
		printf("\nNode: %d\n", node->node_id);
		for (unsigned i = 0; i < edges_num; i++) {
			printf("%d, ", node->edges[i]);
		}
		printf("\n");
	}
}
