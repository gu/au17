#include "parser.h"

using namespace std;

token_type getNextToken(string* input_string, string::size_type* index) {
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