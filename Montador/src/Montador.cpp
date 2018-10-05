// Software básico - Trabalho 02 - Montador

// Includes:
#include <fstream>
#include <iostream>
#include <list>
#include <regex>
#include <string>

// Namespace:
using namespace std;

// Function headers:
bool valid_label(string);
string format_line(string);
string replace_aliases(string, map <string, string>);

// Main function:
int main(int argc, char const *argv[]) {

  fstream asm_file, pre_file;

  unsigned int line = 1;

  map <string, string> aliases_table;
  map <string, int> symbols_table;

  regex equ_directive("(.*): EQU (.*)");
  regex if_directive("(.*: )?IF (.*)");
  regex number("[0-9]+");
  smatch search_matches;

  string condition, file_name, file_line, formated_line, label, value;

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

    formated_line = format_line(file_line);
    formated_line = replace_aliases(formated_line, aliases_table);

    // Checks if the line is an EQU directive.

    if(regex_search(formated_line, search_matches, equ_directive)) {

      label = search_matches[1].str();
      value = search_matches[2].str();

      if(aliases_table.count(label) > 0)  {
        cerr << "[ERROR - Pre-processing] (Line " << line <<  ")" << endl;
        cerr << "A symbol was aliased twice!" << endl;
        cerr << "Symbol: " << label << endl;
        cerr << "Exiting!" << endl;
        exit(3);
      }

      else if(!valid_label(label)) {
        cerr << "[ERROR - Pre-processing] (Line " << line <<  ")" << endl;
        cerr << "An invalid symbol was aliased!" << endl;
        cerr << "Symbol: " << label << endl;
        cerr << "Exiting!" << endl;
        exit(4);
      }

      // I'm not 100% sure that values need to follow the same rules as labels.
      // Let's estimate they are either numbers or labels.

      else if(!regex_match(value, number) && !valid_label(value)) {
        cerr << "[ERROR - Pre-processing] (Line " << line <<  ")" << endl;
        cerr << "An invalid alias was chosen!" << endl;
        cerr << "Alias: " << value << endl;
        cerr << "Exiting!" << endl;
        exit(4);
      }

      else
        aliases_table[label] = value;

    }

    // Checks if the line is an IF directive.

    else if(regex_search(formated_line, search_matches, if_directive)) {

      label = search_matches[1].str();
      condition = search_matches[2].str();

      // We might get a label before the IF statement.

      if(label != "")
        pre_file << label << endl;

      if(condition != "1" && condition != "0") {
        cerr << "[ERROR - Pre-processing] (Line " << line <<  ")" << endl;
        cerr << "An invalid condition was given to an IF directive!" << endl;
        cerr << "Condition: " << condition << endl;
        cerr << "Exiting!" << endl;
        exit(4);
      }

      else if(condition == "0" && !asm_file.eof()) {
        getline(asm_file, file_line);  // Discard the next line;
        line++;
      }

      // If condtion == "1", then we don't need to do anything!

    }

    // Checks if the line is empty or not.

    else if(formated_line != "")
      pre_file << formated_line << endl;

    line++;

  }

  asm_file.close();
  pre_file.close();

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

string format_line(string line) {

  regex comment(";.*"), first_space("^ "), spaces_and_tabs("[ \t]+");
  string formated_line;

  // Removes comments and extra tabs and spaces.

  formated_line = regex_replace(line, comment, "");
  formated_line = regex_replace(formated_line, spaces_and_tabs, " ");
  formated_line = regex_replace(formated_line, first_space, "");

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
    TODO Tweak pre-processor pass label handling. Labels in EQU directives
    should be better examined and labels in IF directives should be examined.
    TODO Create data structures for the tables needed for the 2 loader passes.
    TODO Implement the first pass, that reads and stores the labels.
    More to come...
*/
