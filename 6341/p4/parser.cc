#include "parser.h"

using namespace std;

string error_message;
map<string, struct node*> d;
map<string, struct node*> a;

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
            struct node* left_child = new node();
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
                struct node* right_child = new node();
                right_child->nil = false;
                right_child->terminal = false;
                right_child->left_child = NULL;
                right_child->right_child = NULL;
                right_child->token = NULL;
                p->right_child = right_child;

                struct node* next_left_child = new node();
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
            struct node* final_right_child = new node();
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

int getListLength(struct node* in) {
    int ret = 0;
    if (in->nil) {
        ret = 0;
    } else if (in->right_child && in->right_child->nil) {
        ret = 1;
    } else if (in->right_child && !in->right_child->nil && !in->right_child->right_child) {
        ret = -1;
    } else if (in->right_child && !in->right_child->nil && in->right_child->right_child) {
        int i = getListLength(in->right_child);
        ret = i != -1 ? 1 + i : i;
    }
    return ret;
}

bool validCond(struct node* in) {
    bool ret = true;
    if (in->right_child && in->right_child->nil) {
        ret = in->left_child && getListLength(in->left_child) == 2;
    } else if (in->right_child && !in->right_child->nil) {
        ret = validCond(in->right_child) && getListLength(in->left_child) == 2;
    }
    return ret;
}

int evalCond(struct node* in, struct node* out) {
    int ret = EXIT_SUCCESS;
    struct node *be = new node();
    be->nil = false;
    be->terminal = false;
    be->left_child = NULL;
    be->right_child = NULL;
    be->token = NULL;

    int beRet = eval(in->left_child->left_child, be);
    if (beRet == EXIT_FAILURE) {
        ret = EXIT_FAILURE;
    } else {
        if (!be->nil || !(be->token && be->token->type == LITERAL_ATOM_ID && be->token->literal == "NIL")) {
            int eRet = eval(in->left_child->right_child->left_child, out);
            if (eRet == EXIT_FAILURE) {
                ret = EXIT_FAILURE;
            }
        } else {
            if (!in->right_child->nil) {
                evalCond(in->right_child, out);
            } else {
                error_message = "ERROR: COND string has no truthy conditions.";
                ret = EXIT_FAILURE;
            }
        }
    }

    return ret;
}

bool validName(string name) {
    return name != "T" && name != "NIL" && name != "CAR" && name != "CDR" && name != "CONS" && name != "ATOM" && 
    name != "EQ" && name != "NULL" && name != "INT" && name != "PLUS" && name != "MINUS" && name != "TIMES" && 
    name != "LESS" && name != "GREATER" && name != "COND" && name != "QUOTE" && name != "DEFUN";
}

bool validDefunParamList(struct node* in, vector<string>* formals) {
    if (in->terminal && in->nil) {
        return true;
    } else if (in->left_child->terminal && in->left_child->token && in->left_child->token->type == LITERAL_ATOM_ID && validName(in->left_child->token->literal)) {
        for (vector<string>::iterator it = formals->begin(); it != formals->end(); ++it) {
            if (in->left_child->token->literal == *it) {
                error_message = "Error: DEFUN formal parameters must be unique.";
                return false;
            }
        }
        formals->push_back(in->left_child->token->literal);
        return validDefunParamList(in->right_child, formals);
    } else {
        error_message = "Error: DEFUN formal parameter list is invalid.";
        return false;
    }
}

bool validDefun(struct node* in) {
    vector<string> formals;
    if (!in->left_child->terminal || in->left_child->token->type != LITERAL_ATOM_ID || !validName(in->left_child->token->literal)) {
        error_message = "Error: Invalid DEFUN function name.";
        return false;
    } else if (!validDefunParamList(in->right_child->left_child, &formals)) {
        return false;
    }

    return true;
}

int addpairs(struct node* x, struct node* y) {
    int ret = EXIT_SUCCESS;

    if ((x->terminal && x->nil) || (y->terminal && y->nil)) {
        return ret;
    }

    struct node* x_val = new node();
    x_val->nil = false;
    x_val->terminal = false;
    x_val->left_child = NULL;
    x_val->right_child = NULL;
    x_val->token = NULL;
    if (eval(x->left_child, x_val) == EXIT_FAILURE) {
        ret = EXIT_FAILURE;
    } else {
        string d = y->left_child->token->literal;
        a[d] = x_val;
        ret = addpairs(x->right_child, y->right_child);
    }
    return ret;
}

