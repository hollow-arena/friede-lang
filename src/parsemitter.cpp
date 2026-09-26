#include "parsemitter.h"
#include <vector>
#include <string>
#include <optional>
#include <unordered_set>
#include <unordered_map>

#ifdef DEBUG
#include <iostream>
#endif

using namespace std;

struct CppToken {
    TokenType type;
    KeywordType keyword;
    string text;
    int line;
    int col;

    CppToken(Token t) {
        type = t.type;
        keyword = t.keyword;
        line = t.line;
        col = t.col;
        text = string(t.ptr, t.len);
    }
};

struct TypeDesc {
    KeywordType type;
    string userObjectName;

    TypeDesc(KeywordType kt, string uon = "") : type(kt), userObjectName(move(uon)) {}
};

struct Field {
    TypeDesc td;
    size_t offset;
};

struct Param {
    string name;
    TypeDesc type;
};

struct Function {
    TypeDesc returnType;
    vector<Param> params;
};

static vector<vector<CppToken>> grouped_tokens;

// key = name of enum, values = hashmaps of enum value name -> enum value
static unordered_map<string, unordered_map<string, size_t>> enums;

// key = name of struct/class, values = hashmaps of fields -> offsets
static unordered_map<string, unordered_map<string, Field>> structs;
static unordered_map<string, unordered_map<string, Field>> classes;

// key = function name, value = vector of type + names
static unordered_map<string, Function> functions;

// This will be more work to reason through
// static unordered_set<string> variables;

#ifdef DEBUG
static int token_count = 0;
#endif

void push_one() {
    grouped_tokens.emplace_back();
}

void send_token(Token t) {

    #ifdef DEBUG
    token_count++;
    #endif

    if (t.type == TOK_COMMENT) return;

    if (t.type == TOK_LCURLY || t.type == TOK_RCURLY) {

        if (!grouped_tokens.back().empty()) grouped_tokens.emplace_back();

        grouped_tokens.back().emplace_back(t);
        grouped_tokens.emplace_back();
    }

    else if (t.type == TOK_EOF) {
        if (!grouped_tokens.back().empty()) grouped_tokens.emplace_back();
        grouped_tokens.back().emplace_back(t);
    }

    else if (t.type == TOK_SEMICOLON) {
        grouped_tokens.back().emplace_back(t);
        grouped_tokens.emplace_back();
    }

    else grouped_tokens.back().emplace_back(t);
}

void collect_custom_names() {

    size_t i = 0;
    int curly_counter = 0;

    while (i < grouped_tokens.size()) {
        if (grouped_tokens[i].size() == 2 && grouped_tokens[i][1].type == TOK_IDENT) {
            if (curly_counter) {
                // TODO: Add error log for no nested enum/struct/class definitions
            }
            else {
                switch (grouped_tokens[i][0].keyword) {
                    // If enum, can populate values on the fly
                    case KEYWORD_ENUM: {

                        string enum_name = grouped_tokens[i][1].text;
                        auto it = enums.find(enum_name);
                        if (it != enums.end()) {
                            // TODO: Add error log for duplicate enum name
                        }

                        else if (i + 3 < grouped_tokens.size()
                            && grouped_tokens[i+1].size() == 1 && grouped_tokens[i+1][0].type == TOK_LCURLY
                            && grouped_tokens[i+3].size() == 1 && grouped_tokens[i+3][0].type == TOK_RCURLY) {

                                auto& values = enums.try_emplace(move(enum_name)).first->second;
                                const auto& enum_body = grouped_tokens[i + 2];
                                size_t j = 0;

                                while (j < enum_body.size()) {
                                    const auto& name_tok = enum_body[j];
                                    if (name_tok.type == TOK_IDENT) {
                                        bool success = values.try_emplace(string(name_tok.text), values.size()).second;
                                        if (!success) {
                                            // TODO: Add error handling that enum values must not have duplicate names
                                            break;
                                        }
                                    }
                                    else {
                                        // TODO: Add error handling that enum values must be valid variable names
                                        break;
                                    }
                                    if (j + 1 < enum_body.size() && enum_body[j + 1].type != TOK_COMMA) {
                                        // TODO: Add error handling that enum values must be comma separated
                                        break;
                                    }
                                    j += 2;
                                }

                                i += 3;
                        }
                        else {
                            // TODO: Add error log for invalid enum format
                        }
                        break;
                    }
                    // TODO: Add classes and structs
                    default: break;
                }
            }
        }
        else if (grouped_tokens[i][0].type == TOK_LCURLY) {
            curly_counter++;
        }
        else if (grouped_tokens[i][0].type == TOK_RCURLY) {
            curly_counter--;
            if (curly_counter < 0) {
                // TODO: Add error log for too many right curly braces
                curly_counter = 0;
            }
        }
        i++;
    }
}

void build_expr(vector<Token>& line) {
    
}

void build_all_exprs() {

    #ifdef DEBUG
    for (auto& line : grouped_tokens) {
        for (const auto& t : line) {
            cout << t.text << " ";
        }
        cout << endl;
    }
    #endif

    // First pass, get all function / struct / class names, anything user defined
    collect_custom_names();

    // Debug, display enums and their values
    #ifdef DEBUG
    for (const auto& [enum_name, enum_values] : enums) {
        cout << enum_name << ":" << endl;
        for (const auto& [value_name, value] : enum_values) {
            cout << value_name << " = " << value << endl;
        }
        cout << endl;
    }
    #endif

    // TODO: Finish passes and all the actual important stuff
}

#ifdef DEBUG
int num_of_tokens() {
    return token_count;
}
#endif