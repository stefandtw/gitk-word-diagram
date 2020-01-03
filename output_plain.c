#include <stdio.h>
#include "output_plain.h"
#include "graph.h"
#include "graph_walk.h"

static void print_file(GraphFile* file);
static void print_hunk(GraphHunk* hunk);
static void print_word(GraphWord* word);
static void print_edge(GraphEdge* edge, GraphRoot* root);

void output_plain(GraphRoot* g) {
	graph_walk(g, &print_file, &print_hunk, &print_word, &print_edge);
}

static void print_file(GraphFile* file) {
	printf("File %s\n", file->name);
}

static void print_hunk(GraphHunk* hunk) {
	printf("Hunk %s\n", hunk->name);
}

static void print_word(GraphWord* word) {
	printf("Word %i/%i/%i %s\n", word->num_added, word->num_removed, word->num_unchanged, word->str);
}

static void print_edge(GraphEdge* edge, GraphRoot* root) {
	printf("edge %i/%i/%i %s -> %s \n", edge->num_added, edge->num_removed, edge->num_unchanged, edge->src->name, edge->dest->name);
}
