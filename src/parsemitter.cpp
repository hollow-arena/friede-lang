#include "parsemitter.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstddef>

using namespace std;

static vector<Token> tokens;
static vector<Token> line;

// Tokens come in start to finish
// May need to reverse to pop them to emit bytecode
void send_token(Token t) {
    tokens.push_back(t);
}

void build_expr(vector<Token>& line) {
    
}

void build_lines() {
    size_t i = 0;
    
    auto flush = [&]() {
        #ifdef DEBUG
        for (const auto& t : line) {
            cout.write(t.ptr, t.len);
            cout << "  ";
        }
        cout << endl;
        #endif
        build_expr(line);
        line.clear();
    };

    while (tokens[i].type != TOK_EOF) {

        while (tokens[i].type != TOK_SEMICOLON
            && tokens[i].type != TOK_LCURLY
            && tokens[i].type != TOK_RCURLY
            && tokens[i].type != TOK_EOF) {
            if (tokens[i].type == TOK_COMMENT) { i++; continue; }
            line.push_back(tokens[i++]);
        }

        if (tokens[i].type == TOK_SEMICOLON) line.push_back(tokens[i++]);
        if (!line.empty()) flush();

        if (tokens[i].type == TOK_LCURLY || tokens[i].type == TOK_RCURLY) {
            line.push_back(tokens[i++]);
            flush();
        }
    }
}

#ifdef DEBUG
int num_of_tokens() {
    return tokens.size();
}
#endif