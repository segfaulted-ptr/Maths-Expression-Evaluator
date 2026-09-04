/**
 * A command line maths expression evaluator in C to study and practice Abstract Syntax Trees.
 * 
 * run with: cd "C:\Users\user\Desktop\Programming Stuff\C\Projects\Maths Expression Evaluator\"; gcc -std=c11 -Wall -Wextra -Wswitch MEE_main.c MEE.c -o MEE;
 */

#include "MEE.h"
#include <string.h>
#include <stdio.h>

#define MAX_EXP_SIZE 256

int main(){

    char input_exp[MAX_EXP_SIZE];
    memset(input_exp, 0, MAX_EXP_SIZE);

    printf("Enter Expression: ");
    fgets(input_exp, MAX_EXP_SIZE+1, stdin);
    input_exp[strcspn(input_exp, "\n")] = '\0';

    Tokens_LL_t *tokens = tokenize_exp(input_exp);
    // printf("Tokenizer Result: \n");
    // print_tokens(tokens);

    AST_Node_t *Exp_AST = parse_exp(tokens);
    // printf("\n\nResult AST: \n");
    // print_AST(Exp_AST);

    /**
     * TODO: Fix the parser function
     * UPDATE: Fixed
     */
    // int64_t result = evaluate_exp(parse_exp(tokenize_exp(input_exp)));
    int64_t result = evaluate_exp(Exp_AST);
    printf("%s = %lld\n", input_exp, result);
    return 0;
}