#include <iostream>
#include "common.h"
#include "token.h"

using namespace std;

static string input_line;
static string::size_type line_idx;
static struct token* current_token;

int init();

struct token* getCurrentToken();

void moveToNext();

struct token* getNextToken(string* input_string, string::size_type* index);