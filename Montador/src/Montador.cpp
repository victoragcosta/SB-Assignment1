// Software básico - Trabalho 02 - Montador

// Includes:
#include <fstream>
#include <iostream>
#include <list>
#include <regex>
#include <string>
#include <map>
#include "Operation.hpp"

// Enumerations:
typedef enum {
  NORMAL,
  FATAL,
  LEXICAL,
  SYNTACTIC,
  SEMANTIC
} ErrorType;

typedef enum {
  TEXT,
  DATA,
  BSS
} Section;

// Structs:
typedef struct {
  unsigned int bss = 0;
  unsigned int data = 0;
  unsigned int text = 0;
} SectionLines;

// Namespace:
using namespace std;

// Function headers:
bool valid_label(string);
int print_error(ErrorType, int, string);
string format_line(string);
string replace_aliases(string, map <string, string>);

// Main function:
int main(int argc, char const *argv[]) {

  // Error flags:
  bool pre_error = false, pass1_error = false;

  // Streams for assembly and preprocessed files
  fstream asm_file, pre_file;

  // Original file line counter
  unsigned int line_num = 1;

  // Address counter
  unsigned int address;

  // Section indicator
  Section actual_section;

  // Buffer to hold file lines.
  list <pair<unsigned int, string>> buffer;

  // Table for EQU directives
  map <string, string> aliases_table;

  // Tables generated in the first pass to be used in the second pass
  map <string, pair<int,bool>> symbols_table;
  map <string, list<int>> use_table;
  map <string, int> definitions_table;

  // Table with all instructions
  map <string, Operation*> opcodes_table;
  opcodes_table["ADD"]    = new Operation(1,  2, 1);
  opcodes_table["SUB"]    = new Operation(2,  2, 1);
  opcodes_table["MUL"]    = new Operation(3,  2, 1);
  opcodes_table["DIV"]    = new Operation(4,  2, 1);
  opcodes_table["JMP"]    = new Operation(5,  2, 1);
  opcodes_table["JMPN"]   = new Operation(6,  2, 1);
  opcodes_table["JMPP"]   = new Operation(7,  2, 1);
  opcodes_table["JMPZ"]   = new Operation(8,  2, 1);
  opcodes_table["COPY"]   = new Operation(9,  3, 2);
  opcodes_table["LOAD"]   = new Operation(10, 2, 1);
  opcodes_table["STORE"]  = new Operation(11, 2, 1);
  opcodes_table["INPUT"]  = new Operation(12, 2, 1);
  opcodes_table["OUTPUT"] = new Operation(13, 2, 1);
  opcodes_table["STOP"]   = new Operation(14, 1, 0);

  // Regular expressions for EQU directive, IF directive and numbers
  regex equ_directive("^(.*): EQU (.*)$");
  regex if_directive("^(.*:)? ?IF (.*)$");
  regex number("[0-9]+");
  regex section_directive("^SECTION (.*)$");
  regex double_label_regex("^(.*):(.*):.*$");
  regex public_directive("^(.*: )?PUBLIC ([^ ,]+)$");
  regex extern_directive("^(.+): EXTERN$");
  regex label_regex("^(.*): ([^ ]+) ?([^,]*)?,? ?([^,]*)?$");
  smatch search_matches;  // Search results

  string condition, file_name, file_line, formated_line, label, value, operation, argument1, argument2;

  // Struct to hold the number of each section.
  SectionLines sections;

  // Tests if there is program name and file to be assembled
  if(argc != 2) {
      print_error(FATAL, 0, "Incorrect number of arguments given to function!");
      cerr << "Exiting!" << endl;
      exit(1);
  }

  // Gets assembly file name.
  file_name = argv[1];

  // Tries to open file stream.
  asm_file.open(file_name + ".asm", ios::in);

  // Exits if there's no file to be opened.
  if(!asm_file.is_open()) {
    print_error(FATAL, 0, "Couldn't open file: " + file_name + ".asm!");
    cerr << "Exiting!" << endl;
    exit(2);
  }

  // Pre-processing pass:

  cout << "Starting pre-processing pass..." << endl << endl;

  // Iterate over the original code file
  while (getline(asm_file, file_line)) {

    // Removes comments and replaces extra spaces
    formated_line = format_line(file_line);
    // Replaces EQU directives
    formated_line = replace_aliases(formated_line, aliases_table);

    // Checks if the line is an EQU directive.

    if(regex_search(formated_line, search_matches, equ_directive)) {

      label = search_matches[1].str();
      value = search_matches[2].str();

      if(aliases_table.count(label) > 0)  {
        print_error(SEMANTIC, line_num, "A symbol was aliased twice!");
        pre_error = true;
      }

      else if(!valid_label(label)) {
        print_error(LEXICAL, line_num, "An invalid symbol was aliased!");
        pre_error = true;
      }

      // The value of an alias should always be a number.

      else if(!regex_match(value, number)) {
        print_error(SYNTACTIC, line_num, "An invalid alias was chosen!");
        pre_error = true;
      }

      else
        aliases_table[label] = value;

    }

    // Checks if the line is an IF directive.

    else if(regex_search(formated_line, search_matches, if_directive)) {

      label = search_matches[1].str();
      condition = search_matches[2].str();

      // We might get a label before the IF statement.

      if(label != "") {
        print_error(SYNTACTIC, line_num,
                    "A label was placed before an IF directive!");
        pre_error = true;
      }

      if(condition != "1" && condition != "0") {
        print_error(SYNTACTIC, line_num,
                    "An invalid condition was given to an IF directive!");
        pre_error = true;
      }

      else if(condition == "0" && !asm_file.eof()) {
        getline(asm_file, file_line);  // Discard the next line;
        line_num++;
      }

      // If condition == "1", then we don't need to do anything!

    }

    // Checks if the line is empty or not.

    else if(formated_line != "")
      buffer.push_back(make_pair(line_num, formated_line));

    line_num++;

  }

  // Closes the original file.
  asm_file.close();

  // If there was a pre-processing error, exit the program.
  if(pre_error) {
    print_error(FATAL, 0, "Pre-processing pass was not successful!");
    cerr << "Exiting!" << endl;
    exit(3);
  }

  // Creates a new file for the pre-processed output.
  pre_file.open(file_name + ".pre", ios::out);

  // Tests if the file has opened (it should open, but better safe than sorry).
  if(!pre_file.is_open()) {
    print_error(FATAL, 0, "Couldn't create file: " + file_name + ".pre!");
    cerr << "Exiting!" << endl;
    exit(2);
  }

  // Saves each pre-processed line in the .pre file.
  for(auto const& pair : buffer) {
    pre_file << pair.second << endl;
  }

  pre_file.close();

  cout << "Pre-processing pass was successful!" << endl << endl;
  cout << "Starting first compiling pass..." << endl << endl;

  // First pass:

  // Address counter
  address = 0;
  // Iterate over pre-processed file
  for(auto const& pre_line : buffer) {
    // TODO Finish the first pass

    line_num = pre_line.first;
    formated_line = pre_line.second;

    // Section directive:
    if(regex_search(formated_line, search_matches, section_directive)) {

      label = search_matches[1].str();

      if(label == "TEXT") {

        // Only one section declaration can exist!
        if(sections.text != 0) {
          print_error(SEMANTIC, line_num, "The TEXT section was redeclared!");
          pass1_error = true;
        }

        else {
          sections.text = line_num;
          actual_section = Section::TEXT;
        }

      }

      else if(label == "DATA") {

        // The TEXT section has to come first.
        if(sections.text == 0) {
          print_error(SEMANTIC, line_num,
                      "The DATA section was declared before the TEXT section!");
          pass1_error = true;
        }

        // Only one section declaration can exist!
        else if(sections.data != 0) {
          print_error(SEMANTIC, line_num, "The DATA section was redeclared!");
          pass1_error = true;
        }

        else {
          sections.data = line_num;
          actual_section = Section::DATA;
        }

      }

      else if(label == "BSS") {

        // The TEXT section has to come first.
        if(sections.text == 0) {
          print_error(SEMANTIC, line_num,
                      "The BSS section was declared before the TEXT section!");
          pass1_error = true;
        }

        // Only one section declaration can exist!
        else if(sections.bss != 0) {
          print_error(SEMANTIC, line_num, "The BSS section was redeclared!");
          pass1_error = true;
        }

        else {
          sections.bss = line_num;
          actual_section = Section::BSS;
        }

      }

      // Invalid section argument.
      else {
        print_error(SYNTACTIC, line_num, "Invalid SECTION directive!");
        pass1_error = true;
      }

    } // End Section directive
    // Tests for double labels
    else if (regex_search(formated_line, search_matches, double_label_regex)) {
      print_error(SYNTACTIC, line_num, "You cannot have two labels on the same line!");
      pass1_error = true;
    } // End double labels
    // Public
    else if(regex_search(formated_line, search_matches, public_directive)) {
      label = search_matches[1].str();
      argument1 = search_matches[2].str();
      if(label != ""){
        print_error(SYNTACTIC, line_num, "PUBLIC directives must not have labels!");
        pass1_error = true;
      } else if(argument1 == "") {
        print_error(SYNTACTIC, line_num, "PUBLIC directive must have one argument!");
        pass1_error = true;
      } else if(!valid_label(argument1)) {
        print_error(LEXICAL, line_num, "Argument invalid");
        pass1_error = true;
      } else if(definitions_table.count(argument1) > 0) {
        print_error(SEMANTIC, line_num, "Repeated declaration of label "+argument1+" as PUBLIC");
        pass1_error = true;
      } else {
        definitions_table[argument1] = line_num; // line_num as placeholder for error messages
      }
    } // End public
    // Extern
    else if(regex_search(formated_line, search_matches, extern_directive)) {
      // TODO finish extern
    } // End extern
    // Tests for a generic code line
    else if(regex_search(formated_line, search_matches, label_regex)) {
      label = search_matches[1].str();
      operation = search_matches[2].str();
      argument1 = search_matches[3].str();
      argument2 = search_matches[4].str();

      // Adds label to symbols_table if there's one
      if(label != ""){
        if(!valid_label(label)) {
          print_error(SEMANTIC, line_num, "The label is not valid!");
          pass1_error = true;
        }
        else if (symbols_table.count(label) > 0) {
          print_error(SEMANTIC, line_num, "Label was redefined!");
          pass1_error = true;
        } else {
          symbols_table[label] = make_pair(address, false);
        }
      }

      // Tests if it's a valid operation
      if(opcodes_table.count(operation) > 0){
        // Refactor into a function maybe?
        if(argument1 != "" || argument2 != ""){
          for (auto const& iter : symbols_table ) {
            // If it is extern
            if(iter.second.second == true){  
              if(argument1.find(iter.first)){
                use_table[iter.first].push_back(address+1);
              }
              if(argument2.find(iter.first)){
                use_table[iter.first].push_back(address+2);
              }
            }
          }
        }
        address += opcodes_table[operation]->getSize();
      }
      // If not empty, must be a directive
      else if(operation != "") {
        if ((operation == "SPACE" && argument1 == "") || operation == "CONST") {
          address += 1;
        }
        else if(operation == "SPACE" && argument1 != "") {
          address += stoi(argument1);
        }
        else {
          print_error(SYNTACTIC, line_num, "Invalid directive or operation!");
          pass1_error = true;
        }
      }

    } // End code with generic code line
    else {
      print_error(SYNTACTIC, line_num, "Invalid code line!");
      pass1_error = true;
    }

  }
  
  // Copies symbols values to definitions table
  for(auto const& iter : definitions_table) {
    if(symbols_table.count(iter.first) > 0){
      definitions_table[iter.first] = symbols_table[iter.first].first;
    } else {
      print_error(SEMANTIC, iter.second, "Label "+ iter.first +" was never defined!");
      pass1_error = true;
    }
  }
  
  if(sections.text == 0) {
    print_error(FATAL, 0, "No TEXT section found!");
    pass1_error = true;
  }

  // Second pass:


  /* for(auto const& pair : aliases_table) {
    cout << pair.first << ": " << pair.second << endl;
  } */

  // cleans all allocated objects in table
  opcodes_table.clear();

  return 0;

}

