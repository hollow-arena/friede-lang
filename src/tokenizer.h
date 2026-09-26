#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TOK_INT_LITERAL,
    TOK_FLOAT_LITERAL,
    TOK_IDENT,       //   any runtime defined values
    TOK_KEYWORD,     //   as defined in keywords
    TOK_PLUS,        //   +
    TOK_MINUS,       //   -
    TOK_STAR,        //   *
    TOK_SLASH,       //   /
    TOK_MOD,         //   %
    TOK_BITOR,       //   |
    TOK_BITAND,      //   &
    TOK_BITXOR,      //   ^
    TOK_BITNOT,      //   ~
    TOK_BITLSHIFT,   //   <<
    TOK_BITRSHIFT,   //   >>
    TOK_DOT,         //   .
    TOK_TERNARY,     //   ?
    TOK_COMMA,       //   ,
    TOK_CHAR,        //   'x', '\', '\t', '\n'
    TOK_STR,         //   "Hello World"
    TOK_COMMENT,     //   #
    TOK_LPAREN,      //   (
    TOK_RPAREN,      //   )
    TOK_LBRACK,      //   [
    TOK_RBRACK,      //   ]
    TOK_LCURLY,      //   {
    TOK_RCURLY,      //   }
    TOK_LT,          //   <
    TOK_GT,          //   >
    TOK_LE,          //   <=
    TOK_GE,          //   >=
    TOK_ASSIGN,      //   =
    TOK_PLUSEQ,      //   +=
    TOK_MINUSEQ,     //   -=
    TOK_STAREQ,      //   *=
    TOK_SLASHEQ,     //   /=
    TOK_MODEQ,       //   %=
    TOK_BITOREQ,     //   |=
    TOK_BITANDEQ,    //   &=
    TOK_BITXOREQ,    //   ^=
    TOK_BITLSHIFTEQ, //   <<=
    TOK_BITRSHIFTEQ, //   >>=
    TOK_EQ,          //   ==
    TOK_NOTEQ,       //   !=
    TOK_SEMICOLON,   //   ;
    TOK_COLON,       //   :
    TOK_BANG,        //   !
    TOK_EOF,         //   '\0'
    TOK_UNKNOWN      //   When all hope is gone
} TokenType;

#define KEYWORD_LIST(X) \
    /* types */ \
    X(VOID, "void") \
    X(INT, "int") \
    X(FLOAT, "float") \
    X(CHAR, "char") \
    X(BOOL, "bool") \
    X(STR, "str") \
    X(ARR, "arr") /* generic type, e.g. arr<int> for an array of ints, is technically a struct */ \
    X(FUNC, "func") \
    X(CORO, "coro") /* coroutines */ \
    X(NULL, "null") \
    X(VAR, "var") /* type inference */ \
    \
    /* boolean values */ \
    X(TRUE, "true") \
    X(FALSE, "false") \
    \
    /* logical ops */ \
    X(OR, "or") \
    X(AND, "and") \
    X(NOT, "not") \
    \
    /* type qualifiers */ \
    X(CONST, "const") /* essentially "readonly". Can be assigned at runtime. Shallow constant. */ \
    \
    /* user defined types */ \
    X(CLASS, "class") \
    X(STRUCT, "struct") \
    X(ENUM, "enum") \
    \
    /* type keywords */ \
    X(NEW, "new") /* used to call constructor for any struct / class */ \
    X(WEAK, "weak") /* weak references, classes only */ \
    /* lists, sets, dicts, etc available in separate containers module built over array as primitive */ \
    \
    /* imports */ \
    X(IMPORT, "import") \
    X(USING, "using") \
    \
    /* control flow */ \
    X(IF, "if") \
    X(ELIF, "elif") \
    X(ELSE, "else") \
    X(SWITCH, "switch") \
    X(CASE, "case") \
    X(DEFAULT, "default") \
    \
    /* loops */ \
    X(FOR, "for") /* for var i in items.iter(), for int i in range(10), etc. */ \
    X(IN, "in") /* all for loops go through a generator / coroutine like Python */ \
    \
    /* old design:
       "to",      for i = 0 to 10 ======> for (int i = 0; i < 10; ++i)
       "thru",    for i = 0 thru 10 ====> for (int i = 0; i <= 10; ++i)
       "by",      for i = 0 to 10 by 2 => for (int i = 0; i < 10; i += 2)
       "foreach", iterate through a collection */ \
    \
    X(WHILE, "while") \
    X(DO, "do") \
    \
    /* jumps */ \
    X(BREAK, "break") \
    X(CONTINUE, "continue") \
    X(RETURN, "return") /* "yield()" is a method, not a keyword */ \
    \
    /* error handling */ \
    X(TRY, "try") \
    X(EXCEPT, "except") \
    X(RAISE, "raise")

#define AS_ENUM(name, str) KEYWORD_##name,
#define AS_STR(name, str)  str,

typedef enum { NO_KEYWORD, KEYWORD_LIST(AS_ENUM) KEYWORD_COUNT } KeywordType;

typedef struct {
    TokenType type;
    KeywordType keyword;
    char* ptr;
    uint64_t len;
    int line;
    int col;
} Token;

typedef struct {
    char* stream;
    char* ptr;
    int line;
    int col;
} Reader;

extern const char *keywords[];

extern const int keyword_len;

bool generate_tokens(char* filepath);

#ifdef __cplusplus
}
#endif