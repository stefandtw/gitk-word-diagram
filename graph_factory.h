#ifndef GRAPH_FACTORY_H
#define GRAPH_FACTORY_H

#include "ast.h"
#include "graph.h"

GraphRoot* new_graph(Ast* ast, char* filefilter, char* hunkfilter, char* wordfilter);

#endif
