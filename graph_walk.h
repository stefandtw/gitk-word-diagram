#ifndef GRAPH_WALK_H
#define GRAPH_WALK_H

#include "graph.h"

void graph_walk(GraphRoot* root, void (*file_func)(GraphFile*), void (*hunk_func)(GraphHunk*), void (*word_func)(GraphWord*), void (*edge_func)(GraphEdge*,GraphRoot*));

void graph_type_walk(Graph* g, GraphRoot* root, void (*func)(Graph*, GraphRoot*, void*), void* arg);


#endif
