#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "ast.h"

char* filefilter;
char* hunkfilter;
char* wordfilter;

GraphRoot* new_graph(Ast* ast, char* filefilter, char* hunkfilter, char* wordfilter);
static GraphRoot* walk(Ast* ast, Graph* parent, GraphRoot* root);
static GraphRoot* new_graph_root(void);
static GraphFile* new_graph_file(char* name);
static GraphHunk* new_graph_hunk(char* name);
static GraphWord* new_graph_word(char* str);
static GraphEdge* new_graph_edge(Graph* src, Graph* dest);
static void graph_word_occurence(GraphWord* word, GraphEdge* edge, GraphWordType wtype);
static Graph* find_node(Graph* graph, char* name);
static GraphEdge* find_edge(GraphEdge* graph, Graph* src, Graph* dest);

GraphRoot* new_graph(Ast* ast, char* _filefilter, char* _hunkfilter, char* _wordfilter) {
	filefilter = _filefilter;
	hunkfilter = _hunkfilter;
	wordfilter = _wordfilter;
	GraphRoot* root = new_graph_root();
	return walk(ast, NULL, root);
}

static GraphRoot* walk(Ast* ast, Graph* parent, GraphRoot* root) {
	if (ast == NULL) {
		return root;
	}
	static Graph* last_file = NULL;
	static Graph* last_hunk = NULL;
	static Graph* last_word = NULL;
	static GraphEdge* last_edge = NULL;
	AstFile* file = (AstFile*) ast;
	AstHunk* hunk = (AstHunk*) ast;
	AstWord* word = (AstWord*) ast;
	Graph* graph = NULL;
	switch (ast->type) {
		case AST_FILE:
			if (filefilter != NULL && strcmp(file->name, filefilter) != 0) {
				break;
			}
			graph = (Graph*) new_graph_file(file->name);
			if (last_file != NULL) {
				last_file->next = graph;
				graph->prev = last_file;
			}
			if (root->first_file == NULL) {
				root->first_file = (GraphFile*) graph;
			}
			last_file = graph;
			walk((Ast*) file->first_hunk, graph, root);
			break;
		case AST_HUNK:
			if (hunkfilter != NULL && strcmp(hunk->name, hunkfilter) != 0) {
				break;
			}
			if (!(graph = find_node((Graph*) root->first_hunk, hunk->name))) {
				graph = (Graph*) new_graph_hunk(hunk->name);
				if (last_hunk != NULL) {
					last_hunk->next = graph;
					graph->prev = last_hunk;
				}
				last_hunk = graph;
			}
			if (root->first_hunk == NULL) {
				root->first_hunk = (GraphHunk*) graph;
			}
			GraphEdge* edge = find_edge(root->first_edge, parent, graph);
			if (edge == NULL) {
				edge = new_graph_edge(parent, graph);
				if (last_edge != NULL) {
					last_edge->next = edge;
					edge->prev = last_edge;
				}
				if (root->first_edge == NULL) {
					root->first_edge = edge;
				}
				last_edge = edge;
			}
			walk((Ast*) hunk->first_word, graph, root);
			break;
		case AST_WORD:
			if (wordfilter != NULL && strcmp(word->str, wordfilter) != 0) {
				break;
			}
			if (!(graph = find_node((Graph*) root->first_word, word->str))) {
				graph = (Graph*) new_graph_word(word->str);
				if (last_word != NULL) {
					last_word->next = graph;
					graph->prev = last_word;
				}
				last_word = graph;
			}
			if (root->first_word == NULL) {
				root->first_word = (GraphWord*) graph;
			}
			edge = find_edge(root->first_edge, parent, graph);
			if (edge == NULL) {
				edge = new_graph_edge(parent, graph);
				if (last_edge != NULL) {
					last_edge->next = edge;
					edge->prev = last_edge;
				}
				if (root->first_edge == NULL) {
					root->first_edge = edge;
				}
				last_edge = edge;
			}
			graph_word_occurence((GraphWord*) graph, edge, word->wtype);
			if (edge->num_added + edge->num_removed > root->highest_edge_num_changed) {
				root->highest_edge_num_changed = edge->num_added + edge->num_removed;
			}
			break;
		default:
			fprintf(stderr, "Missing implementation for %i\n", ast->type);
			exit(1);
	}
	walk(ast->next, parent, root);
	return root;
}

static GraphRoot* new_graph_root(void) {
	GraphRoot* root = malloc(sizeof(GraphRoot));
	root->first_file = NULL;
	root->first_hunk = NULL;
	root->first_word = NULL;
	root->first_edge = NULL;
	root->highest_edge_num_changed = 0;
	return root;
}

static GraphFile* new_graph_file(char* name) {
	GraphFile* file = malloc(sizeof(GraphFile));
	file->type = AST_FILE;
	file->name = name;
	file->next = NULL;
	file->prev = NULL;
	return file;
}

static GraphHunk* new_graph_hunk(char* name) {
	GraphHunk* hunk = malloc(sizeof(GraphHunk));
	hunk->type = AST_HUNK;
	hunk->name = name;
	hunk->next = NULL;
	hunk->prev = NULL;
	return hunk;
}

static GraphWord* new_graph_word(char* str) {
	GraphWord* word = malloc(sizeof(GraphWord));
	word->type = AST_WORD;
	word->str = str;
	word->next = NULL;
	word->prev = NULL;
	word->num_added = 0;
	word->num_removed = 0;
	word->num_unchanged = 0;
	return word;
}

static GraphEdge* new_graph_edge(Graph* src, Graph* dest) {
	GraphEdge* edge = malloc(sizeof(GraphEdge));
	edge->type = GRAPH_EDGE;
	edge->next = NULL;
	edge->prev = NULL;
	edge->num_added = 0;
	edge->num_removed = 0;
	edge->num_unchanged = 0;
	edge->src = src;
	edge->dest = dest;
	return edge;
}

static void graph_word_occurence(GraphWord* word, GraphEdge* edge, GraphWordType wtype) {
	switch (wtype) {
		case ADDED:
			word->num_added++;
			edge->num_added++;
			break;
		case REMOVED:
			word->num_removed++;
			edge->num_removed++;
			break;
		case UNCHANGED:
			word->num_unchanged++;
			edge->num_unchanged++;
			break;
		default:
			fprintf(stderr, "Missing implementation for %i\n", wtype);
			exit(1);
	}
}

static Graph* find_node(Graph* graph, char* name) {
	if (graph == NULL) {
		return NULL;
	}
	if (strcmp(graph->name, name) == 0) {
		return graph;
	}
	return find_node(graph->next, name);
}

static GraphEdge* find_edge(GraphEdge* graph, Graph* src, Graph* dest) {
	if (graph == NULL) {
		return NULL;
	}
	if (graph->src == src && graph->dest == dest) {
		return graph;
	}
	return find_edge(graph->next, src, dest);
}

