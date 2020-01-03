#ifndef GRAPH_H
#define GRAPH_H

typedef enum GraphType GraphType;
typedef enum GraphWordType GraphWordType;
typedef struct GraphRoot GraphRoot;
typedef struct GraphEdge GraphEdge;
typedef struct Graph Graph;
typedef struct GraphFile GraphFile;
typedef struct GraphHunk GraphHunk;
typedef struct GraphWord GraphWord;

enum GraphType {
	GRAPH_FILE,
	GRAPH_HUNK,
	GRAPH_WORD,
	GRAPH_EDGE
};

enum GraphWordType {
	GRAPH_ADDED = '+',
	GRAPH_REMOVED = '-',
	GRAPH_UNCHANGED = ' '
};

/* abstract type for all graph nodes */
struct Graph {
	GraphType type;
	Graph* next;
	Graph* prev;
	char* name;
};

struct GraphFile {
	GraphType type;
	GraphFile* next;
	GraphFile* prev;
	char* name;
};

struct GraphHunk {
	GraphType type;
	GraphHunk* next;
	GraphHunk* prev;
	char* name;
};

struct GraphWord {
	GraphType type;
	GraphWord* next;
	GraphWord* prev;
	char* str;
	int num_added;
	int num_removed;
	int num_unchanged;
};

struct GraphEdge {
	GraphType type;
	GraphEdge* next;
	GraphEdge* prev;
	int num_added;
	int num_removed;
	int num_unchanged;
	Graph* src;
	Graph* dest;
};

struct GraphRoot {
	GraphFile* first_file;
	GraphHunk* first_hunk;
	GraphWord* first_word;
	GraphEdge* first_edge;
	int highest_edge_num_changed;
};

void graph_setnext(Graph* g, Graph* next, GraphRoot* root);

#endif