// Function implementations:
bool valid_label(string label) {

  bool valid = true;
  regex valid_chars("[a-zA-Z0-9_]+");

  if(label.length() > 50 || !isalpha(label.at(0))
     || !regex_match(label, valid_chars))
    valid = false;

  return valid;

}

int print_error(ErrorType type, int line_num, string message) {

  switch (type) {

    case FATAL:
      cerr << "[FATAL ERROR]" << endl;
      break;

    case LEXICAL:
      cerr << "[LEXICAL ERROR] (Line " << line_num <<  ")" << endl;
      break;

    case SYNTACTIC:
      cerr << "[SYNTACTIC ERROR] (Line " << line_num <<  ")" << endl;
      break;

    case SEMANTIC:
      cerr << "[SEMANTIC ERROR] (Line " << line_num <<  ")" << endl;
      break;

    default:
      cerr << "[ERROR]" << endl;

  }

  cerr << message << endl;
  cerr << endl;

  return 0;

}

string format_line(string line) {

  regex colon(":"), comment(";.*"), first_space("^ "), last_space(" $");
  regex spaces_and_tabs("[ \t]+");
  string formated_line;

  // Removes comments and extra tabs and spaces.

  formated_line = regex_replace(line, comment, "");
  formated_line = regex_replace(formated_line, colon, ": ");
  formated_line = regex_replace(formated_line, spaces_and_tabs, " ");
  formated_line = regex_replace(formated_line, first_space, "");
  formated_line = regex_replace(formated_line, last_space, "");

  // Converts the whole string to uppercase.

  for (auto & c: formated_line)
    c = toupper(c);

  return formated_line;

}

