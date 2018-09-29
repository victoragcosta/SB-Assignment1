// Software básico - Trabalho 02 - Montador

// Includes:
#include <iostream>
#include <string>

// Namespace:
using namespace std;

// Main function:
int main(int argc, char const *argv[]) {

  map <string, int> symbolTable;

  if(argc != 2) {
      cerr << "Incorrect number of files given to function." << endl;
      cerr << "Exiting!" << endl;
      exit(1);
  }

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