int eval(struct node* input, struct node* output) {
    int ret = EXIT_SUCCESS;

    if (input->terminal && !input->nil && input->token->type == LITERAL_ATOM_ID && input->token->literal == "T") {
        output->terminal = true;
        output->token = input->token;
    } else if (input->terminal && (input->nil || (input->token->type == LITERAL_ATOM_ID && input->token->literal == "NIL"))) {
        output->nil = true;
        output->terminal = true;
        if (input->token != NULL) {
            output->token = input->token;
        }
    } else if (input->terminal && input->token && input->token->type == NUMERIC_ATOM_ID) {
        output->terminal = true;
        output->token = input->token;
    } else if (input->terminal && input->token && input->token->type == LITERAL_ATOM_ID && a.find(input->token->literal) != a.end()) {
        struct node* v = a.find(input->token->literal)->second;
        output->terminal = v->terminal;
        output->token = v->token;
    } else if (getListLength(input) > 0) {
        int listLength = getListLength(input);

        if (input->left_child && input->left_child->terminal && input->left_child->token && input->left_child->token->type == LITERAL_ATOM_ID) {
            if (listLength == 3 && (input->left_child->token->literal == "PLUS" ||
                                    input->left_child->token->literal == "MINUS" ||
                                    input->left_child->token->literal == "TIMES" ||
                                    input->left_child->token->literal == "LESS" ||
                                    input->left_child->token->literal == "GREATER")) {
                struct node *s1 = new node();
                s1->nil = false;
                s1->terminal = false;
                s1->left_child = NULL;
                s1->right_child = NULL;
                s1->token = NULL;
                struct node *s2 = new node();
                s2->nil = false;
                s2->terminal = false;
                s2->left_child = NULL;
                s2->right_child = NULL;
                s2->token = NULL;

                int s1Ret = eval(input->right_child->left_child, s1);
                int s2Ret = eval(input->right_child->right_child->left_child, s2);
                if (s1Ret == EXIT_FAILURE || s2Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (!s1->terminal || s1->token == NULL || s1->token->type != NUMERIC_ATOM_ID ||
                           !s2->terminal || s2->token == NULL || s2->token->type != NUMERIC_ATOM_ID) {
                    error_message = "ERROR: Expected both subexpressions for " + input->left_child->token->literal +
                            " to be numeric atoms, got something else.";
                    ret = EXIT_FAILURE;
                } else {
                    output->terminal = true;
                    output->token = new token();
                    if (input->left_child->token->literal == "PLUS") {
                        output->token->type = NUMERIC_ATOM_ID;
                        output->token->numeral = s1->token->numeral + s2->token->numeral;
                    } else if (input->left_child->token->literal == "MINUS") {
                        output->token->type = NUMERIC_ATOM_ID;
                        output->token->numeral = s1->token->numeral - s2->token->numeral;
                    } else if (input->left_child->token->literal == "TIMES") {
                        output->token->type = NUMERIC_ATOM_ID;
                        output->token->numeral = s1->token->numeral * s2->token->numeral;
                    } else if (input->left_child->token->literal == "LESS") {
                        output->token->type = LITERAL_ATOM_ID;
                        if (s1->token->numeral < s2->token->numeral) {
                            output->token->literal = "T";
                        } else {
                            output->nil = true;
                            output->token->literal = "NIL";
                        }
                    } else if (input->left_child->token->literal == "GREATER") {
                        output->token->type = LITERAL_ATOM_ID;
                        if (s1->token->numeral > s2->token->numeral) {
                            output->token->literal = "T";
                        } else {
                            output->nil = true;
                            output->token->literal = "NIL";
                        }
                    }
                }
                delete s1;
                delete s2;
            } else if (input->left_child->token->literal == "EQ" && listLength == 3) {
                struct node *s1 = new node();
                s1->nil = false;
                s1->terminal = false;
                s1->left_child = NULL;
                s1->right_child = NULL;
                s1->token = NULL;
                struct node *s2 = new node();
                s2->nil = false;
                s2->terminal = false;
                s2->left_child = NULL;
                s2->right_child = NULL;
                s2->token = NULL;

                int s1Ret = eval(input->right_child->left_child, s1);
                int s2Ret = eval(input->right_child->right_child->left_child, s2);
                if (s1Ret == EXIT_FAILURE || s2Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (!s1->terminal || s1->token == NULL ||
                           (s1->token->type != NUMERIC_ATOM_ID && s1->token->type != LITERAL_ATOM_ID) ||
                           !s2->terminal || s2->token == NULL ||
                           (s2->token->type != NUMERIC_ATOM_ID && s2->token->type != LITERAL_ATOM_ID)) {
                    error_message = "ERROR: Expected both subexpressions for " + input->left_child->token->literal +
                                    " to be atoms, got something else.";
                    ret = EXIT_FAILURE;
                } else {
                    output->terminal = true;
                    output->token = new token();
                    output->token->type = LITERAL_ATOM_ID;
                    if (s1->token->type == NUMERIC_ATOM_ID && s2->token->type == NUMERIC_ATOM_ID &&
                        s1->token->numeral == s2->token->numeral) {
                        output->token->literal = "T";
                    } else if (s1->token->type == LITERAL_ATOM_ID && s2->token->type == LITERAL_ATOM_ID &&
                               s1->token->literal == s2->token->literal) {
                        output->token->literal = "T";
                    } else {
                        output->nil = true;
                        output->token->literal = "NIL";
                    }
                }
                delete s1;
                delete s2;
            } else if (listLength == 2 && (input->left_child->token->literal == "ATOM" ||
                                           input->left_child->token->literal == "INT" ||
                                           input->left_child->token->literal == "NULL")) {

                struct node *s1 = new node();
                s1->nil = false;
                s1->terminal = false;
                s1->left_child = NULL;
                s1->right_child = NULL;
                s1->token = NULL;
                int s1Ret = eval(input->right_child->left_child, s1);
                if (s1Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else {
                    output->terminal = true;
                    output->token = new token();
                    output->token->type = LITERAL_ATOM_ID;
                    if (input->left_child->token->literal == "ATOM") {
                        if (s1->terminal && s1->token &&
                            (s1->token->type == NUMERIC_ATOM_ID || s1->token->type == LITERAL_ATOM_ID)) {
                            output->token->literal = "T";
                        } else {
                            output->nil = true;
                            output->token->literal = "NIL";
                        }
                    } else if (input->left_child->token->literal == "INT") {
                        if (s1->terminal && s1->token && s1->token->type == NUMERIC_ATOM_ID) {
                            output->token->literal = "T";
                        } else {
                            output->nil = true;
                            output->token->literal = "NIL";
                        }
                    } else if (input->left_child->token->literal == "NULL") {
                        if (s1->terminal && (s1->nil || (s1->token && s1->token->type == LITERAL_ATOM_ID &&
                            s1->token->literal == "NIL"))) {
                            output->token->literal = "T";
                        } else {
                            output->nil = true;
                            output->token->literal = "NIL";
                        }
                    }
                }
                delete s1;
            } else if (listLength == 2 && (input->left_child->token->literal == "CAR" ||
                                           input->left_child->token->literal == "CDR")) {

                struct node *s1 = new node();
                s1->nil = false;
                s1->terminal = false;
                s1->left_child = NULL;
                s1->right_child = NULL;
                s1->token = NULL;
                int s1Ret = eval(input->right_child->left_child, s1);
                if (s1Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (s1->terminal && s1->token && (s1->token->type == NUMERIC_ATOM_ID || s1->token->type == LITERAL_ATOM_ID)) {
                    ret = EXIT_FAILURE;
                    error_message = "ERROR: Expected subexpression for " + input->left_child->token->literal +
                                    " to be list, got an atom.";
                } else if (!s1->left_child || !s1->right_child) {
                    ret = EXIT_FAILURE;
                    error_message = "ERROR: Expected subexpression for " + input->left_child->token->literal +
                                    " to be list, got something else.";
                } else {
                    if (input->left_child->token->literal == "CAR") {
                        output->nil = s1->left_child->nil;
                        output->terminal = s1->left_child->terminal;
                        output->left_child = s1->left_child->left_child;
                        output->right_child = s1->left_child->right_child;
                        output->token = s1->left_child->token;
                    } else if (input->left_child->token->literal == "CDR") {
                        output->nil = s1->right_child->nil;
                        output->terminal = s1->right_child->terminal;
                        output->left_child = s1->right_child->left_child;
                        output->right_child = s1->right_child->right_child;
                        output->token = s1->right_child->token;
                    }
                }
                delete s1;
            } else if (listLength == 3 && input->left_child->token->literal == "CONS") {
                struct node *s1 = new node();
                s1->nil = false;
                s1->terminal = false;
                s1->left_child = NULL;
                s1->right_child = NULL;
                s1->token = NULL;
                struct node *s2 = new node();
                s2->nil = false;
                s2->terminal = false;
                s2->left_child = NULL;
                s2->right_child = NULL;
                s2->token = NULL;

                int s1Ret = eval(input->right_child->left_child, s1);
                int s2Ret = eval(input->right_child->right_child->left_child, s2);
                if (s1Ret == EXIT_FAILURE || s2Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else {
                    output->left_child = s1;
                    output->right_child = s2;
                }
            } else if (listLength == 2 && input->left_child->token->literal == "QUOTE") {
                output->nil = input->right_child->left_child->nil;
                output->terminal = input->right_child->left_child->terminal;
                output->left_child = input->right_child->left_child->left_child;
                output->right_child = input->right_child->left_child->right_child;
                output->token = input->right_child->left_child->token;
            } else if (listLength >= 2 && input->left_child->token->literal == "COND") {
                if (validCond(input->right_child)) {
                    ret = evalCond(input->right_child, output);
                } else {
                    error_message = "ERROR: Invalid COND string";
                    ret = EXIT_FAILURE;
                    return ret;
                }
            } else if (listLength == 4 && input->left_child->token->literal == "DEFUN") {
                if (validDefun(input->right_child)) {
                    string name = input->right_child->left_child->token->literal;
                    struct node *def = new node();
                    def->left_child = input->right_child->right_child->left_child;
                    def->right_child = input->right_child->right_child->right_child->left_child;
                    d[name] = def;
                    output->terminal = true;
                    struct token* fn_tok = new token();
                    fn_tok->type = LITERAL_ATOM_ID;
                    fn_tok->literal = name;
                    output->token = fn_tok;
                } else {
                    ret = EXIT_FAILURE;
                }
            } else if (d.find(input->left_child->token->literal) != d.end()) {
                struct node* fn_frame = d.find(input->left_child->token->literal)->second;
                int formal_param_length = getListLength(fn_frame->left_child);
                if (formal_param_length != listLength - 1) {
                    error_message = "Error: Incorrect number of parameters for function: " + input->left_child->token->literal + ".";
                    ret = EXIT_FAILURE;
                    return ret;
                }

                if (addpairs(input->right_child, fn_frame->left_child) == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                    return ret;
                }

                struct node* s1 = new node();
                s1->nil = false;
                s1->terminal = false;
                s1->left_child = NULL;
                s1->right_child = NULL;
                s1->token = NULL;
                int s1Ret = eval(fn_frame->right_child, s1);
                if (s1Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                    return ret;
                }

                output->terminal = s1->terminal;
                output->nil = s1->nil;
                output->right_child = s1->right_child;
                output->left_child = s1->left_child;
                output->token = s1->token;
            } else {
                ret = EXIT_FAILURE;
                error_message = "ERROR: Unknown operator " + input->left_child->token->literal +
                        " or invalid list length for operator.";
            }
        } else {
            error_message = "ERROR: Invalid operator.";
            ret = EXIT_FAILURE;
        }
    } else {
        error_message = "ERROR: Expected list of length > 0, got something else.";
        ret = EXIT_FAILURE;
    }
    return ret;
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

void printSubList(struct node* root) {
    if (root->terminal && !root->nil) {
        if (root->token->type == LITERAL_ATOM_ID) {
            cout << root->token->literal;
        } else {
            cout << root->token->numeral;
        }
    } else if (root->nil) {
        cout << "NIL";
    } else {

        cout << "(";
        while (!root->nil && root->right_child) {
            printSubList(root->left_child);
            root = root->right_child;
            if (root->right_child) {
                cout << " ";
            }
        }

        if (root->nil) {
            cout << ")";
        } else {
            cout << " . ";
            printSubList(root);
            cout << ")";
        }
    }
}

void printList(struct node* root) {
    printSubList(root);
    cout << endl;
}

void parseStart() {
    do {
        struct node* expr_root = new node();
        expr_root->nil = false;
        expr_root->terminal = false;
        expr_root->left_child = NULL;
        expr_root->right_child = NULL;
        expr_root->token = NULL;

        if (parseExpr(expr_root) == EXIT_SUCCESS) {
            struct node* output_root = new node();
            output_root->nil = false;
            output_root->terminal = false;
            output_root->left_child = NULL;
            output_root->right_child = NULL;
            output_root->token = NULL;
            if (eval(expr_root, output_root) == EXIT_SUCCESS) {
                printList(output_root);
                a.clear();
            } else {
                cout << error_message << endl;
                break;
            }
        } else {
            break;
        }

    } while (getCurrentToken()->type != EOF_ID);
}