#include <iostream>
#include <string>
#include <vector>

#include "common.h"

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
    ret.type = EOF_ID;
    return ret;
  }

  char c = (*input_string)[*index];

  if (c == ' ') {
    (*index)++;
    while (*index < (*input_string).length() && (*input_string)[*index] == ' ') {
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
    while (*index < (*input_string).length()) {
      c = (*input_string)[*index];
      if (c < 48 || (c > 57 && c < 65) || c > 90) {
        break;
      }
      if (ret.type == NUMERIC_ATOM_ID && (c > 64 && c < 91)) {
        ret.type = ERROR_ID;
      }
      (*index)++;
    }
    int end_idx = *index;
    string token = (*input_string).substr(start_idx, end_idx - start_idx);
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
  int open_parenthesis_count = 0;
  int close_parenthesis_count = 0;
  int numeric_atom_count = 0;
  int numeric_atom_sum = 0;
  int literal_atom_count = 0;
  vector<string> literal_atom_collection;

  while (getline(cin, input_line)) {

    int line_idx = 0;

    token_type tok = getNextToken(&input_line, &line_idx);
    while (tok.type != ERROR_ID && tok.type != EOF_ID) {
      if (tok.type == OPEN_PARENTHESIS_ID) {
        open_parenthesis_count++;
      } else if (tok.type == CLOSING_PARENTHESIS_ID) {
        close_parenthesis_count++;
      } else if (tok.type == NUMERIC_ATOM_ID) {
        numeric_atom_count++;
        numeric_atom_sum += tok.numeral;
      } else if (tok.type == LITERAL_ATOM_ID) {
        literal_atom_count++;
        literal_atom_collection.push_back(tok.literal);
      }

      tok = getNextToken(&input_line, &line_idx);
    }

    if (tok.type == ERROR_ID) {
      cout << "ERROR: Invalid token " << tok.literal << endl;
      return 0;
    }
  }

  cout << "LITERAL ATOMS: " << literal_atom_count;
  for (vector<string>::iterator it = literal_atom_collection.begin(); it != literal_atom_collection.end(); ++it) {
    cout << ", " << *it;
  }
  cout << endl;
  cout << "NUMERIC ATOMS: " << numeric_atom_count << ", " << numeric_atom_sum << endl;
  cout << "OPEN PARENTHESES: " << open_parenthesis_count << endl;
  cout << "CLOSING PARENTHESES: " << close_parenthesis_count << endl;

  return 0;
}


  
