#include "tokenizer.h"
#include "parsemitter.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

const char *keywords[] = { KEYWORD_LIST(AS_STR) };

const int keyword_len = sizeof(keywords) / sizeof(keywords[0]);

static char *load_file(char* filename) {
    FILE *fp;

    fp = fopen(filename, "rb");

    if (fp == NULL) {
        printf("Error: Could not open file\n");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size < 1) {
        printf("Error: Could not locate end of file\n");
        fclose(fp);
        return NULL;
    }
    rewind(fp);

    char *buf = malloc(size + 1);
    if (!buf) {
        printf("Error: Could not allocate memory\n");
        fclose(fp);
        return NULL;
    }

    if ((long)fread(buf, 1, size, fp) < size) {
        printf("Error: Partial read failure on writing buf");
        free(buf);
        fclose(fp);
        return NULL;
    }
    buf[size] = '\0';
    fclose(fp);

    return buf;
}

static void skip_whitespace(Reader* r) {
    while (isspace(*r->ptr)) {
        if (*r->ptr == '\n') {
            r->line++;
            r->col = 1;
        }
        else r->col++;
        r->ptr++;
    }
}

// Only call if current character is < or >
static inline bool check_bitshift(Reader *r) {
    if (r->ptr[1] && r->ptr[1] == r->ptr[0]) {
        r->ptr++;
        r->col++;
        return true;
    }
    return false;
}

static inline bool check_trailing_equals(Reader *r) {
    if (r->ptr[1] && r->ptr[1] == '=') {
        r->ptr++;
        r->col++;
        return true;
    }
    return false;
}

