#include <stdlib.h>
#include "ast.h"

AstFile* new_file(char* name, AstHunk* first_hunk, AstFile* next) {
	AstFile* file = malloc(sizeof(AstFile));
	file->type = AST_FILE;
	file->name = name;
	file->first_hunk = first_hunk;
	file->next = next;
	return file;
}

AstHunk* new_hunk(char* name, AstWord* first_word) {
	AstHunk* hunk = malloc(sizeof(AstHunk));
	hunk->type = AST_HUNK;
	hunk->name = name;
	hunk->first_word = first_word;
	hunk->next = NULL;
	return hunk;
}

AstWord* new_word(AstWordType wtype, char* str, AstWord* prev) {
	AstWord* word = malloc(sizeof(AstWord));
	word->type = AST_WORD;
	word->wtype = wtype;
	word->str = str;
	if (prev != NULL) {
		prev->next = word;
	}
	return word;
}

void free_file(AstFile* file) {
	free(file);
}

void free_hunk(AstHunk* hunk) {
	free(hunk);
}

void free_word(AstWord* word) {
	free(word);
}

