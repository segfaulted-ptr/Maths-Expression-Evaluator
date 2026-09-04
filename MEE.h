#ifndef MEE_H
#define MEE_H

#include <stdint.h>

typedef enum Type Type_t;
typedef struct Token    Token_t;
typedef struct Tokens   Tokens_LL_t;
typedef struct AST_Node AST_Node_t;
typedef struct MEE_AST  MEE_AST_t;


// Takes a math expression string as input and 
// returns a xx--dynamically allocated AST_Node_t array--xx linked list of Tokens_LL_t type tokens
Tokens_LL_t *tokenize_exp(const char *input_exp);
void print_tokens(Tokens_LL_t *tokens);

// Takes an xx--AST_Node_t array--xx linked list of Tokens_LL_t type as input and 
// returns an MEE_AST_t Abstract Syntax Tree
AST_Node_t *parse_exp(Tokens_LL_t *tokens_list);
void print_AST(AST_Node_t *Exp_AST);

// Takes an MEE_AST_t ast and 
// returns the evaluated result
int64_t evaluate_exp(AST_Node_t *exp_ast);

#endif // MEE_H