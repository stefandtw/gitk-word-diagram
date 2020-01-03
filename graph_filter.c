#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "graph_filter.h"
#include "graph_walk.h"

static void remove_edges_with_node(Graph* node, GraphRoot* root);
static void remove_edge_with_node(Graph* cur, GraphRoot* root, void* node);
static void remove_unchanged_words(GraphRoot* root);
static void remove_unchanged_word(Graph* cur, GraphRoot* root, void* arg);
static void limit_words(GraphRoot* root);
static void limit_hunks(GraphRoot* root);
static void limit_files(GraphRoot* root);
static void limit(Graph* cur, GraphRoot* root, void* arg);
static int word_rating(Graph* word_g, GraphRoot* root);
static int hunk_rating(Graph* hunk_g, GraphRoot* root);
static int file_rating(Graph* file_g, GraphRoot* root);
static void remove_dangling_nodes(GraphRoot* root);
static void remove_dangling_node(Graph* cur, GraphRoot* root, void* arg);

void graph_filter(GraphRoot* root) {
	remove_unchanged_words(root);
	limit_files(root);
	limit_hunks(root);
	limit_words(root);
	remove_dangling_nodes(root);
}

static void remove_edges_with_node(Graph* node, GraphRoot* root) {
	graph_type_walk((Graph*) root->first_edge, root, &remove_edge_with_node, node);
}

static void remove_edge_with_node(Graph* cur, GraphRoot* root, void* node) {
	GraphEdge* e = (GraphEdge*) cur;
	if (e->src == node || e->dest == node) {
		graph_setnext(cur->prev, cur->next, root);
	}
}

static void remove_unchanged_words(GraphRoot* root) {
	graph_type_walk((Graph*) root->first_word, root, &remove_unchanged_word, NULL);
}

static void remove_unchanged_word(Graph* cur, GraphRoot* root, void* arg) {
	GraphWord* word = (GraphWord*) cur;
	if (word->num_added == 0 && word->num_removed == 0) {
		graph_setnext(cur->prev, cur->next, root);
		remove_edges_with_node(cur, root);
	}
}

int limit_to;
Graph** highest_value;
int highest_value_size;
int (*rating_func)(Graph*, GraphRoot*);

static void limit_words(GraphRoot* root) {
	limit_to = 5;
	rating_func = &word_rating;
	highest_value = malloc(sizeof(Graph*) * limit_to);
	highest_value_size = 0;
	graph_type_walk((Graph*) root->first_word, root, &limit, NULL);
	free(highest_value);
}

static void limit_hunks(GraphRoot* root) {
	limit_to = 8;
	rating_func = &hunk_rating;
	highest_value = malloc(sizeof(Graph*) * limit_to);
	highest_value_size = 0;
	graph_type_walk((Graph*) root->first_hunk, root, &limit, NULL);
	free(highest_value);
}

static void limit_files(GraphRoot* root) {
	limit_to = 8;
	rating_func = &file_rating;
	highest_value = malloc(sizeof(Graph*) * limit_to);
	highest_value_size = 0;
	graph_type_walk((Graph*) root->first_file, root, &limit, NULL);
	free(highest_value);
}

static void limit(Graph* cur, GraphRoot* root, void* arg) {
	// find lowest of highest_value
	int lowest_v = INT_MAX;
	int lowest_i = -1;
	for (int i=0; i<highest_value_size; i++) {
		if ((*rating_func)(highest_value[i], root) < lowest_v) {
			lowest_i = i;
			lowest_v = (*rating_func)(highest_value[i], root);
		}
	}
	// compare cur to lowest
	int cur_v = (*rating_func)(cur, root);
	if (highest_value_size < limit_to && cur_v > 0) {
		// add to highest_value
		highest_value[highest_value_size] = cur;
		highest_value_size++;
	} else if (cur_v > lowest_v) {
		// remove previously lowest from graph
		Graph* lowest = (Graph*) highest_value[lowest_i];
		// add to highest_value
		highest_value[lowest_i] = cur;

		graph_setnext(lowest->prev, lowest->next, root);
		remove_edges_with_node(lowest, root);
	} else {
		// remove node from graph
		graph_setnext(cur->prev, cur->next, root);
		remove_edges_with_node(cur, root);
	}
}

static int word_rating(Graph* word_g, GraphRoot* root) {
	GraphWord* word = (GraphWord*) word_g;
	return word->num_added + word->num_removed - 10*word->num_unchanged;
}

static int hunk_rating(Graph* hunk_g, GraphRoot* root) {
	GraphHunk* hunk = (GraphHunk*) hunk_g;
	GraphEdge* e = root->first_edge;
	int value = 0;
	while (e) {
		if (e->src == (Graph*) hunk) {
			int word_v = word_rating(e->dest, root);
			if (word_v > 0) {
				value += word_v;
			}
		}
		e = e->next;
	}
	return value;
}

static int file_rating(Graph* file_g, GraphRoot* root) {
	GraphFile* file = (GraphFile*) file_g;
	GraphEdge* e = root->first_edge;
	int value = 0;
	while (e) {
		if (e->src == (Graph*) file) {
			int hunk_v = hunk_rating(e->dest, root);
			if (hunk_v > 0) {
				value += hunk_v;
			}
		}
		e = e->next;
	}
	return value;
}

static void remove_dangling_nodes(GraphRoot* root) {
	graph_type_walk((Graph*) root->first_file, root, &remove_dangling_node, NULL);
	graph_type_walk((Graph*) root->first_hunk, root, &remove_dangling_node, NULL);
	graph_type_walk((Graph*) root->first_word, root, &remove_dangling_node, NULL);
}

static void remove_dangling_node(Graph* cur, GraphRoot* root, void* arg) {
	GraphEdge* e = root->first_edge;
	while (e) {
		if (e->src == cur || e->dest == cur) {
			return;
		}
		e = e->next;
	}
	graph_setnext(cur->prev, cur->next, root);
}
