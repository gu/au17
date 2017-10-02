#include <iostream>

#include "common.h"
#include "scanner.h"

using namespace std;

struct node {
    bool terminal;
    bool nil;

    struct node* left_child;
    struct node* right_child;

    struct token* token;
};

int parseExpr(struct node* p) {
    if (getCurrentToken()->type == LITERAL_ATOM_ID || getCurrentToken()->type == NUMERIC_ATOM_ID) {
        p->terminal = true;
        p->token = getCurrentToken();
        moveToNext();
    } else if (getCurrentToken()->type == OPEN_PARENTHESIS_ID) {
        moveToNext();

        if (getCurrentToken()->type == CLOSING_PARENTHESIS_ID) {
            p->terminal = true;
            p->nil = true;
        } else {
            p->terminal = false;
            struct node* left_child = (struct node*) malloc(sizeof(struct node));
            left_child->nil = false;
            left_child->terminal = false;
            left_child->left_child = NULL;
            left_child->right_child = NULL;
            left_child->token = NULL;
            p->left_child = left_child;
            if (parseExpr(p->left_child) == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }

            while (getCurrentToken()->type != CLOSING_PARENTHESIS_ID) {
                struct node* right_child = (struct node*) malloc(sizeof(struct node));
                right_child->nil = false;
                right_child->terminal = false;
                right_child->left_child = NULL;
                right_child->right_child = NULL;
                right_child->token = NULL;
                p->right_child = right_child;

                struct node* next_left_child = (struct node*) malloc(sizeof(struct node));
                next_left_child->nil = false;
                next_left_child->terminal = false;
                next_left_child->left_child = NULL;
                next_left_child->right_child = NULL;
                next_left_child->token = NULL;

                p->right_child->left_child = next_left_child;
                if (parseExpr(p->right_child->left_child) == EXIT_FAILURE) {
                    return EXIT_FAILURE;
                }

                p = p->right_child;
            }
            struct node* final_right_child = (struct node*) malloc(sizeof(struct node));
            final_right_child->left_child = NULL;
            final_right_child->right_child = NULL;
            final_right_child->token = NULL;
            final_right_child->terminal = true;
            final_right_child->nil = true;
            p->right_child = final_right_child;

        }
        moveToNext();
    } else {
        if (getCurrentToken()->type == CLOSING_PARENTHESIS_ID) {
            cout << "ERROR: Unexpected closing parenthesis found." << endl;
        } else if (getCurrentToken()->type == ERROR_ID) {
            cout << "ERROR: Invalid token: " << getCurrentToken()->literal << endl;
        } else if (getCurrentToken()->type == EOF_ID) {
            cout << "ERROR: Unexpected EOF." << endl;
        }
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void printSubexpr(struct node* expr_root) {
    if (expr_root->terminal && !expr_root->nil) {
        if (expr_root->token->type == LITERAL_ATOM_ID) {
            cout << expr_root->token->literal;
        } else {
            cout << expr_root->token->numeral;
        }
    } else if (expr_root->nil) {
        cout << "NIL";
    } else {
        cout << "(";
        printSubexpr(expr_root->left_child);
        cout << " . ";
        printSubexpr(expr_root->right_child);
        cout << ")";
    }
}

void printExpr(struct node* expr_root) {
    printSubexpr(expr_root);
    cout << endl;
}

void parseStart() {
    do {
        struct node* expr_root = (struct node*) malloc(sizeof(struct node));
        expr_root->nil = false;
        expr_root->terminal = false;
        expr_root->left_child = NULL;
        expr_root->right_child = NULL;
        expr_root->token = NULL;

        if (parseExpr(expr_root) == EXIT_SUCCESS) {
            printExpr(expr_root);
        } else {
            break;
        }

    } while (getCurrentToken()->type != EOF_ID);
}

int main() {
    if (init() == EXIT_SUCCESS) {
        parseStart();
    }

    return 0;
}


  
