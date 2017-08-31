#include <iostream>
#include <string>

#include "common.h"
// #include "file1.h"

using namespace std;

struct token_type {
  int type;

  string literal;
  int numeral;
};

token_type getNextToken(string* input_string, int* index) {
  token_type ret;

  // Escape if the current position is at the end of an input line.
  if (*index >= (*input_string).length()) {
    // cout << "in escape" << endl; // dev
    ret.type = EOF_ID;
    return ret;
  }

  char c = (*input_string)[*index];
  // cout << "ASCII: " << int(c) << endl; // dev

  if (c == ' ') {
    (*index)++;
    while (*index < (*input_string).length() && (*input_string)[*index] == ' ') {
      // c = (*input_string)[*index]; // dev
      // cout << "ASCII: " << int(c) << endl; // dev
      (*index)++;
    }
    ret.type = *index == (*input_string).length() ? EOF_ID : 0;
  } else if (c == '(') {
    (*index)++;
    ret.type = OPEN_PARENTHESIS_ID;
  } else if (c == ')') {
    (*index)++;
    ret.type = CLOSING_PARENTHESIS_ID;
  } else if ((c > 64 && c < 91) || (c > 47 && c < 58)) {
    ret.type = (c > 64 && c < 91) ? LITERAL_ATOM_ID : NUMERIC_ATOM_ID;
    int start_idx = *index;
    (*index)++;
    while (*index < (*input_string).length() && (*input_string)[*index] != ' ') {
      c = (*input_string)[*index];
      if (ret.type == NUMERIC_ATOM_ID && (c > 64 && c < 91)) {
        ret.type = ERROR_ID;
      }
      (*index)++;
    }
    int end_idx = *index;
    string token = (*input_string).substr(start_idx, end_idx);
    if (ret.type == NUMERIC_ATOM_ID) {
      ret.numeral = stoi(token);
    } else {
      ret.literal = token;
    }
  } else {
    ret.type = ERROR_ID;
    ret.literal = "Unknown character";
  }

  return ret;
}

int main() {
  string input_line;

  while (getline(cin, input_line)) {

    int line_idx = 0;
    int line_length = input_line.length();
    cout << "INPUT: " << input_line << " LENGTH: " << line_length << endl;

    token_type tok = getNextToken(&input_line, &line_idx);
    cout << "TOKEN TYPE: " << tok.type << endl;
    while (tok.type != ERROR_ID && tok.type != EOF_ID) {


      tok = getNextToken(&input_line, &line_idx);
      cout << "TOKEN TYPE: " << tok.type << endl;
    }
    
    cout << endl;
  }

  return 0;
}


  
