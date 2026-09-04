#include "MEE.h"
#include <stdlib.h>
#include <stdio.h>

// FOR DEBUG USAGE
#define GET_TAG_STR(tag) (tag==INTEGER? "INTEGER"                               \
                            : tag==OPERATOR? "OPERATOR"                         \
                                : tag==L_PAREN || tag==R_PAREN? "PARENTHESIS"   \
                                    : "UNKNOWN TAG")

#define RED_CONSOLE_TEXT "\033[31m"
#define GREEN_CONSOLE_TEXT "\033[32m"
#define YELLOW_CONSOLE_TEXT "\033[33m"
#define RESET_CONSOLE_TEXT "\033[0m"

#define MAX_PAREN_STACK_SIZE 16

enum Type{
    EMPTY = -1,
    INTEGER,
    OPERATOR,
    L_PAREN,
    R_PAREN,
    TOK_END,
};


struct Token{   // Token_t
    Type_t tag;
    int64_t value;
};

struct Tokens{  // Tokens_LL_t
    Token_t token;
    Tokens_LL_t *next;
};

/**
 * struct Tokens_LL_t{
 *      Tokens_node *head_node;
 *      Tokens_node *tail_node;
 *      uint64_t length;
 * };
 */


struct AST_Node{ // AST_Node_t
    Token_t token;
    AST_Node_t *left;
    AST_Node_t *right;
};

struct MEE_AST{
    AST_Node_t *head;
};

void print_tokens(Tokens_LL_t *tokens_list);
void print_AST(AST_Node_t *Exp_AST);

static Tokens_LL_t *new_Token_Node(Type_t tag, int64_t value){
    Tokens_LL_t *node = malloc(sizeof(Tokens_LL_t));
    if(node == NULL){
        perror("Memory Allocation Failed.\n");
        exit(EXIT_FAILURE);
    }
    node->token.tag = tag;
    node->token.value = value;
    node->next = NULL;
    return node;
}
/**
 * @brief Pushes a new node to the end of the list
 */
static void push_Token_Node(Tokens_LL_t **tokens_list, Tokens_LL_t *new_node){

    if(*tokens_list == NULL){
        *tokens_list = new_node;
        return;
    }
    Tokens_LL_t *curr_node = *tokens_list;
    while(curr_node->next != NULL){
        curr_node = curr_node->next;
    }
    curr_node->next = new_node;
    return;
}
static Token_t pop_Token_Node(Tokens_LL_t **tokens_list){
    if(tokens_list == NULL){
        printf("List Pointer is NULL.\n");
        exit(EXIT_FAILURE);
    }
    if(*tokens_list == NULL){
        printf("Token List empty: No Node Available to POP.\n");
        exit(EXIT_FAILURE);
        // return NULL;
    }
    Token_t curr_node = (*tokens_list)->token;
    Tokens_LL_t *node_to_free = *tokens_list;
    *tokens_list = ((*tokens_list)->next);
    free(node_to_free);
    return curr_node;
}
static Token_t peek_Token_Node(Tokens_LL_t *tokens_list){
    if(tokens_list == NULL){
        printf("Token List empty: No Node Available to PEEK.\n");
        exit(EXIT_FAILURE);
    }
    Token_t curr_node = tokens_list->token;
    return curr_node;
}
/**
 * TODO: Improve the Tokens Linked List to not make an extra node at the end.
 * 
 * UPDATE: The new_Token_Node() function seems to have fixed the issue of making an extra node the tokenizer loop was making lol
 * 
 * UPDATED: UPDATE: This maybe because I use an EMPTY=-1 tag for empty token nodes, so it may be just discarding that node, 
 *                  which is nice too.. for now.
 */
