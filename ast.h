#ifndef AST_H
#define AST_H

typedef enum AstType AstType;
typedef enum AstWordType AstWordType;
typedef struct Ast Ast;
typedef struct AstFile AstFile;
typedef struct AstHunk AstHunk;
typedef struct AstWord AstWord;

enum AstType {
	AST_FILE,
	AST_HUNK,
	AST_WORD
};

enum AstWordType {
	ADDED = '+',
	REMOVED = '-',
	UNCHANGED = ' '
};

/* abstract type for all ast elements */
struct Ast {
	AstType type;
	Ast* next;
};

struct AstFile {
	AstType type;
	AstFile* next;
	char* name;
	AstHunk* first_hunk;
};

struct AstHunk {
	AstType type;
	AstHunk* next;
	char* name;
	AstWord* first_word;
};

struct AstWord {
	AstType type;
	AstWord* next;
	AstWordType wtype;
	char* str;
};

AstFile* new_file(char* name, AstHunk* first_hunk, AstFile* next);
AstHunk* new_hunk(char* name, AstWord* first_word);
AstWord* new_word(AstWordType wtype, char* str, AstWord* prev);

void free_file(AstFile* file);
void free_hunk(AstHunk* hunk);
void free_word(AstWord* word);

#endif
