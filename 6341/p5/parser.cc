#include "parser.h"

using namespace std;

string error_message;

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

int evalCond(struct node* in, struct node* out, int expr_type) {
    int ret = EXIT_SUCCESS;
    struct node *be = new node();
    be->type = 0;

    int beRet = eval(in->left_child->left_child, be);
    if (beRet == EXIT_FAILURE) {
        ret = EXIT_FAILURE;
    } else if (be->type != BOOL_ID) {
        error_message = "TYPE ERROR: Expected COND boolean expression to be of type BOOL.";
        ret = EXIT_FAILURE;
    } else {
        int eRet = eval(in->left_child->right_child->left_child, out);
        if (eRet == EXIT_FAILURE) {
            ret = EXIT_FAILURE;
        } else {
            if (expr_type == 0) {
                expr_type = out->type;
            }
            if (expr_type == out->type) {
                if (!in->right_child->nil) {
                    ret = evalCond(in->right_child, out, expr_type);
                }
            } else {
                error_message = "TYPE ERROR: Expected COND expression to have the same type.";
                ret = EXIT_FAILURE;
            }
        }
    }

    return ret;
}

int processCondCaseOne(struct node* in, struct node* out) {
    int ret = EXIT_SUCCESS;

    struct node *be = new node();
    be->type = -1;
    eval2(in->left_child->left_child, be);

    if (be->type == TRUE_ID) {
        int eRet = eval2(in->left_child->right_child->left_child, out);
        if (eRet == EXIT_FAILURE) {
            ret = EXIT_FAILURE;
        }
    } else {
        if (!in->right_child->nil) {
            ret = processCondCaseOne(in->right_child, out);
        } else {
            ret = EXIT_FAILURE;
        }
    }
    return ret;
}

int validateExprsForCond(struct node* in, int* anyNat_count, int* true_count, int* false_count, int* anyBool_count, vector<int>& list_len) {
    int ret = EXIT_SUCCESS;
    struct node *e = new node();
    e->type = -1;

    int eRet = eval2(in->left_child->right_child->left_child, e);
    if (eRet == EXIT_FAILURE) {
        ret = EXIT_FAILURE;
    } else {
        if (e->type == ANYNAT_ID) {
            (*anyNat_count)++;
        } else if (e->type == TRUE_ID) {
            (*true_count)++;
        } else if (e->type == FALSE_ID) {
            (*false_count)++;
        } else if (e->type == ANYBOOL_ID) {
            (*anyBool_count)++;
        } else if (e->type >= 0) {
            list_len.push_back(e->type);
        }
        if (!in->right_child->nil) {
            ret = validateExprsForCond(in->right_child, anyNat_count, true_count, false_count, anyBool_count, list_len);
        }
    }
    return ret;

}

int processCondCaseTwo(struct node* in, struct node* out) {
    int ret = EXIT_SUCCESS;

    int anyNat_count = 0;
    int true_count = 0;
    int false_count = 0;
    int anyBool_count = 0;
    vector<int> list_lens;

    ret = validateExprsForCond(in, &anyNat_count, &true_count, &false_count, &anyBool_count, list_lens);

    if (ret == EXIT_SUCCESS) {
        if (anyNat_count > 0 && true_count == 0 && false_count == 0 && anyBool_count == 0 && list_lens.size() == 0) {
            out->type = ANYNAT_ID;
        } else if (anyNat_count == 0 && true_count > 0 && false_count == 0 && anyBool_count == 0 && list_lens.size() == 0) {
            out->type = TRUE_ID;
        } else if (anyNat_count == 0 && true_count == 0 && false_count > 0 && anyBool_count == 0 && list_lens.size() == 0) {
            out->type = FALSE_ID;
        } else if (anyNat_count == 0 && true_count >= 0 && false_count >= 0 && anyBool_count >= 0 && list_lens.size() == 0) {
            out->type = ANYBOOL_ID;
        } else if (anyNat_count == 0 && true_count == 0 && false_count == 0 && anyBool_count == 0 && list_lens.size() > 0) {
            int minVal = -1;
            for (vector<int>::iterator it = list_lens.begin(); it != list_lens.end(); ++it) {
                if (minVal == -1) {
                    minVal = *it;
                } else if (*it < minVal) {
                    minVal = *it;
                }
            }
            out->type = minVal;
        }
    }

    return ret;
}