string replace_aliases(string line, map <string, string> aliases_table) {

  list <string> words;
  size_t position = 0;
  string aux, modded_line = "", word;

  // It might be a good idea to keep the original line intact.

  aux = line;

  // First, let's get rid of empty lines! Remember: we already formatted the
  // line, so this corner case catches comments, empty lines, etc...

  if(aux == "")
    return line;

  // Now we split the line along spaces and stores it's words in a list called
  // words.

  /* TODO: create split function for better readability */
  while((position = aux.find(" ")) != string::npos) {
    word = aux.substr(0, position);
    words.push_back(word);
    aux.erase(0, position + 1);
  }

  words.push_back(aux);

  // Now we have to go through each word and replace any alias with it's value,
  // being careful to avoid messing with labels and section statements.
  // Finally, we have to put the whole line back together. Sounds easy, right?

  word = words.front();
  words.pop_front();

  // First let's see if the first word is a label.

  if(word.find(":") != string::npos) {

    // Maybe the line only had a label?

    if(words.empty())
      return line;

    modded_line.append(word + " ");
    word = words.front();
    words.pop_front();

  }

  // Ok, now the next word should be an instruction or a directive. We should
  // not replace those. We might even do a quick check to see if the line is a
  // SECTION or a BEGIN directive or an instruction without parameters.

  if(word == "SECTION" || word == "BEGIN" || words.empty())
    return line;

  else
    modded_line.append(word);

  // Almost there! Now we know that all remaining words are parameters. These
  // should be checked against the aliases_table.

  do {

    word = words.front();
    words.pop_front();

    // Check to see if the word is in the aliases_table.

    for(auto const& pair : aliases_table) {
      if(word == pair.first) {
        word = pair.second;
        break;
      }
    }

    modded_line.append(" " + word);

  } while(!words.empty());

  return modded_line;

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
    TODO Create a function to exit the program.
    TODO Create data structures for the tables needed for the 2 loader passes.
    TODO Implement the first pass, that reads and stores the labels.
    More to come...
*/