Tokens_LL_t *tokenize_exp(const char *input_exp){
    /**
     * Handles: 
     *      - arithmetic operations: * + / - 
     *      - parenthesis
     * Improvements:
     *      - trigonometric operations: sin, cos, tan, sec, cosec, cot, arc_sin, arc_cos, arc_tan, arc_sec, arc_cosec, arc_cot 
     *      - extended arithmetic operations: ^, %, !, 
     */
    Tokens_LL_t *tokens_list = NULL;
    Tokens_LL_t *curr_node = NULL;

    printf("[LEXER]: Tokenizing input...\n");
    // example1: 4 + 2 * 10 + 3 * (5 + 1)
    // example2: 4 + 2 * 10 + 3 * (5 + 1) - 2
    while(*input_exp != '\0'){

        // if not one of the required symbols, then continue
        if(
            !(*input_exp == '+' || *input_exp == '-' || *input_exp == '*' || *input_exp == '/' 
            || *input_exp == '(' || *input_exp == ')' || (*input_exp >= '0' && *input_exp <= '9'))
        ){
            input_exp++;
            continue;
        }

        // Number
        if(*input_exp >= '0' && *input_exp <= '9'){
            int64_t number = 0;
            do{
                number = number*10 + *input_exp - '0';
                input_exp++;
            }while(*input_exp!='\0' && *input_exp >= '0' && *input_exp <= '9');
            curr_node = new_Token_Node(INTEGER, number);
        }
        // Operator
        else{
            switch (*input_exp)
            {
                case '+':
                case '-':
                case '*':
                case '/':
                    curr_node = new_Token_Node(OPERATOR, *input_exp);
                    break;
                case '(':
                    curr_node = new_Token_Node(L_PAREN, *input_exp);
                    break;
                case ')':
                    curr_node = new_Token_Node(R_PAREN, *input_exp);
                    break;
                
                default:
                    printf("ILLEGAL SYMBOL ENCOUNTERED: \'%c\'\n", *input_exp);
                    exit(EXIT_FAILURE);
            }
        }
        // explicitely check the null terminator again because
        // we used the while loop in switch
        if(*input_exp != '\0'){
            input_exp++;
        }
        // printf("TAG: %s\n", GET_TAG_STR(curr_node->token.tag));

        push_Token_Node(&tokens_list, curr_node);
    }
    curr_node = new_Token_Node(TOK_END, -1);
    push_Token_Node(&tokens_list, curr_node);
    // print_tokens(tokens_list);
    // printf("Printing all the Tokens:\n");
    return tokens_list;
}



