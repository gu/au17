#include <iostream>
#include <string>
#include <vector>

#include "common.h"
#include "parser.h"

using namespace std;

int main() {
  string input_line;
  int open_parenthesis_count = 0;
  int close_parenthesis_count = 0;
  int numeric_atom_count = 0;
  int numeric_atom_sum = 0;
  int literal_atom_count = 0;
  vector<string> literal_atom_collection;

  while (getline(cin, input_line)) {

    string::size_type line_idx = 0;

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


  
