#include <iostream>
#include "common.h"
#include "token.h"

using namespace std;

extern string input_line;
extern string::size_type line_idx;
extern struct token* current_token;

int init();

struct token* getCurrentToken();

void moveToNext();

struct token* getNextToken(string* input_string, string::size_type* index);