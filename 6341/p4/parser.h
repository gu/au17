#include "common.h"
#include "scanner.h"
#include "node.h"
#include <vector>
#include <map>

using namespace std;

int parseExpr(struct node* p);

void printSubexpr(struct node* expr_root);

void printExpr(struct node* expr_root);

void parseStart();

int eval(struct node* input, struct node* output);