static AST_Node_t *new_AST_Node(Type_t tag, int64_t value){

    AST_Node_t *new_node = malloc(sizeof(AST_Node_t));
    if(new_node == NULL){
        perror("Memory Allocation Failed. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    new_node->token.tag = tag;
    new_node->token.value = value;
    new_node->left = NULL;
    new_node->right = NULL;

    return new_node;
}
static int op_precedence(char op){
    switch(op){
        case '+':   return 1;
        case '-':   return 2;
        case '*':
        case '/':   return 3;
    }
    printf("%sOperator Precedence error!%s\n\n", RED_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
    return -1;
}
/**
 * @brief Parses the tokens using pratt parsing
*/
static AST_Node_t *pratt_parser(Tokens_LL_t **tokens, float prev_binding_power){

    /**
     * init LHS TREE variable
     * ITERATE TOKENS
     *      INTEGER
     *          PUSH TO LHS
     *      OPERATOR
     *          CHECK prev binding pow
     *          PEEK NEXT TOK
     *          RET
     *          or 
     *          POP LHS and PUSH TO CURR AST NODE
     *  
     * RECURSE
     */
    AST_Node_t *LHS = NULL;
    Token_t token;
    float curr_binding_power = 0.0;
    while((*tokens)->token.tag != TOK_END){
        // printf("\nTokens left: \n");
        // print_tokens(*tokens);
        token = pop_Token_Node(tokens);
        // printf("Token POPed: %s, val: %lld\n", GET_TAG_STR(token.tag), token.value);
        switch(token.tag){
            case INTEGER:
                LHS = new_AST_Node(token.tag, token.value);
                Token_t token_peek = peek_Token_Node(*tokens);
                if(token_peek.tag == TOK_END){
                    printf("peek Tokens is end.\n");
                    return LHS;
                }
                curr_binding_power = op_precedence(token_peek.value);
                if(curr_binding_power < prev_binding_power){
                    printf("Current binding power is less than previous.\n");
                    return LHS;
                }
                break;
            case OPERATOR:
                curr_binding_power = op_precedence(token.value);
                // if(curr_binding_power > prev_binding_power){
                    AST_Node_t *new_node = new_AST_Node(token.tag, token.value);
                    new_node->left = LHS;
                    LHS = new_node;
                    printf("Going Right\n");
                    LHS->right = pratt_parser(tokens, curr_binding_power);
                    printf("returned from right\n");
                    // prev_binding_power = curr_binding_power;
                // }
                // printf("Current AST: \n");
                // print_AST(LHS);
                break;
            case TOK_END:
                printf("Tokens END!\n");
                break;
            case L_PAREN:
            case R_PAREN:
                printf("UNHANDLED CASE: PARENTHESIS.\n");
                exit(EXIT_FAILURE);
            case EMPTY:
                printf("IMPOSSIBLE CASE: EMPTY NODE.\n");
                exit(EXIT_FAILURE);
        }
    }
    return LHS;
}
/**
 * @brief Uses Pratt Parsing to parse the tokens into an AST
 * @return Returns an AST of maths tokens
 */
AST_Node_t *parse_exp(Tokens_LL_t *tokens){

    AST_Node_t *parsed_tokens = NULL;
    float curr_binding_power = 0.0;

    parsed_tokens = pratt_parser(&tokens, curr_binding_power);

    return parsed_tokens;
}

/**
 * @brief Evaluate the arithmetic expression given the operands and operator,
 * otherwise report an error and exit the program!
 */
static uint64_t evaluate_arithmetic(uint64_t l_num, uint64_t r_num, char op){

    printf("\'%llu\' %c \'%llu\'\n", l_num, op, r_num);
    switch(op){
        case '+':   return l_num + r_num;
        case '-':   return l_num - r_num;
        case '*':   return l_num * r_num;
        case '/':   return l_num / r_num;
    }
    printf("Wrong Operator: %c\n", op);
    exit(EXIT_FAILURE);
}
/**
 * TODO: implement the evaluation function
 * UPDATE: Implemented!
 */
int64_t evaluate_exp(AST_Node_t *exp_ast){

    AST_Node_t *curr_node = exp_ast;

    switch(curr_node->token.tag){

        case OPERATOR:
            uint64_t left_num = 0;
            uint64_t right_num = 0;
            // get the left number
            if(curr_node->left->token.tag == OPERATOR){
                left_num = evaluate_exp(curr_node->left);
            } else{
                left_num = curr_node->left->token.value;
            }
            // get the right number
            if(curr_node->right->token.tag == OPERATOR){
                right_num = evaluate_exp(curr_node->right);
            } else{
                right_num = curr_node->right->token.value;
            }
            char operator = curr_node->token.value;
            free(curr_node);
            return evaluate_arithmetic(left_num, right_num, operator);
        case INTEGER:
            uint64_t number = curr_node->token.value;
            free(curr_node);
            return number;
        case TOK_END:
            printf("Tokens Ended.\n");
            break;
        case EMPTY:
        case L_PAREN:
        case R_PAREN:
            printf("Anomaly Encountered. Exiting program\n");
            exit(EXIT_FAILURE);
    }
    printf("Unexpected/Unhandled Tag: %s\n", GET_TAG_STR(curr_node->token.tag));
    exit(EXIT_FAILURE);
}

void print_tokens(Tokens_LL_t *tokens_list){

    Tokens_LL_t *curr_node = tokens_list;
    while(curr_node != NULL){
        // printf("\033[33mNo NULL encountered\033[0m\n");
        switch (curr_node->token.tag)
        {
        case INTEGER:
            printf("TAG: INT, VAL: %lld\n", curr_node->token.value);
            break;
        
        case OPERATOR:
            printf("TAG: OPERATOR, VAL: %c\n", (char)curr_node->token.value);
            break;

        case L_PAREN:
            printf("TAG: L_PAREN, VAL: %c\n", (char)curr_node->token.value);
            break;

        case R_PAREN:
            printf("TAG: R_PAREN, VAL: %c\n", (char)curr_node->token.value);
            break;

        case TOK_END:
            printf("TAG: TOK_END, VAL: %lld\n", curr_node->token.value);
            break;

        case EMPTY:
            printf("%sEMPTY NODE%s\n", RED_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
            break;
        // default:
        //     break;
        }
        curr_node = curr_node->next;
    }
    printf("%sNULL encountered%s\n\n\n", GREEN_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
}

void print_AST(AST_Node_t *Exp_AST){

    if(Exp_AST == NULL){
        // printf("%sNULL%s\n", RED_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
        printf("\n");
        return;
    }

    switch(Exp_AST->token.tag){

        case INTEGER:
            printf("NODE: \'%lld\'\n", Exp_AST->token.value);
            break;

        case OPERATOR:
            printf("NODE: \'%c\'\n", (char)Exp_AST->token.value);
        case L_PAREN:
        case R_PAREN:
        case TOK_END:
            break;
        
        case EMPTY:
            printf("%sNODE is EMPTY%s\n", YELLOW_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
    }
    if(Exp_AST->left != NULL){
        printf("LEFT: ");
        print_AST(Exp_AST->left);
    }
    // printf("\n");
    if(Exp_AST->right != NULL){
        printf("RIGHT: ");
        print_AST(Exp_AST->right);
    }
    // printf("\n");

    return;
}