int validateBoolExprsForCond(struct node* in, int *anybool_count) {
    int ret = EXIT_SUCCESS;
    struct node *be = new node();
    be->type = -1;

    int beRet = eval2(in->left_child->left_child, be);
    if (beRet == EXIT_FAILURE) {
        ret = EXIT_FAILURE;
    } else {
        if (be->type == ANYBOOL_ID) {
            (*anybool_count)++;
        }
        if (!in->right_child->nil) {
            ret = validateBoolExprsForCond(in->right_child, anybool_count);
        }
    }
    return ret;
}

int evalCond2(struct node* in, struct node* out) {
    int ret = EXIT_SUCCESS;
    int anybool_count = 0;
    
    int validateBoolRet = validateBoolExprsForCond(in, &anybool_count);
    if (validateBoolRet == EXIT_FAILURE) {
        ret = EXIT_FAILURE;
    } else if (anybool_count == 0) {
        int c1ret = processCondCaseOne(in, out);
        if (c1ret == EXIT_FAILURE) {
            ret = EXIT_FAILURE;
        }
    } else if (anybool_count > 0) {
        int c2ret = processCondCaseTwo(in, out);
        if (c2ret == EXIT_FAILURE) {
            ret = EXIT_FAILURE;
        }
    }

    return ret;
}

