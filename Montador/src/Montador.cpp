// Software básico - Trabalho 02 - Montador

// Includes:
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

// Namespace:
using namespace std;

// Main function:
int main(int argc, char const *argv[]) {

  fstream asm_file, pre_file;

  map <string, string> aliases_table;
  map <string, int> symbol_table;

  regex comment(";.*"), first_space("^ "), spaces(" +") , tabs("\t+");

  string file_name, file_line;

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
    file_line = regex_replace(file_line, comment, "");
    file_line = regex_replace(file_line, tabs, " ");
    file_line = regex_replace(file_line, spaces, " ");
    file_line = regex_replace(file_line, first_space, "");
    // cout << file_line << endl;
    if(file_line != "")
      pre_file << file_line << endl;
  }

  asm_file.close();
  pre_file.close();

  return 0;

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
