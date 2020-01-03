#include <stdlib.h>
#include <stdio.h>
#include "graph_walk.h"
#include "graph.h"

void graph_walk(GraphRoot* root, void (*file_func)(GraphFile*), void (*hunk_func)(GraphHunk*), void (*word_func)(GraphWord*), void (*edge_func)(GraphEdge*,GraphRoot*)) {
	graph_type_walk((Graph*) root->first_file, root, (void*) file_func, NULL);
	graph_type_walk((Graph*) root->first_hunk, root, (void*) hunk_func, NULL);
	graph_type_walk((Graph*) root->first_word, root, (void*) word_func, NULL);
	graph_type_walk((Graph*) root->first_edge, root, (void*) edge_func, NULL);
}

void graph_type_walk(Graph* g, GraphRoot* root, void (*func)(Graph*, GraphRoot*, void*), void* arg) {
	if (g == NULL) {
		return;
	}

	(*func)((Graph*) g, root, arg);

	graph_type_walk(g->next, root, func, arg);
}

