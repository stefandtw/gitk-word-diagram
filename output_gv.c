#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include "graph.h"
#include "graph_walk.h"
#include "output_gv.h"

Agraph_t* ag;
GVC_t* gvc;
Agnode_t* current_file;
Agnode_t* current_hunk;

static void add_file(GraphFile* file);
static void add_hunk(GraphHunk* hunk);
static void add_word(GraphWord* word);
static void add_edge(GraphEdge* edge, GraphRoot* root);
static void add_button_row(Agnode_t* related_node, char* param, char* node_name);
static char* node_label(char* s, int maxcharsline, int maxlines, bool escapehtml);

void output_gv(GraphRoot* g, int width, int height) {
	char gdef[300];
	int graphviz_dpi = 96;
	double width_inches = (double) width / graphviz_dpi;
	double height_inches = (double) height / graphviz_dpi;
	sprintf(gdef, "strict digraph x {"
			" size=\"%f,%f\";"
			" ratio=collapse;"
			" splines=true;"
			" outputorder=edgesfirst; rankdir=RL;"
			" nodesep=0.02;"
			" node [fontname = courier];"
			" }", width_inches, height_inches);
	ag = agmemread(gdef);
	gvc = gvContext();
	graph_walk(g, &add_file, &add_hunk, &add_word, &add_edge);
	gvLayout(gvc, ag, "dot");
	gvRender(gvc, ag, "dot", stdout);
	gvFreeLayout(gvc, ag);
	gvLayout(gvc, ag, "dot");
	gvRenderFilename(gvc, ag, "imap_np", "words.imap_np");
	gvRenderFilename(gvc, ag, "png", "words.png");
	gvFreeLayout(gvc, ag);
	agclose(ag);
	gvFreeContext(gvc);
}

static void add_file(GraphFile* file) {
	char* label = node_label(file->name, 20, 4, false);
	Agnode_t* node = agnode(ag, file->name, TRUE);
	agsafeset(node, "label", label, "\\N");
	agsafeset(node, "shape", "box", "");
	agsafeset(node, "fontcolor", "gray40", "");
	add_button_row(node, "file", file->name);
	current_file = node;
}

static void add_hunk(GraphHunk* hunk) {
	char* function = hunk->name;
	char* label = node_label(function, 30, 2, false);
	Agnode_t* node = agnode(ag, function, TRUE);
	agsafeset(node, "label", label, "\\N");
	agsafeset(node, "ishunknode", "true", "");
	agsafeset(node, "fontcolor", "blue3", "");
	agsafeset(node, "fontsize", "12", "14");
	agsafeset(node, "shape", "box", "");
	add_button_row(node, "hunk", hunk->name);
	current_hunk = node;
}

static void add_word(GraphWord* word) {
	char* label = node_label(word->str, 25, 4, false);
	Agnode_t* ag_node = agnode(ag, word->str, TRUE);
	agsafeset(ag_node, "label", label, "\\N");
	agsafeset(ag_node, "style", "filled", "");
	agsafeset(ag_node, "fontcolor", "black", "");
	if (word->num_added > 0 && word->num_removed == 0) {
		agsafeset(ag_node, "fillcolor", "#e7ffe7", "");
	} else if (word->num_added == 0 && word->num_removed > 0) {
		agsafeset(ag_node, "fillcolor", "#ffe7e7", "");
	} else if (word->num_added > 0 && word->num_removed > 0) {
		agsafeset(ag_node, "fillcolor", "#ffe7e7:#e7ffe7", "");
	}
	add_button_row(ag_node, "word", word->str);
}

