// Software básico - Trabalho 02 - Montador

// Includes:
#include <fstream>
#include <iostream>
#include <list>
#include <regex>
#include <string>
#include <map>

// Enumerations:
typedef enum {
  NORMAL,
  FATAL,
  LEXICAL,
  SYNTACTIC,
  SEMANTIC
} ErrorType;

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
  bool pre_error = false;

  // Streams for assembly and preprocessed files
  fstream asm_file, pre_file;

  // Original file line counter
  unsigned int line = 1;

  // Buffer to hold file lines.
  list <pair<unsigned int, string>> buffer;

  // Table for EQU directives
  map <string, string> aliases_table;
  // Table generated in the first pass to be used in the second pass
  map <string, int> symbols_table;

  // Regular expressions for EQU directive, IF directive and numbers
  regex equ_directive("(.*): EQU (.*)");
  regex if_directive("(.*:)? ?IF (.*)");
  regex number("[0-9]+");
  smatch search_matches;

  string condition, file_name, file_line, formated_line, label, value;

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
        print_error(SEMANTIC, line, "A symbol was aliased twice!");
        pre_error = true;
      }

      else if(!valid_label(label)) {
        print_error(LEXICAL, line, "An invalid symbol was aliased!");
        pre_error = true;
      }

      // The value of an alias should always be a number.

      else if(!regex_match(value, number)) {
        print_error(SYNTACTIC, line, "An invalid alias was chosen!");
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
        print_error(SYNTACTIC, line,
                    "A label was placed before an IF directive!");
        pre_error = true;
      }

      if(condition != "1" && condition != "0") {
        print_error(SYNTACTIC, line,
                    "An invalid condition was given to an IF directive!");
        pre_error = true;
      }

      else if(condition == "0" && !asm_file.eof()) {
        getline(asm_file, file_line);  // Discard the next line;
        line++;
      }

      // If condtion == "1", then we don't need to do anything!

    }

    // Checks if the line is empty or not.

    else if(formated_line != "")
      buffer.push_back(make_pair(line, formated_line));

    line++;

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

  // First pass:

  for(auto const& pair : buffer) {
    // TODO
  }

  // Second pass:


  /* for(auto const& pair : aliases_table) {
    cout << pair.first << ": " << pair.second << endl;
  } */

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

int print_error(ErrorType type, int line, string message) {

  switch (type) {

    case FATAL:
      cerr << "[FATAL ERROR]" << endl;
      break;

    case LEXICAL:
      cerr << "[LEXICAL ERROR] (Line " << line <<  ")" << endl;
      break;

    case SYNTACTIC:
      cerr << "[SYNTACTIC ERROR] (Line " << line <<  ")" << endl;
      break;

    case SEMANTIC:
      cerr << "[SEMANTIC ERROR] (Line " << line <<  ")" << endl;
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