int eval(struct node* input, struct node* output) {
    int ret = EXIT_SUCCESS;
    if (input->terminal && !input->nil && input->token->type == LITERAL_ATOM_ID && input->token->literal == "T") {
        output->type = BOOL_ID;
    } else if (input->terminal && !input->nil && input->token->type == LITERAL_ATOM_ID && input->token->literal == "F") {
        output->type = BOOL_ID;
    } else if (input->terminal && (input->nil || (input->token->type == LITERAL_ATOM_ID && input->token->literal == "NIL"))) {
        output->type = LISTNAT_ID;
    } else if (input->terminal && input->token && input->token->type == NUMERIC_ATOM_ID) {
        output->type = NAT_ID;
    } else if (input->terminal && input->token && (input->token->type == NUMERIC_ATOM_ID || input->token->type == LITERAL_ATOM_ID)) {
        error_message = "TYPE ERROR: Unexpected Atom";
        ret = EXIT_FAILURE;
    } else if (getListLength(input) >= 2) {
        int listLength = getListLength(input);

        if (input->left_child && input->left_child->terminal && input->left_child->token && input->left_child->token->type == LITERAL_ATOM_ID) {
            if (listLength == 3 && (input->left_child->token->literal == "PLUS" ||
                                    input->left_child->token->literal == "LESS" || 
                                    input->left_child->token->literal == "EQ")) {
                struct node *s1 = new node();
                s1->type = 0;
                struct node *s2 = new node();
                s2->type = 0;

                int s1Ret = eval(input->right_child->left_child, s1);
                int s2Ret = eval(input->right_child->right_child->left_child, s2);
                if (s1Ret == EXIT_FAILURE || s2Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (s1->type != NAT_ID || s2->type != NAT_ID) {
                    error_message = "TYPE ERROR: Expected both subexpressions for " + input->left_child->token->literal +
                            " to be of type Nat, got something else.";
                    ret = EXIT_FAILURE;
                } else {
                    if (input->left_child->token->literal == "PLUS") {
                        output->type = NAT_ID;
                    } else if (input->left_child->token->literal == "EQ") {
                        output->type = BOOL_ID;
                    } else if (input->left_child->token->literal == "LESS") {
                        output->type = BOOL_ID;
                    }
                }
                delete s1;
                delete s2;
            } else if (input->left_child->token->literal == "CONS" && listLength == 3) {
                struct node *s1 = new node();
                s1->type = 0;
                struct node *s2 = new node();
                s2->type = 0;

                int s1Ret = eval(input->right_child->left_child, s1);
                int s2Ret = eval(input->right_child->right_child->left_child, s2);
                if (s1Ret == EXIT_FAILURE || s2Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (s1->type != NAT_ID) {
                    error_message = "TYPE ERROR: Expected left subexpressions for " + input->left_child->token->literal +
                            " to be of type Nat, got something else.";
                    ret = EXIT_FAILURE;
                } else if (s2->type != LISTNAT_ID) {
                    error_message = "TYPE ERROR: Expected right subexpressions for " + input->left_child->token->literal +
                            " to be of type List(Nat), got something else.";
                    ret = EXIT_FAILURE;
                } else {
                    output->type = LISTNAT_ID;
                }
            } else if (listLength == 2 && (input->left_child->token->literal == "CAR" ||
                                            input->left_child->token->literal == "CDR" || 
                                            input->left_child->token->literal == "NULL")) {
                struct node *s1 = new node();
                s1->type = 0;

                int s1Ret = eval(input->right_child->left_child, s1);
                if (s1Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (s1->type != LISTNAT_ID) {
                    error_message = "TYPE ERROR: Expected subexpression for " + input->left_child->token->literal + 
                                    " to be of type List(Nat), got something else.";
                    ret = EXIT_FAILURE;
                } else {
                    if (input->left_child->token->literal == "CAR") {
                        output->type = NAT_ID;
                    } else if (input->left_child->token->literal == "CDR") {
                        output->type = LISTNAT_ID;
                    } else if (input->left_child->token->literal == "NULL") {
                        output->type = BOOL_ID;
                    }
                }
                delete s1;
            } else if (listLength == 2 && (input->left_child->token->literal == "ATOM" ||
                                            input->left_child->token->literal == "INT")) {
                struct node *s1 = new node();
                s1->type = 0;
                int s1Ret = eval(input->right_child->left_child, s1);
                if (s1Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (s1->type == 0) {
                    error_message = "TYPE ERROR: Expected subexpression for " + input->left_child->token->literal +
                                " to of type Nat, Bool, or List(Nat), got something else.";
                } else {
                    output->type = BOOL_ID;
                }
                delete s1;
            } else if (listLength >= 2 && input->left_child->token->literal == "COND") {
                if (validCond(input->right_child)) {
                    ret = evalCond(input->right_child, output, 0);
                } else {
                    error_message = "TYPE ERROR: Invalid COND string";
                    ret = EXIT_FAILURE;
                    return ret;
                }
            } else {
                ret = EXIT_FAILURE;
                error_message = "TYPE ERROR: Unknown operator " + input->left_child->token->literal +
                        " or invalid list length for operator.";
            }
        } else {
            error_message = "TYPE ERROR: Invalid operator.";
            ret = EXIT_FAILURE;
        }
    } else {
        error_message = "TYPE ERROR: Expected list of length >=2, got something else.";
        ret = EXIT_FAILURE;
    }
    return ret;
}

int eval2(struct node* input, struct node* output) {
    int ret = EXIT_SUCCESS;
    if (input->terminal && !input->nil && input->token->type == LITERAL_ATOM_ID && input->token->literal == "T") {
        output->type = TRUE_ID;
    } else if (input->terminal && !input->nil && input->token->type == LITERAL_ATOM_ID && input->token->literal == "F") {
        output->type = FALSE_ID;
    } else if (input->terminal && (input->nil || (input->token->type == LITERAL_ATOM_ID && input->token->literal == "NIL"))) {
        output->type = 0;
    } else if (input->terminal && input->token && input->token->type == NUMERIC_ATOM_ID) {
        output->type = ANYNAT_ID;
    } else if (input->terminal && input->token && (input->token->type == NUMERIC_ATOM_ID || input->token->type == LITERAL_ATOM_ID)) {
        error_message = "EMPTY LIST ERROR: Unexpected Atom";
        ret = EXIT_FAILURE;
    } else if (getListLength(input) >= 2) {
        int listLength = getListLength(input);

        if (input->left_child && input->left_child->terminal && input->left_child->token && input->left_child->token->type == LITERAL_ATOM_ID) {
            if (listLength == 2 && input->left_child->token->literal == "CAR") {
                struct node *s1 = new node();
                s1->type = -1;

                int s1Ret = eval2(input->right_child->left_child, s1);
                if (s1Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (s1->type == 0) {
                    error_message = "EMPTY LIST ERROR: Expected subexpression for " + input->left_child->token->literal + 
                                    " be a List with greater than 0 elements.";
                    ret = EXIT_FAILURE;
                } else {
                    output->type = ANYNAT_ID;
                }
                delete s1;   
            } else if (listLength == 2 && input->left_child->token->literal == "CDR") {
                struct node *s1 = new node();
                s1->type = -1;

                int s1Ret = eval2(input->right_child->left_child, s1);
                if (s1Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (s1->type == 0) {
                    error_message = "EMPTY LIST ERROR: Expected subexpression for " + input->left_child->token->literal + 
                                    " be a List with greater than 0 elements.";
                    ret = EXIT_FAILURE;
                } else {
                    output->type = s1->type - 1;
                }
                delete s1;   
            } else if (input->left_child->token->literal == "CONS" && listLength == 3) {
                struct node *s1 = new node();
                s1->type = -1;
                struct node *s2 = new node();
                s2->type = -1;

                int s1Ret = eval2(input->right_child->left_child, s1);
                int s2Ret = eval2(input->right_child->right_child->left_child, s2);
                if (s1Ret == EXIT_FAILURE || s2Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else {
                    output->type = s2->type + 1;
                }
            } else if (listLength == 2 && input->left_child->token->literal == "NULL") {
                struct node *s1 = new node();
                s1->type = -1;

                int s1Ret = eval2(input->right_child->left_child, s1);
                if (s1Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (s1->type == 0) {
                    output->type = ANYBOOL_ID;
                } else if (s1->type > 0) {
                    output->type = FALSE_ID;
                }
                delete s1;   
            } else if (listLength == 2 && input->left_child->token->literal == "ATOM") {
                struct node *s1 = new node();
                s1->type = -1;

                int s1Ret = eval2(input->right_child->left_child, s1);
                if (s1Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (s1->type == TRUE_ID || s1->type == FALSE_ID || s1->type == ANYBOOL_ID || s1->type == ANYNAT_ID) {
                    output->type = TRUE_ID;
                } else if (s1->type >= 0) {
                    output->type = FALSE_ID;
                }
                delete s1;
            } else if (listLength == 2 && input->left_child->token->literal == "INT") {
                struct node *s1 = new node();
                s1->type = -1;

                int s1Ret = eval2(input->right_child->left_child, s1);
                if (s1Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (s1->type == TRUE_ID || s1->type == FALSE_ID || s1->type == ANYBOOL_ID || s1->type >= 0) {
                    output->type = FALSE_ID;
                } else if (s1->type == ANYNAT_ID) {
                    output->type = TRUE_ID;
                }
                delete s1;
            } else if (listLength == 3 && (input->left_child->token->literal == "PLUS" ||
                                    input->left_child->token->literal == "LESS" || 
                                    input->left_child->token->literal == "EQ")) {
                struct node *s1 = new node();
                s1->type = -1;
                struct node *s2 = new node();
                s2->type = -1;

                int s1Ret = eval2(input->right_child->left_child, s1);
                int s2Ret = eval2(input->right_child->right_child->left_child, s2);
                if (s1Ret == EXIT_FAILURE || s2Ret == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                } else if (input->left_child->token->literal == "EQ" || input->left_child->token->literal == "LESS") {
                    output->type = ANYBOOL_ID;
                } else if (input->left_child->token->literal == "PLUS") {
                    output->type = ANYNAT_ID;
                }
                delete s1;
                delete s2;
            } else if (listLength >= 2 && input->left_child->token->literal == "COND") {
                ret = evalCond2(input->right_child, output);
            } else {
                ret = EXIT_FAILURE;
                error_message = "EMPTY LIST ERROR: Unknown operator " + input->left_child->token->literal +
                        " or invalid list length for operator.";
            }
        } else {
            error_message = "EMPTY LIST ERROR: Invalid operator.";
            ret = EXIT_FAILURE;
        }
    } else {
        error_message = "EMPTY LIST ERROR: Expected list of length >=2, got something else.";
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
            output_root->type = 0;
            if (eval(expr_root, output_root) == EXIT_SUCCESS) {
                struct node* output_root2 = new node();
                output_root2->type = 0;
                if (eval2(expr_root, output_root2) == EXIT_SUCCESS) {
                    printList(expr_root);
                } else {
                    cout << error_message << endl;
                    break;
                }
            } else {
                cout << error_message << endl;
                break;
            }
        } else {
            break;
        }

    } while (getCurrentToken()->type != EOF_ID);
}