#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

void graph_setnext(Graph* g, Graph* next, GraphRoot* root) {
	if (g == NULL && next == NULL) {
		return;
	}
	if (g == NULL) {
		if (next->type == GRAPH_FILE) {
			root->first_file = (GraphFile*) next;
		} else if (next->type == GRAPH_HUNK) {
			root->first_hunk = (GraphHunk*) next;
		} else if (next->type == GRAPH_WORD) {
			root->first_word = (GraphWord*) next;
		} else if (next->type == GRAPH_EDGE) {
			root->first_edge = (GraphEdge*) next;
		} else {
			fprintf(stderr, "graph_setnext: Missing implementation for %i\n", next->type);
			exit(1);
		}
	} else {
		g->next = next;
	}
	if (next != NULL) {
		next->prev = g;
	}
}