static Token lex_next(Reader* r) {
    skip_whitespace(r);

    Token t;
    // Start line and column will be the same for all situations
    t.ptr = r->ptr;
    t.keyword = NO_KEYWORD;
    t.line = r->line;
    t.col = r->col;

    char ch = *(r->ptr);

    // Check for EOF
    if (!ch) {
        t.type = TOK_EOF;
        t.len = 0;

        return t;
    }

    // Check for comment
    if (ch == '#') {
        t.ptr = r->ptr;
        while (*r->ptr && *r->ptr != '\n') {
            r->ptr++;
            r->col++;
        }

        t.type = TOK_COMMENT;
        t.len = r->ptr - t.ptr;

        if (*r->ptr == '\n') {
            r->ptr++;
            r->line++;
            r->col = 1;
        }

        return t;
    }

    // Ident or keyword
    if (isalpha(ch) || ch == '_') {
        while (isalnum(*r->ptr) || *r->ptr == '_') {
            r->ptr++;
            r->col++;
        }
        
        // We now have length of identifier
        t.len = r->ptr - t.ptr;

        // check if keyword
        bool is_keyword = false;
        int i  = 0;
        for (; i < keyword_len; ++i) {
            size_t key_len = strlen(keywords[i]);
            if (key_len != t.len) continue;

            if (!memcmp(t.ptr, keywords[i], key_len)) {
                is_keyword = true;
                break;
            }
        }

        t.type = is_keyword ? TOK_KEYWORD : TOK_IDENT;
        t.keyword = is_keyword ? (KeywordType)(i + 1) : NO_KEYWORD;

        return t;
    }

    // Numbers, check for leading dot
    if(isdigit(ch) || (ch == '.' && isdigit(r->ptr[1]))) {
        t.ptr = r->ptr;
        bool is_float = false;

        if (*r->ptr == '.') {
            is_float = true;
            r->ptr++;
            r->col++;
        }

        while (isdigit(*r->ptr)) {
            r->ptr++;
            r->col++;
            if (*r->ptr == '.' && !is_float) {
                is_float = true;
                r->ptr++;
                r->col++;
            }
        }

        t.type = is_float ? TOK_FLOAT_LITERAL : TOK_INT_LITERAL;
        t.len = r->ptr - t.ptr;

        return t;
    }

    // Character literals, must be one character excluding escape situations
    // Must always check next char is not EOF
    if (ch == '\'' && r->ptr[1]) {
        // valid escape characters here are \', \\, \t, and \n
        bool is_escape = r->ptr[1] == '\\';
        char escaped = is_escape ? r->ptr[2] : 0;
        bool valid_escape = escaped == '\'' || escaped == '\\' || escaped == 't' || escaped == 'n';

        bool plain_literal = !is_escape && r->ptr[1] != '\'' && r->ptr[2] == '\'';
        bool escape_literal = is_escape && valid_escape && r->ptr[3] == '\'';

        if (plain_literal || escape_literal) {
            int consumed = is_escape ? 4 : 3;

            r->ptr += consumed;
            r->col += consumed;

            t.type = TOK_CHAR;
            t.len = consumed;
        }
        else {
            r->ptr++;
            r->col++;

            t.type = TOK_UNKNOWN;
            t.len = 1;
        }

        return t;
    }

    // String literals
    // Allowing newlines in strings explicitly at this time
    if (ch == '"') {
        do {
            if (*r->ptr == '\n') {
                r->line++;
                r->col = 1;
            }
            // Valid escape characters are \", \\, \t, and \n
            else if (*r->ptr == '\\') {
                char next = r->ptr[1];
                if (next == '"' || next == '\\' || next == 't' || next == 'n') {
                    r->col++;
                    r->ptr++;
                }
                // If invalid escape character, consider an invalid string
                else break;
            }
            else r->col++;

            r->ptr++;
        } while (*r->ptr && *r->ptr != '"');

        if (*r->ptr == '"') {
            r->ptr++;
            r->col++;
            t.type = TOK_STR;
            t.len = r->ptr - t.ptr;
        }
        else {
            r->ptr = t.ptr + 1;
            r->line = t.line;
            r->col = t.col + 1;

            t.type = TOK_UNKNOWN;
            t.len = 1;
        }

        return t;
    }

    // Single token operators
    switch(ch) {
        case '+':
            if (check_trailing_equals(r)) t.type = TOK_PLUSEQ;
            else t.type = TOK_PLUS;
            break;
        case '-':
            if (check_trailing_equals(r)) t.type = TOK_MINUSEQ;
            else t.type = TOK_MINUS;
            break;
        case '*':
            if (check_trailing_equals(r)) t.type = TOK_STAREQ;
            else t.type = TOK_STAR;
            break;
        case '/':
            if (check_trailing_equals(r)) t.type = TOK_SLASHEQ;
            else t.type = TOK_SLASH;
            break;
        case '%':
            if (check_trailing_equals(r)) t.type = TOK_MODEQ;
            else t.type = TOK_MOD;
            break;
        case '|':
            if (check_trailing_equals(r)) t.type = TOK_BITOREQ;
            else t.type = TOK_BITOR;
            break;
        case '&':
            if (check_trailing_equals(r)) t.type = TOK_BITANDEQ;
            else t.type = TOK_BITAND;
            break;
        case '^':
            if (check_trailing_equals(r)) t.type = TOK_BITXOREQ;
            else t.type = TOK_BITXOR;
            break;
        case '~':
            t.type = TOK_BITNOT;
            break;
        case '.':
            t.type = TOK_DOT;
            break;
        case '?':
            t.type = TOK_TERNARY;
            break;
        case ',':
            t.type = TOK_COMMA;
            break;
        case '(':
            t.type = TOK_LPAREN;
            break;
        case ')':
            t.type = TOK_RPAREN;
            break;
        case '[':
            t.type = TOK_LBRACK;
            break;
        case ']':
            t.type = TOK_RBRACK;
            break;
        case '{':
            t.type = TOK_LCURLY;
            break;
        case '}':
            t.type = TOK_RCURLY;
            break;
        case '<':
            if (check_bitshift(r)) {
                if (check_trailing_equals(r)) t.type = TOK_BITLSHIFTEQ; // <<=
                else t.type = TOK_BITLSHIFT;
            }
            else if (check_trailing_equals(r)) t.type = TOK_LE;
            else t.type = TOK_LT;
            break;
        case '>':
            if (check_bitshift(r)) {
                if (check_trailing_equals(r)) t.type = TOK_BITRSHIFTEQ; // >>=
                else t.type = TOK_BITRSHIFT;
            }
            else if (check_trailing_equals(r)) t.type = TOK_GE;
            else t.type = TOK_GT;
            break;
        case '=':
            if (check_trailing_equals(r)) t.type = TOK_EQ;
            else t.type = TOK_ASSIGN;
            break;
        case ';':
            t.type = TOK_SEMICOLON;
            break;
        case ':':
            t.type = TOK_COLON;
            break;
        // only valid with != for now, no other use for ! since we're using not for negation
        case '!':
            if (check_trailing_equals(r)) t.type = TOK_NOTEQ;
            else t.type = TOK_BANG;
            break;
        default:
            t.type = TOK_UNKNOWN;
            break;
    }

    r->ptr++;
    r->col++;
    t.len = r->ptr - t.ptr;
    return t;
}

