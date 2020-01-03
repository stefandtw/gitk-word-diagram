%{
#include "parser.h"
%}

%option noyywrap nodefault yylineno

%x IN_HEADER
%x IN_ATLINE
%x IN_ATLINE_END
%x IN_HUNK
%x IN_CHANGE
%x IN_ANYWORDS

%%

<INITIAL>.|"\n"	{ /* ignore what is before "diff" */ }
<INITIAL,IN_HUNK>^"diff"	{ BEGIN IN_HEADER; return DIFF; }

<IN_HEADER,IN_HUNK>^"--- "a?"/"	{ BEGIN IN_ANYWORDS; return MINUS3; }
<IN_ANYWORDS>^"+++ "b?"/"	{ return PLUS3; }

<IN_ANYWORDS,IN_HUNK>^"@@"	{ BEGIN IN_ATLINE; return AT2; }
<IN_ATLINE>[^@]+	{ yylval.str = strdup(yytext); return RANGE; }
<IN_ATLINE>"@@"" "?	{ BEGIN IN_ATLINE_END; return AT2; }
<IN_ATLINE_END>.+	{ yylval.str = strdup(yytext); return FUNC; }
<IN_ATLINE,IN_ATLINE_END>"\n"	{ BEGIN IN_HUNK; return EOL; }
<IN_ATLINE>[^ \t\n]	{ fprintf(stderr, "warning:%d: character '%s' before @@\n", yylineno, yytext);  }

<IN_HUNK>^"~"$	{ return yytext[0]; }
<IN_HUNK>^[ +-]	{ BEGIN IN_CHANGE; return yytext[0]; }
<IN_CHANGE>"\n"	{ BEGIN IN_HUNK; return EOL; }
<IN_CHANGE>[a-zA-Z0-9\x80-\xf3_]+	{ yylval.str = strdup(yytext); return HUNKWORD; }
<IN_HUNK,IN_CHANGE>.	{ /* ignore */ }

<IN_HEADER,IN_ANYWORDS>[^ \t\n]+	{ yylval.str = strdup(yytext); return ANYWORD; }
<IN_HEADER,IN_HUNK,IN_ANYWORDS>"\n"	{ return EOL; }
<IN_HEADER,IN_ATLINE,IN_HUNK,IN_ANYWORDS>[ \t]	{ /* ignore whitespace */ }

%%

