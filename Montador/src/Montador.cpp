// Software básico - Trabalho 02 - Montador

// Includes:
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

// Namespace:
using namespace std;

// Function headers:
bool valid_label(string);
string format_line(string);

// Main function:
int main(int argc, char const *argv[]) {

  fstream asm_file, pre_file;

  map <string, string> aliases_table;
  map <string, int> symbols_table;

  regex equ_directive("([a-zA-Z0-9_]+): EQU ([a-zA-Z0-9_]+)");
  smatch regex_matches;

  string file_name, file_line, formated_line, label, value;

  if(argc != 2) {
      cerr << "Incorrect number of files given to function." << endl;
      cerr << "Exiting!" << endl;
      exit(1);
  }

  file_name = argv[1];

  asm_file.open(file_name + ".asm", ios::in);

  if(!asm_file.is_open()) {
    cerr << "Couldn't open file: " << file_name << ".asm" << endl;
    cerr << "Exiting!" << endl;
    exit(2);
  }

  // Pre-processing pass:

  pre_file.open(file_name + ".pre", ios::out);

  while (getline(asm_file, file_line)) {

    formated_line = format_line(file_line);   // Remove comments and extras.

    if(regex_search(formated_line, regex_matches, equ_directive)) {

      label = regex_matches[1].str();
      value = regex_matches[2].str();

      if(valid_label(label))
        aliases_table[label] = value;

      else {
        cerr << "[ERROR - Pre-processing] An invalid symbol was aliased: ";
        cerr << label << endl;
        cerr << "Exiting!" << endl;
        exit(3);
      }

    }

    else if(formated_line != "")
      pre_file << formated_line << endl;

  }

  asm_file.close();
  pre_file.close();

  for(auto const& pair : aliases_table) {
    cout << pair.first << ': ' << pair.second << endl;
  }

  return 0;

}

// Function implementations:
bool valid_label(string label) {

  bool valid = true;

  if(label.length() > 50 || !isalpha(label.at(0)))
    valid = false;

  return valid;

}

string format_line(string line) {

  regex comment(";.*"), first_space("^ "), spaces_and_tabs("[ \t]+");
  string formated_line;

  formated_line = regex_replace(line, comment, "");
  formated_line = regex_replace(formated_line, spaces_and_tabs, " ");
  formated_line = regex_replace(formated_line, first_space, "");

  return formated_line;

}

// Adicional notes:

/*
Directive list:

* Section: 1 operand, size 0. Delimits sections.
* Space: 1 operand, size 1. Reserves one or more uninitialized memory spaces.
* Const: 1 operand, size 1. Reserves one memory space to store a constant.
* Public: 0 operands, size 0. Indicates that a label is public.
* Equ: 1 operand, size 0. Creates a text alias for a symbol.
* If: 1 operand, size 0. If the operand value is 1, include the next code line.
* Extern: 0 operands, size 0. Indicates that a label is an extern symbol.
* Begin: 0 operands, size 0. Start a module.
* End: 0 operands, size 0. Ends a module.

*/

/* To-do list:
    TODO Create file manipulation code. This should handle the files given by
    arguments, handle the output files and handle possible errors.
    TODO Implement pre-processor pass. This pass should remove comments and
    evaluate the EQU and IF directives. The results from this pass need to be
    outputed to a .pre file.
    TODO Create data structures for the tables needed for the 2 loader passes.
    TODO Implement the first pass, that reads and stores the labels.
    More to come...
*/