#ifdef DEBUG
static void print_token(Token* t) {
    const char *type_name;
    switch (t->type) {
        case TOK_INT_LITERAL:   type_name = "INT_LIT";       break;
        case TOK_FLOAT_LITERAL: type_name = "FLOAT_LIT";     break;
        case TOK_IDENT:         type_name = "IDENT";         break;
        case TOK_KEYWORD:       type_name = "KEYWORD";       break;
        case TOK_PLUS:          type_name = "PLUS";          break;
        case TOK_MINUS:         type_name = "MINUS";         break;
        case TOK_STAR:          type_name = "STAR";          break;
        case TOK_SLASH:         type_name = "SLASH";         break;
        case TOK_MOD:           type_name = "MOD";           break;
        case TOK_BITOR:         type_name = "BIT_OR";        break;
        case TOK_BITAND:        type_name = "BIT_AND";       break;
        case TOK_BITXOR:        type_name = "BIT_XOR";       break;
        case TOK_BITNOT:        type_name = "BIT_NOT";       break;
        case TOK_BITLSHIFT:     type_name = "BIT_LSHIFT";    break;
        case TOK_BITRSHIFT:     type_name = "BIT_RSHIFT";    break;
        case TOK_DOT:           type_name = "DOT";           break;
        case TOK_TERNARY:       type_name = "TERNARY";       break;
        case TOK_COMMA:         type_name = "COMMA";         break;
        case TOK_CHAR:          type_name = "CHAR_LIT";      break;
        case TOK_STR:           type_name = "STR_LIT";       break;
        case TOK_COMMENT:       type_name = "COMMENT";       break;
        case TOK_LPAREN:        type_name = "LPAREN";        break;
        case TOK_RPAREN:        type_name = "RPAREN";        break;
        case TOK_LBRACK:        type_name = "LBRACK";        break;
        case TOK_RBRACK:        type_name = "RBRACK";        break;
        case TOK_LCURLY:        type_name = "LCURLY";        break;
        case TOK_RCURLY:        type_name = "RCURLY";        break;
        case TOK_LT:            type_name = "LT";            break;
        case TOK_GT:            type_name = "GT";            break;
        case TOK_LE:            type_name = "LE";            break;
        case TOK_GE:            type_name = "GE";            break;
        case TOK_ASSIGN:        type_name = "ASSIGN";        break;
        case TOK_PLUSEQ:        type_name = "PLUS_EQ";       break;
        case TOK_MINUSEQ:       type_name = "MINUS_EQ";      break;
        case TOK_STAREQ:        type_name = "STAR_EQ";       break;
        case TOK_SLASHEQ:       type_name = "SLASH_EQ";      break;
        case TOK_MODEQ:         type_name = "MOD_EQ";        break;
        case TOK_BITOREQ:       type_name = "BIT_OR_EQ";     break;
        case TOK_BITANDEQ:      type_name = "BIT_AND_EQ";    break;
        case TOK_BITXOREQ:      type_name = "BIT_XOR_EQ";    break;
        case TOK_BITLSHIFTEQ:   type_name = "BIT_LSHIFT_EQ"; break;
        case TOK_BITRSHIFTEQ:   type_name = "BIT_RSHIFT_EQ"; break;
        case TOK_EQ:            type_name = "EQ";            break;
        case TOK_NOTEQ:         type_name = "NOTEQ";         break;
        case TOK_SEMICOLON:     type_name = "SEMICOLON";     break;
        case TOK_COLON:         type_name = "COLON";         break;
        case TOK_BANG:          type_name = "BANG";          break;
        case TOK_EOF:           type_name = "EOF";           break;
        case TOK_UNKNOWN:       type_name = "UNKNOWN";       break;

        // This should never hit but for safety
        default:            type_name = "WTF???";    break;
    }

    int text_len = (int)(t->len < 20 ? t->len : 20);
    printf("%-14s %-20.*s Line: %-4d Col: %-4d\n",
           type_name, text_len, t->ptr ? t->ptr : "", t->line, t->col);
}
#endif

bool generate_tokens(char* filepath) {

    char *text = load_file(filepath);
    if (!text) return false;
    
    push_one();
    Reader r = { .stream = text, .ptr = text, .line = 1, .col = 1 };

    Token t = {0};
    while (t.type != TOK_EOF) {
        t = lex_next(&r);

        #ifdef DEBUG
        print_token(&t);
        #endif

        send_token(t);
    }

    free(text);

    #ifdef DEBUG
    printf("%d tokens were sent\n", num_of_tokens());
    #endif

    return true;
}