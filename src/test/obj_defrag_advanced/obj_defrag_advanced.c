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

#include<stdio.h>
#include<stdlib.h>
#include <time.h>

#define MAX_NODES 50
#define MAX_EDGES 10

struct node
{
	unsigned node_id;
	unsigned edges_num;
	unsigned *edges;
};

struct graph
{
	unsigned nodes_num;
	struct node node[];
};

static unsigned
rand_nonzero(int max)
{
	int ret;
	do {
		ret = rand() % max;
	} while (ret == 0);

	return (unsigned)ret;
}

static void
create_node(struct node *node, unsigned v)
{
	unsigned edges_num = rand_nonzero(MAX_EDGES);
	node->node_id = v;
	node->edges_num = edges_num;
	node->edges = (unsigned *)malloc(sizeof(int) * edges_num);
}

static struct graph *
create_graph(unsigned nodes_num)
{
	struct graph *graph =
			malloc(sizeof(struct graph) +
					sizeof(struct node) * nodes_num);
	graph->nodes_num = nodes_num;

	for (unsigned i = 0; i < nodes_num; i++) {
		create_node(&graph->node[i], i);
	}
	return graph;
}

static struct node *
get_node(struct graph *graph, unsigned id_node)
{
	struct node *node;

	node = &graph->node[id_node];
	return node;
}

static void
add_edge(struct graph *graph)
{
	unsigned nodes_count = 0;
	unsigned edges_count = 0;
	struct node *node;
	for (nodes_count = 0; nodes_count < graph->nodes_num; nodes_count++) {
		node = get_node(graph, nodes_count);
		unsigned edges_num = node->edges_num;
		for (edges_count = 0; edges_count < edges_num; edges_count++) {
			unsigned node_link =
					(unsigned)rand() % graph->nodes_num;
			node->edges[edges_count] = node_link;
		}
	}
}

static void
print_graph(struct graph *graph)
{
	struct node *node;
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

int
main()
{
	/* number of nodes in a graph */
	srand((unsigned)time(NULL));
	unsigned nodes_num = rand_nonzero(MAX_NODES);
	printf("nodes_num: %d \n", nodes_num);

	struct graph *graph = create_graph(nodes_num);
	add_edge(graph);
	print_graph(graph);

	/*
	 * mix
	 * add edge
	 * dump
	 * defrag
	 * dump
	 */
}
