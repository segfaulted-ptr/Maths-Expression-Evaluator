/**
 * A command line maths expression evaluator in C to study and practice Abstract Syntax Trees.
 * 
 * run with: cd "C:\Users\user\Desktop\Programming Stuff\C\Projects\Maths Expression Evaluator\"; gcc -std=c11 -Wall -Wextra -Wswitch MEE_main.c MEE.c -o MEE;
 */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "MEE.h"

#define MAX_EXP_SIZE 256

int main(){

    /**
     * Handles: 
     *      - arithmetic operations: * + / -
     * Improvements:
     *      - trigonometric operations: sin, cos, tan, sec, cosec, cot, arc_sin, arc_cos, arc_tan, arc_sec, arc_cosec, arc_cot 
     *      - extended arithmetic operations: ^, %, !, (), √(sqrt), PI, Euler's Constant, PHI
     *      - logarithmic functions: ln, log, 10^x
     */

    char input_exp[MAX_EXP_SIZE];
    uint8_t exit_flag = 0;
    while(!exit_flag){

        memset(input_exp, 0, MAX_EXP_SIZE);
        
        printf("Enter Expression (h for help)\n>> ");
        fgets(input_exp, MAX_EXP_SIZE, stdin);
        input_exp[strcspn(input_exp, "\n")] = '\0';

        // exit command
        if(!strncmp(input_exp, "exit", 4) || !strncmp(input_exp, "EXIT", 4)){
            exit_flag = 1;
            continue;
        }
        // clear console screen command
        if(!strncmp(input_exp, "clear", 5)){
            printf("\033[H\033[0J");
            continue;
        }
        // help menu
        if(!strncmp(input_exp, "help", 4) || !strncmp(input_exp, "h", 1)){
            printf("\nUsage: \n");
            printf("Enter any arithmetic expression.\n");
            printf("E.g. \n\t2 + 4\n\t3 * 4 + 1\n\t100 - 1\n\t69 + 420\n");
            printf("\nHelpful text commands: \n");
            printf("clear\t-\tclears console screen\n");
            printf("help \t-\tshows help menu\n");
            printf("exit \t-\texit the program\n");
            printf("----------------------------------------");
            printf("\n\n");
            continue;
        }

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
    }
    return 0;
}