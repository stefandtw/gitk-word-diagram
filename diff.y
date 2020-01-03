%{
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "graph.h"
#include "graph_factory.h"
#include "graph_filter.h"
#include "output_gv.h"
#include "output_plain.h"

int yylex(void);
void yyerror(const char* msg);

int yylineno;

AstFile* first_file;
AstHunk* first_hunk = NULL;
AstWord* first_word = NULL;
AstWord* previous_word = NULL;

%}

%define parse.error verbose

%union {
	char* str;
	char ch;
	struct AstFile* file;
	struct AstHunk* hunk;
	struct AstWord* word;
}

%token DIFF
%token AT2
%token RANGE
%token <str> FUNC
%token PLUS3
%token MINUS3
%token EOL
%token <str> ANYWORD
%token <str> HUNKWORD

%type <file> all;
%type <file> files;
%type <file> file;
%type <str> headers;
%type <str> header;
%type <str> header_line_minus;
%type <str> header_line_plus;
%type <hunk> hunks;
%type <hunk> hunk;
%type <str> range_line;
%type <word> hunk_lines;
%type <word> hunk_line;
%type <word> added_hunkwords;
%type <word> removed_hunkwords;
%type <word> unchanged_hunkwords;

%%

all: files	

files: file files 	{
     $1->next = first_file;
	$$ = $2 != NULL ? $2 : $1;
	first_file = $1;
     }
     | headers /* e.g. pure rename */ { $$ = NULL; }
     | /* end of recursion */ { $$ = NULL; }
     ;

file: headers hunks	{ $$ = new_file($1, first_hunk, NULL); }
    ;

headers: headers header EOL	{ $$ = $2 != NULL && strcmp($2, "dev/null") != 0 ? $2 : $1; }
       | header EOL		{ $$ = $1; }
       ;

header: file_line	{ $$ = NULL; }
      | anywords	{ $$ = NULL; }
      | header_line_minus	{ $$ = $1; }
      | header_line_plus	{ $$ = $1; }
      ;

file_line: DIFF anywords
	 ;

header_line_minus: MINUS3 ANYWORD	{ $$ = $2; }
		 ;

header_line_plus: PLUS3 ANYWORD	{ $$ = $2; }
		;

hunks: hunks hunk	{ $$ = $2; $1->next = $2; }
     | hunk		{ $$ = $1; first_hunk = $$; }
     ;

hunk: range_line hunk_lines	{ $$ = new_hunk($1, first_word); previous_word = NULL; first_word = NULL; }
    ;

range_line: AT2 RANGE AT2 FUNC EOL	{ $$ = $4; }
	  | AT2 RANGE AT2 EOL	{ $$ = "..."; }
	  ;

hunk_lines: hunk_lines hunk_line	{ $$ = $2 != NULL ? $2 : $1; }
	  | hunk_line			{ $$ = NULL; }
	  ;

hunk_line: '~' EOL	{ $$ = NULL; }
	 | '+' EOL	{ $$ = NULL; }
	 | '-' EOL	{ $$ = NULL; }
	 | ' ' EOL	{ $$ = NULL; }
	 | '+' added_hunkwords EOL	{ $$ = $2; previous_word = $2; }
	 | '-' removed_hunkwords EOL	{ $$ = $2; previous_word = $2; }
	 | ' ' unchanged_hunkwords EOL	{ $$ = $2; previous_word = $2; }
	 ;

added_hunkwords: added_hunkwords HUNKWORD	{ $$ = new_word('+', $2, NULL); $1->next = $$; }
	       | HUNKWORD /* end of recursion */	{ $$ = new_word('+', $1, NULL); if (previous_word != NULL) { previous_word->next = $$; } if (first_word == NULL) { first_word = $$; } }
	       ;

removed_hunkwords: removed_hunkwords HUNKWORD	{ $$ = new_word('-', $2, NULL); $1->next = $$; }
		 | HUNKWORD /* end of recursion */	{ $$ = new_word('-', $1, NULL); if (previous_word != NULL) { previous_word->next = $$; } if (first_word == NULL) { first_word = $$; } }
		 ;

unchanged_hunkwords: unchanged_hunkwords HUNKWORD	{ $$ = new_word(' ', $2, NULL); $1->next = $$; }
		   | HUNKWORD /* end of recursion */	{ $$ = new_word(' ', $1, NULL); if (previous_word != NULL) { previous_word->next = $$; } if (first_word == NULL) { first_word = $$; } }
		   ;

anywords: anywords ANYWORD
	| /* end of recursion */
	;

%%

int main(int argc, char** argv) {
	char* file = NULL;
	char* hunk = NULL;
	char* word = NULL;
	int width = 0;
	int height = 0;
	char o;
	char format = 'g';
	static struct option long_options[] = {
		{"file",   required_argument,     0, 'f'},
		{"hunk",   required_argument,     0, 'h'},
		{"word",   required_argument,     0, 'w'},
		{"width",  required_argument,     0, 'W'},
		{"height", required_argument,     0, 'H'},
		{"plain",  no_argument,           0, 'p'},
		{"gv",     no_argument,           0, 'g'},
		{"help",   no_argument,           0, '?'},
		{0, 0, 0, 0}
	};
	while ((o = getopt_long(argc, argv, "h:", long_options, NULL)) != -1) {
		switch (o) {
			case 'f':
				file = optarg;
				break;
			case 'h':
				hunk = optarg;
				break;
			case 'w':
				word = optarg;
				break;
			case 'W':
				width = atoi(optarg);
				break;
			case 'H':
				height = atoi(optarg);
				break;
			case 'g':
			case 'p':
				format = o;
				break;
			case '?':
				printf("Usage: diffwords [OPTIONS]\n");
				printf("Reads a git porcelain word diff from stdin.\n");
				printf("  -g, --gv\tGraphviz and PNG output (default)\n");
				printf("  -p, --plain\tPlain text output\n");
				printf("  -f, --file FILE\tOnly show changes in FILE\n");
				printf("  -h, --hunk HUNK\tOnly show changes in HUNK\n");
				printf("  -w, --word WORD\tOnly show WORD\n");
				printf("  -W, --width WIDTH\tLimit image to WIDTH px\n");
				printf("  -H, --height HEIGHT\tLimit image to HEIGHT px\n");
				printf("  -?, --help\n");
				return 0;
		}
	}
	yyparse();
	GraphRoot* graph = new_graph((Ast*) first_file, file, hunk, word);
	graph_filter(graph);
	if (format == 'p') {
		output_plain(graph);
	} else if (format == 'g') {
		output_gv(graph, width, height);
	}
	return 0;
}

void yyerror(const char* msg) {
	fprintf(stderr, "error:%d: %s\n", yylineno, msg);
}
