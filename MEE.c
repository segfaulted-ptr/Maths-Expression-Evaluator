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

static void print_tokens(Tokens_LL_t *tokens_list);
static void print_AST(AST_Node_t *ast_node);

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
     * Handle tokenizing only here,
     * every symbol in the input string has its own node
     * the extra nodes (e.g. L_PAREN and R_PAREN nodes) are removed by the parsr
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
    // printf("Printing all the Tokens:\n");
    // print_tokens(tokens_list);
    return tokens_list;
}



static AST_Node_t *new_AST_Node(){

    AST_Node_t *new_node = malloc(sizeof(AST_Node_t));
    if(new_node == NULL){
        perror("Memory Allocation Failed. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    new_node->token.tag = EMPTY;
    new_node->token.value = -1;
    new_node->left = NULL;
    new_node->right = NULL;

    return new_node;
}
static MEE_AST_t *new_AST(){

    MEE_AST_t *AST = malloc(sizeof(MEE_AST_t));
    if(AST == NULL){
        perror("Memory Allocation Failed. Exiting...\n");
        // Do i need to free the tokens_list or not??? !!! well, when i had put this in the parse_exp function lol
        exit(EXIT_FAILURE);
    }

    AST->head = new_AST_Node();

    return AST;
}

// Handling parenthesis
int paren_stack[MAX_PAREN_STACK_SIZE] = {0};
size_t paren_stack_idx = 0;
static void paren_push(Type_t val){
    printf("called paren_push: idx:%zu, val: %d\n", paren_stack_idx, (int)val);
    if(MAX_PAREN_STACK_SIZE <= paren_stack_idx){
        perror("Stack Overflow: Parenthesis Stack Full. Exiting Program...\n");
        exit(EXIT_FAILURE);
    }
    paren_stack[paren_stack_idx++] = val;
}
static Type_t paren_pop(){
    printf("called paren_pop: idx:%zu, val: %d\n", paren_stack_idx, (int)paren_stack[paren_stack_idx-1]);
    if(paren_stack_idx == 0){
        perror("Stack Underflow: Parenthesis Stack Empty. Exiting Program...\n");
        exit(EXIT_FAILURE);
    }
    return paren_stack[--paren_stack_idx];
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
 * @brief Checks the precedence of provided operator with the ast
 * from head to last node
 * @returns The address of the node that points to the node holding the operator with higher 
 * precedence.
 */
static AST_Node_t *check_op_precedence(MEE_AST_t *Exp, Token_t Token){
    if(Exp == NULL){
        // perror or printf? i think printf as no system error can be caused here
        // printf("Expression Error: Expression Empty. Exiting...\n");
        // exit(EXIT_FAILURE);
        return NULL;
    }

    AST_Node_t *prev_node = Exp->head;
    AST_Node_t *curr_node = Exp->head->right;
    // AST_Node_t *curr_node = Exp->head;

    int token_precedence = op_precedence(Token.value);
    
    // return head pointer if it has higher precedence operator
    if(prev_node->token.tag == OPERATOR){
        if(token_precedence < op_precedence(prev_node->token.value)){
            printf("%c has lower precedence than %c\n", (char)prev_node->token.value, (char)Token.value);
            return prev_node;
        }
    }

    // 
    while(curr_node != NULL){
        if(curr_node->token.tag == OPERATOR){
            if(token_precedence < op_precedence(curr_node->token.value)){
                printf("%s[WHILE LOOP]: %c has higher precedence than %c%s\n", RED_CONSOLE_TEXT,
                                                                             (char)curr_node->token.value,
                                                                             (char)Token.value,
                                                                             RESET_CONSOLE_TEXT);
                return prev_node;
            }
        }
        prev_node = curr_node;
        curr_node = curr_node->right;
    }
    return NULL;
}
AST_Node_t *parse_exp(Tokens_LL_t *tokens_list){
    
    /**
     * Suggestion:
     * utilize number_stack and operator_stack to make an AST
     * that naturally follows the precedence convention
    **/
    if(tokens_list == NULL){
        printf("No Tokens provided.\n");
        return NULL;
    }

    MEE_AST_t *Exp_ast = new_AST();
    AST_Node_t *curr_ast_node = Exp_ast->head;
    Tokens_LL_t *curr_tok = tokens_list;


    printf("[PARSER]: Parsing the tokens...\n");

    while(curr_tok != NULL){
        // example: 4 + 2 * 10 + 3 * (5 + 1) - 2
        // example: 4 + 2 * 10 + 3 * 5 + 1 - 2
        switch(curr_tok->token.tag){
            case INTEGER:
                curr_ast_node->token.tag = INTEGER;
                curr_ast_node->token.value = curr_tok->token.value;
                break;
            case OPERATOR:
                // if the precedence of current operator is smaller than the one above it,
                // move the above node to the left of the current node and point head to
                // current node...

                // get the address of the memory the parent node is pointing to,
                // and change it.
                // OR just get the parent node who's left/right node points to the address we want and change it from parent!!! ToT ToT
                AST_Node_t *node_addr = NULL;
                if((node_addr = check_op_precedence(Exp_ast, curr_tok->token)) != NULL){

                    // printf("better than nothing.\n");
                    AST_Node_t *new_ast_node = new_AST_Node();
                    new_ast_node->token.tag = OPERATOR;
                    new_ast_node->token.value = curr_tok->token.value;

                    /**
                     * TODO: Improve this junk here about operator precedence!
                     * 
                     * UPDATE: It is functional now, just refactor it and optimize it if it needs it.
                     */
                    // if the higher precedence operator is at head pointer, change the head pointer
                    // only if the head node and current token has different operators
                    if(node_addr == Exp_ast->head 
                        && ( node_addr->token.tag == OPERATOR 
                            && curr_tok->token.tag == OPERATOR 
                            && node_addr->token.value != curr_tok->token.value)){
                        printf("%s[PARSER]: Changing the Head Pointer%s\n", RED_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
                        new_ast_node->left = node_addr;
                        Exp_ast->head = new_ast_node;
                    }
                    else{
                        printf("%s[PARSER]: Changing the Head Pointer's Right Pointer%s\n", YELLOW_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
                        // printf("The node to reparent:\n");
                        // print_AST(node_addr);
                        // printf("\n");
                        new_ast_node->left = node_addr->right;
                        node_addr->right = new_ast_node;
                    }


                    /** freeing the current node here
                     * because: currently we are traversing through the ast tree,
                     * (firstly this should not be applied as we should be working 'on' it
                     * rather than 'in' it.)
                     * through right side nodes, so here our curr_ast_node is actually a right node
                     * of some parent node, so to break the potential bug of accidently looping over the 
                     * same entries, we first free the current node to free the allocated memory,
                     * then we make an entirely new tree with its head node as the node our tree
                     * and assign it to our current node.
                     * 
                     * UPDATE: i think the problem is still there, as freeing the curr_ast_node does nothing
                     * because we again allocate it some node!!
                     * we have to rethink the whole tree traversal now. 
                     * UPDATE: UPDATE: Maybe no!! because as we have got the source/parent node which points
                     * to the node we want to change, we can then just NULL curr_ast_node and
                     * rearrange the new_ast_node we made to be the parent of the node that the head node
                     * points to.
                    */
                    free(curr_ast_node);
                    curr_ast_node = NULL;
                    // curr_ast_node = new_ast_node;

                    // curr_ast_node->token.tag = OPERATOR;
                    // curr_ast_node->token.value = curr_tok->token.value;

                    // curr_ast_node->left = *node_addr;

                    printf("still promising.\n");
                    // new_ast_node->right = new_AST_Node();
                    // curr_ast_node = new_ast_node->right;
                    curr_ast_node = new_AST_Node();
                    printf("Ohh YESS!!\n");
                }
                else{

                    // printf("Fresh Start\n");
                    // make a new node with current node's value.
                    AST_Node_t *new_ast_node = new_AST_Node();
                    new_ast_node->token.tag = curr_ast_node->token.tag;
                    new_ast_node->token.value = curr_ast_node->token.value;
                    
                    // assign the new node to current node's left node.
                    curr_ast_node->left = new_ast_node;
                    
                    // update the current node.
                    curr_ast_node->token.tag = OPERATOR;
                    curr_ast_node->token.value = curr_tok->token.value;
                    
                    // follow the right node.
                    curr_ast_node->right = new_AST_Node();
                    curr_ast_node = curr_ast_node->right;
                }
                break;

            case L_PAREN:
                // add the parenthesis to stack for parenthesis balancing.
                printf("calling paren_push with tag: %s\n", GET_TAG_STR(curr_tok->token.tag));
                paren_push(L_PAREN);
                if(curr_tok->next == NULL){
                    perror("Expression Error: Missing Closing Parenthesis. Exiting Program...\n");
                    exit(EXIT_FAILURE);
                }
                curr_ast_node->right = parse_exp(curr_tok->next);     // recurse for the inner values of expression
                break;
            case R_PAREN:
                if(paren_pop() != L_PAREN){
                    perror("Expression Error: Missing Opening Parenthesis. Exiting Program...\n");
                    exit(EXIT_FAILURE);
                }
                return Exp_ast->head;     // return from inside the parenthesis

            case EMPTY:
                printf("%s[PARSER]: an EMPTY Node encountered while parsing.%s\n", YELLOW_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
                break;
        }
        // free(curr_tok);
        Tokens_LL_t *temp = curr_tok;
        curr_tok = curr_tok->next;
        free(temp);
        // Building the AST node by node
        printf("%sPrinting AST%s\n", GREEN_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
        print_AST(Exp_ast->head);
        printf("\n\n");
    }
    // printf("Current node value: %lld\n", curr_ast_node->token.value);
    // if(curr_ast_node->left == NULL){
    //     printf("Current node -> left : NULL\n");
    // }
    // if(curr_ast_node->right == NULL){
    //     printf("Current node -> right: NULL\n");
    // }
    // idk why but the ouput value changes when i uncomment these lines
    // input: 1 + 2, output: +, 1, 2; with these commented
    // input: 1 + 2, output: +, 1, 0; with these uncommented
    // WTF???
    // curr_ast_node->left = NULL;
    // curr_ast_node->right = NULL;
    // printf("\n\nPrinting the full AST\n");
    // print_AST(Exp_ast->head);
    return Exp_ast->head;
}


/**
 * @brief Evaluate the arithmetic expression given the operands and operator,
 * otherwise report an error and exit the program!
 */
static uint64_t evaluate_arithmetic(uint64_t l_num, uint64_t r_num, char op){

    printf("%llu %c %llu\n", l_num, op, r_num);
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
        case EMPTY:
        case L_PAREN:
        case R_PAREN:
            printf("Anomaly Encountered. Exiting program\n");
            exit(EXIT_FAILURE);
    }
    printf("Unexpected/Unhandled Tag: %s\n", GET_TAG_STR(curr_node->token.tag));
    exit(EXIT_FAILURE);
}

static void print_tokens(Tokens_LL_t *tokens_list){

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

        case EMPTY:
            printf("%sEMPTY NODE%s\n", YELLOW_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
            break;
        // default:
        //     break;
        }
        curr_node = curr_node->next;
    }
    printf("%sNULL encountered%s\n\n\n", GREEN_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
}

static void print_AST(AST_Node_t *ast_node){

    if(ast_node == NULL){
        // printf("%sNULL%s\n", RED_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
        printf("\n");
        return;
    }

    switch(ast_node->token.tag){

        case INTEGER:
            printf("NODE: \'%lld\'.\n", ast_node->token.value);
            break;

        case OPERATOR:
            printf("NODE: \'%c\'.\n", (char)ast_node->token.value);
        case L_PAREN:
        case R_PAREN:
            break;
        
        case EMPTY:
            printf("%sNODE is EMPTY%s\n", YELLOW_CONSOLE_TEXT, RESET_CONSOLE_TEXT);
    }
    printf("LEFT: ");
    print_AST(ast_node->left);
    // printf("\n");
    printf("RIGHT: ");
    print_AST(ast_node->right);
    // printf("\n");

    return;
}