static void add_edge(GraphEdge* edge, GraphRoot* root) {
	Agnode_t* ag_src = agnode(ag, edge->src->name, TRUE);
	Agnode_t* ag_dest = agnode(ag, edge->dest->name, TRUE);
	Agedge_t* ag_edge = agedge(ag, ag_src, ag_dest, NULL, TRUE);
	float ratio = (float)(edge->num_added + edge->num_removed) / root->highest_edge_num_changed;
	// min: 1, max: 5
	float f_penwidth = ratio * 4 + 1;
	char penwidth[20];
	sprintf(penwidth, "%g", f_penwidth);
	agsafeset(ag_edge, "penwidth", penwidth, "");
	if (edge->dest->type == GRAPH_WORD) {
		if (edge->num_added > 0 && edge->num_removed == 0) {
			agsafeset(ag_edge, "color", "green", "");
		} else if (edge->num_added == 0 && edge->num_removed > 0) {
			agsafeset(ag_edge, "color", "red", "");
		} else if (edge->num_added > 0 && edge->num_removed > 0) {
			agsafeset(ag_edge, "color", "#eeee00", "");
		}
		char num_occurences_str[23];
		if (edge->num_removed == 0) {
			sprintf(num_occurences_str, "+%i", edge->num_added);
		} else if (edge->num_added == 0) {
			sprintf(num_occurences_str, "-%i", edge->num_removed);
		} else {
			sprintf(num_occurences_str, "+%i -%i", edge->num_added, edge->num_removed);
		}
		agsafeset(ag_edge, "label", num_occurences_str, "");
	}
}

static void add_button_row(Agnode_t* related_node, char* param, char* node_name) {
	char label[5000];
	char* name_escaped = node_label(node_name, 10000, 1, true);
	sprintf(label, "<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" VALIGN=\"top\">"
			"<TR>"
			"<TD bgcolor=\"#ececec\" HREF=\""
			"gitk:search_prev &quot;%s&quot;"
			"\">&lt;</TD>"
			"<TD style=\"invis\" width=\"3\" cellpadding=\"0\"></TD>"
			"<TD bgcolor=\"#ececec\" HREF=\""
			"gitk:search_next &quot;%s&quot;"
			"\">&gt;</TD>"
			"<TD style=\"invis\" width=\"3\" cellpadding=\"0\"></TD>"
			"<TD bgcolor=\"#ececec\" HREF=\""
			"gitk:load_graph &quot;--%s '%s'&quot;"
			"\">✀</TD>"
			"</TR>"
			"</TABLE>"
			, name_escaped, name_escaped, param, name_escaped
	       );

	char* related_name = agnameof(related_node);
	char name[strlen(related_name) + 1 + strlen(label) + 1];
	sprintf(name, "%s_%s", related_name, label);
	Agnode_t* ag_node = agnode(ag, name, TRUE);
	agsafeset(ag_node, "label", agstrdup_html(ag, label), "\\N");
	agsafeset(ag_node, "fontcolor", "black", "");
	agsafeset(ag_node, "fontsize", "9", "14");
	agsafeset(ag_node, "shape", "none", "");
	agsafeset(ag_node, "width", "0.25", "0.75");
	agsafeset(ag_node, "height", "0.38", "0.5");
	agsafeset(ag_node, "margin", "0.0", "0.11,0.055");
	agsafeset(ag_node, "labelloc", "t", "c");

	Agedge_t* ag_edge = agedge(ag, related_node, ag_node, NULL, TRUE);
	agsafeset(ag_edge, "style", "invis", "");
	agsafeset(ag_edge, "minlen", "0.0", "1");
}

static char* node_label(char* s, int maxcharsline, int maxlines, bool escapehtml) {
	int maxcharstotal = maxcharsline * maxlines - 2;
	int extra_for_escaping = 50;
	char label[strlen(s) + (strlen(s) / maxcharsline) + 1 + extra_for_escaping];
	int inserted = 0;
	int i = 0;
	for (; i<strlen(s); i++) {
		if (escapehtml && s[i] == '"') {
			label[i+inserted++] = '&';
			label[i+inserted++] = 'q';
			label[i+inserted++] = 'u';
			label[i+inserted++] = 'o';
			label[i+inserted++] = 't';
			label[i+inserted++] = ';';
		} else if (escapehtml && s[i] == '&') {
			label[i+inserted++] = '&';
			label[i+inserted++] = 'a';
			label[i+inserted++] = 'm';
			label[i+inserted++] = 'p';
			label[i+inserted++] = ';';
		} else {
			label[i+inserted] = s[i];
		}
		if ((i+1) % maxcharsline == 0) {
			inserted++;
			label[i+inserted] = '\n';
		}
		if (i + 1 == maxcharstotal - 3) {
			label[i+inserted++] = '.';
			label[i+inserted++] = '.';
			label[i+inserted++] = '.';
			break;
		}
	}
	label[i+inserted] = '\0';
	return agstrdup(ag, label);
}
