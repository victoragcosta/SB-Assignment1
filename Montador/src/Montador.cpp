// Software básico - Trabalho 02 - Montador

// Includes:
#include <fstream>
#include <iostream>
#include <list>
#include <regex>
#include <string>
#include <map>
#include "Operation.hpp"

#define DEBUG false

// Enumerations:
typedef enum {
  NORMAL,
  FATAL,
  LEXICAL,
  SYNTACTIC,
  SEMANTIC
} ErrorType;

typedef enum {
  JUMP,
  SPACE,
  CONST,
  EXTERN
} LabelType;

typedef enum {
  BEGIN,
  TEXT,
  DATA,
  BSS,
  END
} Section;

// Structs:
typedef struct {
  unsigned int text = 0;
  unsigned int bss = 0;
  unsigned int data = 0;
} SectionLines;

// Namespace:
using namespace std;

// Function headers:
bool valid_label(string);
int clean_up(void);
int exit_program(int);
int print_error(ErrorType, int, string);
list <string> split_string(string, string);
string format_line(string);
string replace_aliases(string, map <string, string>);

// Global variables:
map <string, Operation*> opcodes_table;

// Main function:
int main(int argc, char const *argv[]) {

  // Error flags:
  bool pre_error = false, pass1_error = false, pass2_error = false;

  // Module flags:
  bool module_start = false, module_end = false, valid_module = false;

  // Streams for assembly and preprocessed files
  fstream asm_file, obj_file, pre_file;

  int const_value, i, offset;

  // Machine code output
  list <int> machine_code, relative_addresses;

  // Buffer to hold file lines.
  list <pair<unsigned int, string>> buffer;

  // List of operands given in a line:
  list <string> operand_list;

  // Table for EQU directives
  map <string, string> aliases_table;

  // Tables generated in the first pass to be used in the second pass
  map <string, pair <int, LabelType>> symbols_table;
  map <string, list<int>> use_table;
  map <string, int> definitions_table;
  map <string, string> constant_table;

  // Regular expressions:
  regex command("^(?:(.*): ?)?([^ :]*)(?: (.*))?$");
  regex equ_directive("^(.*): EQU(?: (.*))?$");
  regex hex_number("0X[0-9A-F]+");
  regex if_directive("^(.*:)? ?IF(?: (.*))?$");
  regex label_and_offset("^([^\\+]*)(?:\\+([0-9]+))?$");
  regex positive_number("[0-9]+");
  regex section_directive("^(?:(.*): ?)?SECTION(?: (.*))?$");
  regex signed_number("-?[0-9]+");
  regex double_label_regex("^(.*):(.*):.*$");
  regex public_directive("^(.*: )?PUBLIC ([^ ,]+)$");
  regex extern_directive("^(.+): EXTERN$");
  regex label_regex("^(.*): ?([^ ]*)(?: (.*))?$");
  regex command_regex("^([^ :]*)(?: (.*))?$");

  smatch search_matches, search_matches2;  // Search results.

  // Label type indicator
  LabelType label_type;

  // Section indicator
  Section actual_section;

  // Section positions
  SectionLines sections;

  // Strings:
  string arg_label, argument1, argument2, condition, file_name, file_line;
  string formated_line, label, operation, operands, value;

  // Counters
  unsigned int address, line_num, operand_num;

  // Instruction data initialization.
  opcodes_table["ADD"] = new Operation(1,  2, 1);
  opcodes_table["SUB"] = new Operation(2,  2, 1);
  opcodes_table["MULT"] = new Operation(3,  2, 1);
  opcodes_table["DIV"] = new Operation(4,  2, 1);
  opcodes_table["JMP"] = new Operation(5,  2, 1);
  opcodes_table["JMPN"] = new Operation(6,  2, 1);
  opcodes_table["JMPP"] = new Operation(7,  2, 1);
  opcodes_table["JMPZ"] = new Operation(8,  2, 1);
  opcodes_table["COPY"] = new Operation(9,  3, 2);
  opcodes_table["LOAD"] = new Operation(10, 2, 1);
  opcodes_table["STORE"] = new Operation(11, 2, 1);
  opcodes_table["INPUT"] = new Operation(12, 2, 1);
  opcodes_table["OUTPUT"] = new Operation(13, 2, 1);
  opcodes_table["STOP"] = new Operation(14, 1, 0);

  // Tests if there is program name and file to be assembled
  if(argc != 2) {
      print_error(FATAL, 0, "Incorrect number of arguments given to function!");
      exit_program(1);
  }

  // Gets assembly file name.
  file_name = argv[1];

  // Tries to open file stream.
  asm_file.open(file_name + ".asm", ios::in);

  // Exits if there's no file to be opened.
  if(!asm_file.is_open()) {
    print_error(FATAL, 0, "Couldn't open file: " + file_name + ".asm!");
    exit_program(2);
  }

  // Pre-processing pass:

  cout << "::Starting pre-processing pass..." << endl << endl;

  // Start line counter:
  line_num = 1;

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

      // Empty EQU statement.

      else if(value == "") {
        print_error(SYNTACTIC, line_num, "An EQU directive needs an alias!");
        pre_error = true;
      }

      // The value of an alias should always be a number.

      else if(!regex_match(value, signed_number)) {
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

      // Empty IF statement.

      else if(condition == "") {
        print_error(SYNTACTIC, line_num,
                    "No condition was given to an IF directive!");
        pre_error = true;
      }

      else if(condition != "1" && condition != "0") {
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
    exit_program(4);
  }

  // Creates a new file for the pre-processed output.
  pre_file.open(file_name + ".pre", ios::out);

  // Tests if the file has opened (it should open, but better safe than sorry).
  if(!pre_file.is_open()) {
    print_error(FATAL, 0, "Couldn't create file: " + file_name + ".pre!");
    exit_program(3);
  }

  // Saves each pre-processed line in the .pre file.
  for(auto const& pair : buffer) {
    pre_file << pair.second << endl;
  }

  pre_file.close();

  cout << "::Pre-processing pass was successful!" << endl << endl;
  cout << "::Starting first compiling pass..." << endl << endl;

  // First pass:

  // TODO Refactor the first pass to utilize the operand_list in address
  // calculations and to stop checking things that are better left to the
  // second pass.

  address = 0;  // Reset address counter.
  actual_section = Section::BEGIN; // Reset section counter.

  // Iterate over pre-processed file
  for(auto const& pair : buffer) {

    line_num = pair.first;
    formated_line = pair.second;

    // Section directive:
    if(regex_search(formated_line, search_matches, section_directive)) {

      if(DEBUG){
        cout << line_num << " SECTION" << endl;
      }

      label = search_matches[1].str();
      argument1 = search_matches[2].str();

      if(label != "") {
        print_error(SEMANTIC, line_num,
                    "SECTION directives cannot have labels!");
        pass1_error = true;
      }

      else if(argument1 == "TEXT") {

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

      else if(argument1 == "DATA") {

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

      else if(argument1 == "BSS") {

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

      // Empty section directive.

      else if(label == "") {
        print_error(SYNTACTIC, line_num, "Empty SECTION directive!");
        pass1_error = true;
      }

      // Invalid section argument.

      else {
        print_error(SYNTACTIC, line_num, "Invalid SECTION directive!");
        pass1_error = true;
      }

    } // End Section directive
    // Tests for double labels
    else if (regex_search(formated_line, search_matches, double_label_regex)) {
      if(DEBUG){
        cout << line_num << " double label" << endl;
      }
      print_error(SYNTACTIC, line_num, "You cannot have two labels on the same line!");
      pass1_error = true;
    } // End double labels
    // Public
    else if(regex_search(formated_line, search_matches, public_directive)) {
      if(DEBUG){
        cout << line_num << " PUBLIC" << endl;
      }
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
      if(DEBUG){
        cout << line_num << " EXTERN" << endl;
      }
      label = search_matches[1].str();

      if(label == "") {
        print_error(SYNTACTIC, line_num, "EXTERN directive must have a label!");
        pass1_error = true;
      }
      else if(symbols_table.count(label) > 0) {
        print_error(SEMANTIC, line_num, "Label redefined!");
        pass1_error = true;
      }
      else {
        symbols_table[label] = make_pair(address, LabelType::EXTERN);
      }
    } // End extern
    // Tests for a generic code line with label
    else if(regex_search(formated_line, search_matches, label_regex)) {
      if(DEBUG) {
        cout << line_num << " LABEL" << endl;
      }
      label = search_matches[1].str();
      operation = search_matches[2].str();
      operands = search_matches[3].str();

      if(operands == "")
        operand_list.clear();

      else
        operand_list = split_string(", ", operands);

      // Adds label to symbols_table if there's one
      if(label == "") {
        print_error(SYNTACTIC, line_num, "Empty label!");
        pass1_error = true;
      }

      else {
        if(!valid_label(label)) {
          print_error(SEMANTIC, line_num, "The label is not valid!");
          pass1_error = true;
        }
        else if (symbols_table.count(label) > 0) {
          print_error(SEMANTIC, line_num, "Label was redefined!");
          pass1_error = true;
        } else {

          if(actual_section == Section::DATA) {
            label_type = LabelType::CONST;
            constant_table[label] = argument1;
          }

          else if(actual_section == Section::BSS)
            label_type = LabelType::SPACE;

          else
            label_type = LabelType::JUMP;


          symbols_table[label] = make_pair(address, label_type);
        }
      }

      // Tests if it's a valid operation
      if(opcodes_table.count(operation) > 0){

        // Refactor into a function maybe?
        if(!operand_list.empty()){

          offset = 1; // The first argument has a single offset.

          for (auto const& operand : operand_list) {

            if(regex_search(operand, search_matches2, label_and_offset)) {

              arg_label = search_matches2[1].str();

              if(symbols_table.count(arg_label) > 0) {
                if(symbols_table[arg_label].second == LabelType::EXTERN)
                  use_table[arg_label].push_back(address+offset);
              }
            }

            offset++;

          }

        }

        address += opcodes_table[operation]->getSize();

      }

      // If not empty, must be a directive
      else if((operation == "SPACE" && operand_list.empty()) ||
              operation == "CONST") {
        address += 1;
      }

      else if(operation == "SPACE" && !operand_list.empty()) {

        argument1 = operand_list.front();

        if(regex_match(argument1, positive_number))
          address += stoi(argument1);

        // But what if the argument for SPACE isn't a positive number?
        else {
          print_error(SYNTACTIC, line_num,
                      "An invalid operand was given to a SPACE directive!");
          pass1_error = true;
        }

      }

      else if(operation != "BEGIN" && operation != "END" && operation != "") {
        print_error(SYNTACTIC, line_num,
                    "Couldn't find any instruction/directive with that name!");
        pass1_error = true;
      }

    } // End code with generic code line with label
    // Tests for a generic code line without label
    else if(regex_search(formated_line, search_matches, command_regex)) {
      if(DEBUG) {
        cout << line_num << " COMMAND" << endl;
      }
      operation = search_matches[1].str();
      operands = search_matches[2].str();

      if(operands == "")
        operand_list.clear();

      else
        operand_list = split_string(", ", operands);

      // Tests if it's a valid operation
      if(opcodes_table.count(operation) > 0){

        // Refactor into a function maybe?
        if(!operand_list.empty()){

          offset = 1; // The first argument has a single offset.

          for (auto const& operand : operand_list) {

            if(regex_search(operand, search_matches2, label_and_offset)) {

              arg_label = search_matches2[1].str();

              if(symbols_table.count(arg_label) > 0) {
                if(symbols_table[arg_label].second == LabelType::EXTERN)
                  use_table[arg_label].push_back(address+offset);
              }
            }

            offset++;

          }

        }

        address += opcodes_table[operation]->getSize();

      }

      // If not empty, must be a directive
      else if((operation == "SPACE" && operand_list.empty()) ||
              operation == "CONST") {
        address += 1;
      }

      else if(operation == "SPACE" && !operand_list.empty()) {

        argument1 = operand_list.front();

        if(regex_match(argument1, positive_number))
          address += stoi(argument1);

        // But what if the argument for SPACE isn't a positive number?
        else {
          print_error(SYNTACTIC, line_num,
                      "An invalid operand was given to a SPACE directive!");
          pass1_error = true;
        }

      }

      else if(operation != "BEGIN" && operation != "END") {
        print_error(SYNTACTIC, line_num,
                    "Couldn't find any instruction/directive with that name!");
        pass1_error = true;
      }

    } // End code with generic code line without label

    else {
      if (DEBUG) {
        cout << line_num << " ELSE" << endl;
      }
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

  // Prints tables for debug reasons
  if(DEBUG) {

    cout << endl;
    cout << " Symbols table:" << endl;
    cout << " Symbol | address | extern" << endl;
    for(auto const& iter : symbols_table) {
      cout << " " << iter.first << " | " << iter.second.first << " | " << iter.second.second << endl;
    }
    cout << endl;

    cout << " Definitions table:" << endl;
    cout << " Symbol | address" << endl;
    for (auto const& iter : definitions_table)
    {
      cout << " " << iter.first << " | " << iter.second << endl;
    }
    cout << endl;

    cout << " Use table:" << endl;
    cout << " Symbol | address " << endl;
    for (auto const& iter : use_table)
    {
      for(auto const& iter2 : iter.second) {
        cout << " " << iter.first << " | " << iter2 << endl;
      }
    }
    cout << endl;
  }

  if(pass1_error) {
    print_error(FATAL, 0, "First compiling pass was not successful!");
    exit_program(5);
  }

  cout << "::First compiling pass was successful!" << endl << endl;
  cout << "::Starting second compiling pass..." << endl << endl;

  // Second pass:

  address = 0;  // Restart the address counter. It will be needed.
  actual_section = Section::BEGIN; // Reset the section variable.

  for(auto const& pair : buffer) {

    // TODO Finish second pass.

    line_num = pair.first;
    formated_line = pair.second;

    if(actual_section == Section::END) {
      print_error(SEMANTIC, line_num,
                  "No commands can be given after the END directive.");
      pass2_error = true;
      break;
    }

    // Valid command (This should always match...)!
    if(regex_search(formated_line, search_matches, command)) {

      label = search_matches[1].str();  // Some commands NEED labels...
      operation = search_matches[2].str();
      operands = search_matches[3].str();

      if(operands == "")
        operand_list.clear();

      else
        operand_list = split_string(", ", operands);

      operand_num = operand_list.size();

      // Line contains an instruction:
      if(opcodes_table.count(operation) > 0) {

        machine_code.push_back(opcodes_table[operation]->getOpcode());
        address++;

        // Invalid section.
        if(actual_section != Section::TEXT) {
          print_error(SYNTACTIC, line_num,
                      "An instruction was used outside the TEXT SECTION!");
          pass2_error = true;
        }

        // Invalid number of arguments.
        else if(opcodes_table[operation]->getNParameters() != operand_num) {
          print_error(SYNTACTIC, line_num,
                      "An invalid number of operands was given!");
          pass2_error = true;
        }

        // Valid operation.
        else {

          // Operand analysis.
          for(auto const& operand : operand_list) {

            if(regex_search(operand, search_matches2, label_and_offset)) {

              arg_label = search_matches2[1].str();

              if(search_matches2[2].str() == "")
                offset = 0;

              else
                offset = stoi(search_matches2[2].str());

              // Ok, this next bit of code is a bit tricky.
              // We first check if the label given as an argument exist.
              // If it does, we get it's address: symbols_table[label].first.
              // And add that to the offset given.
              // The result is stored as machine code.
              // Finally, we update the machine code address.

              if(symbols_table.count(arg_label) > 0) {
                machine_code.push_back(symbols_table[arg_label].first + offset);
                relative_addresses.push_back(address);
                address++;
              }

              else {
                print_error(SEMANTIC, line_num,
                            "A missing label was used as an operand!");
                pass2_error = true;
              }

            }

            else {
              print_error(SYNTACTIC, line_num,
                          "An invalid operand format was used!");
              pass2_error = true;
            }

          } // End of operand analysis.

          // Even if the instruction and the operands are valid, there are still
          // some possible bugs that can occur when you match an instruction
          // with an operand.

          // Jump instructions cannot be to a different section.
          if(operation == "JMP" || operation == "JMPN" || operation == "JMPP"
             || operation == "JMPZ") {

            argument1 = operand_list.front();

            // We already checked for bad/inexistant operands, therefore we do
            // not need to print any errors if this if statement isn't executed.
            if(symbols_table.count(argument1) > 0) {

              label_type = symbols_table[argument1].second;

              if(label_type == LabelType::CONST ||
                 label_type == LabelType::SPACE) {

                print_error(SEMANTIC, line_num,
                            "Jump destination is in another section!");
                pass2_error = true;

              }

            }

          } // End of jump exceptions.

          // Constants cannot be overwritten - Part I.
          else if(operation == "STORE" || operation == "INPUT") {

            argument1 = operand_list.front();

            // We already checked for bad/inexistant operands, therefore we do
            // not need to print any errors if this if statement isn't executed.
            if(symbols_table.count(argument1) > 0) {

              label_type = symbols_table[argument1].second;

              if(label_type == LabelType::CONST ||
                 label_type == LabelType::JUMP) {

                print_error(SEMANTIC, line_num,
                            "You can only save values to the BSS section!");
                pass2_error = true;
              }

            }

          } // End of constant exceptions - Part I.

          // Constants cannot be overwritten - Part II.
          else if(operation == "COPY") {

            argument2 = operand_list.back();

            // We already checked for bad/inexistant operands, therefore we do
            // not need to print any errors if this if statement isn't executed.
            if(symbols_table.count(argument2) > 0) {

              label_type = symbols_table[argument2].second;

              if(label_type == LabelType::CONST ||
                 label_type == LabelType::JUMP) {

                print_error(SEMANTIC, line_num,
                            "You can only save values to the BSS section!");
                pass2_error = true;
              }

            }

          } // End of constant exceptions - Part II.

          // Program cannot divide by 0.
          else if(operation == "DIV") {

            argument1 = operand_list.front();

            // We already checked for bad/inexistant operands, therefore we do
            // not need to print any errors if this if statement isn't executed.
            if(symbols_table.count(argument1) > 0) {

              label_type = symbols_table[argument1].second;

              if(label_type == LabelType::CONST) {
                if(constant_table[argument1] == "0") {
                  print_error(SEMANTIC, line_num, "You cannot divide by 0!");
                  pass2_error = true;
                }
              }

            }

          }

        } // End of valid instruction.

      } // End of instruction.

      // If the operation isn't an instruction, them it must be a directive!
      // SPACE directive:
      else if(operation == "SPACE") {

        // The SPACE directive needs to be in the BSS SECTION.
        if(actual_section != Section::BSS) {
          print_error(SEMANTIC, line_num,
                      "A SPACE directive was used outside the BSS SECTION!");
          pass2_error = true;
        }

        // Regular SPACE:
        else if(operand_num == 0) {
          machine_code.push_back(0);
          address++;
        }

        // SPACE with argument:
        else if(operand_num == 1) {

          argument1 = operand_list.front();

          // Valid operand:
          if(regex_match(argument1, positive_number)) {

            offset = stoi(argument1);

            if(offset == 0) {
              print_error(SYNTACTIC, line_num,
                          "An invalid operand was given to a SPACE directive!");
              pass2_error = true;
            }

            else {

              for(i = 0; i < offset; i++)
                machine_code.push_back(0);

              address += offset;

            }

          }

          // Invalid operand:
          else {
            print_error(SYNTACTIC, line_num,
                        "An invalid operand was given to a SPACE directive!");
            pass2_error = true;
          }

        } // End of SPACE with argument.

        else {
          print_error(SYNTACTIC, line_num,
                      "An invalid number of operands was given!");
          pass2_error = true;
        }

      } // End of SPACE.

      // CONST directive.
      else if(operation == "CONST") {

        // The CONST directive needs to be in the DATA SECTION.
        if(actual_section != Section::DATA) {
          print_error(SEMANTIC, line_num,
                      "A CONST directive was used outside the DATA SECTION!");
          pass2_error = true;
        }

        // The CONST directive must have an argument:
        else if(operand_num == 1) {

          argument1 = operand_list.front();

          // Valid decimal operand:
          if(regex_match(argument1, signed_number)) {
            const_value = stoi(argument1);

            if(const_value < -32768 || const_value > 32767) {
              print_error(SYNTACTIC, line_num,
                          "A CONST directive operand exceed 16 bits!");
              pass2_error = true;
            }

            else {
              machine_code.push_back(const_value);
              address++;
            }

          }

          // Valid hexadecimal number:
          else if(regex_match(argument1, hex_number)) {
            const_value = stoul(argument1, nullptr, 16);

            if(const_value > 65535) {
              print_error(SYNTACTIC, line_num,
                          "A CONST directive operand exceed 16 bits!");
              pass2_error = true;
            }

            else {
              machine_code.push_back(const_value);
              address++;
            }

          }

          // Invalid operand:
          else {
            print_error(SYNTACTIC, line_num,
                        "An invalid operand was given to a CONST directive!");
            pass2_error = true;
          }

        } // End of CONST with argument.

        else {
          print_error(SYNTACTIC, line_num,
                      "An invalid number of operands was given!");
          pass2_error = true;
        }

      } // End of CONST directive.

      // SECTION directive:
      else if(operation == "SECTION") {

        // Theoretically speaking, we can assume this SECTION statement is
        // valid due to the first compiling pass. In practice, a quick check
        // never hurts!

        if(operand_num != 1) {
          print_error(SYNTACTIC, line_num,
                      "An invalid number of operands was given!");
          pass2_error = true;
        }

        else {

          argument1 = operand_list.front();

          if(argument1 == "TEXT")
            actual_section = Section::TEXT;

          else if(argument1 == "BSS")
            actual_section = Section::BSS;

          else if(argument1 == "DATA")
            actual_section = Section::DATA;

          else {
            print_error(SYNTACTIC, line_num,
                        "An invalid operand was given to a SECTION directive!");
            pass2_error = true;
          }

        }

      } // End of SECTION directive.

      // BEGIN directive
      else if(operation == "BEGIN") {

        if(module_start) {
          print_error(SEMANTIC, line_num,
                      "Only one BEGIN directive can exist!");
          pass2_error = true;
        }

        else if(label == "") {
          print_error(SYNTACTIC, line_num,
                      "A BEGIN directive needs to be labeled!");
          pass2_error = true;
        }

        else if(actual_section != Section::BEGIN) {
          print_error(SEMANTIC, line_num,
                      "A BEGIN directive cannot come after any command!");
          pass2_error = true;
        }

        else
          module_start = true;

      } // End of BEGIN directive.

      // END directive
      else if(operation == "END") {
        module_end = true;
        actual_section = Section::END;
      }

      // Invalid operation (The first processing pass should have caught this):
      else if(operation != "" && operation != "PUBLIC"
              && operation != "EXTERN") {
        print_error(SYNTACTIC, line_num,
                    "Couldn't find any instruction/directive with that name!");
        pass2_error = true;
      }

      // Note: To the second processing pass, the directives "IF" and "EQU"
      // shouldn't exist and the directives "PUBLIC" and "EXTERN" aren't useful.

    } // End of valid command.

    // Theoretically speaking, this else should never, EVER be triggered!
    // Unless you have a line full of spaces or colons.
    // Did you break my formatting function just to get here?
    else {
      print_error(SYNTACTIC, line_num, "Invalid command!");
      pass2_error = true;
    }

  }

  valid_module = module_start && module_end;

  // Ok, quick check to see if somebody forgot to BEGIN or END a module!
  // Remember, either we have both or we have none. Otherwise, it's an error!

  if(module_start != module_end) {

    if(module_start) {
      print_error(FATAL, 0, "A module needs an END directive!");
      pass2_error = true;
    }

    else {
      print_error(FATAL, 0, "A module needs a BEGIN directive!");
      pass2_error = true;
    }

  }

  if(pass2_error) {
    print_error(FATAL, 0, "Second compiling pass was not successful!");
    exit_program(6);
  }

  cout << "::Second compiling pass was successful!" << endl << endl;

  // Creates a new file for the compilation output.
  obj_file.open(file_name + ".obj", ios::out);

  // Tests if the file has opened (it should open, but better safe than sorry).
  if(!obj_file.is_open()) {
    print_error(FATAL, 0, "Couldn't create file: " + file_name + ".obj!");
    exit_program(3);
  }

  if(valid_module) {

    // TABLE USE:
    obj_file << "TABLE USE" << endl;

    for(auto const& extern_label : use_table) {

      // The first position of the map contains the label's name.
      label = extern_label.first;

      // The second position of the map contains the label's use addresses.
      for(auto const& address : extern_label.second) {

        // For each address, we print a line in the use table.
        obj_file << label << " " << address << endl;

      }

    }

    obj_file << endl;

    // TABLE DEFINITION:
    obj_file << "TABLE DEFINITION" << endl;

    for(auto const& public_label : definitions_table) {

      // The first position of the map contains the label's name.
      label = public_label.first;

      // The second position of the map contains the label's address.
      address = public_label.second;

      obj_file << label << " " << address << endl;

    }

    obj_file << endl;

    // RELATIVE (0 indexed!):
    obj_file << "RELATIVE" << endl;

    for(auto iter = relative_addresses.begin();
        iter != relative_addresses.end(); iter++) {

      if(iter != relative_addresses.begin())
        obj_file << " ";

      obj_file << *iter;

      if(iter == prev(relative_addresses.end()))
        obj_file << endl;

    }

    obj_file << endl;

    // CODE:
    obj_file << "CODE" << endl;

  }

  for (auto iter = machine_code.begin(); iter != machine_code.end(); iter++) {

    if (iter != machine_code.begin())
      obj_file << " ";

    obj_file << *iter;

  }

  obj_file.close();

  cout << "::File compilation was successful!" << endl << endl;

  // Clean up allocated memory.
  clean_up();

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

int clean_up(void) {

  for(auto const& pair : opcodes_table)
    delete opcodes_table[pair.first];

  return 0;

}

int exit_program(int error_code) {

  cerr << "::Program execution could not continue!" << endl;

  switch (error_code) {

    case 1:
      cerr << "[EXIT] Incorrect number of arguments given to program!" << endl;
      break;

    case 2:
      cerr << "[EXIT] Could not open an input file!" << endl;
      break;

    case 3:
      cerr << "[EXIT] Could not create an output file!" << endl;
      break;

    case 4:
      cerr << "[EXIT] The pre-processing pass was not sucessful!" << endl;
      break;

    case 5:
      cerr << "[EXIT] The first compiling pass was not sucessful!" << endl;
      break;

    case 6:
      cerr << "[EXIT] The second compiling pass was not sucessful!" << endl;
      break;

    default:
      cerr << "[EXIT] An Unknown error ocurred!" << endl;

  }

  cerr << "Exiting!" << endl << endl;

  clean_up();

  exit(error_code);

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

list <string> split_string(string delimeter, string input) {

  list <string> results;
  size_t position;
  string result;

  while((position = input.find(delimeter)) != string::npos) {
    result = input.substr(0, position);
    results.push_back(result);
    input.erase(0, position + delimeter.size());
  }

  results.push_back(input);

  return results;

}

string format_line(string line) {

  regex colon(":"), comment(";.*"), first_space("^ "), last_space(" $");
  regex offset_plus1(" \\+"), offset_plus2("\\+ "), spaces_and_tabs("[ \t]+");
  string formated_line;

  // Removes comments and extra tabs and spaces.

  formated_line = regex_replace(line, comment, "");
  formated_line = regex_replace(formated_line, colon, ": ");
  formated_line = regex_replace(formated_line, spaces_and_tabs, " ");
  formated_line = regex_replace(formated_line, offset_plus1, "+");
  formated_line = regex_replace(formated_line, offset_plus2, "+");
  formated_line = regex_replace(formated_line, first_space, "");
  formated_line = regex_replace(formated_line, last_space, "");

  // Converts the whole string to uppercase.

  for (auto & c: formated_line)
    c = toupper(c);

  return formated_line;

}

string replace_aliases(string line, map <string, string> aliases_table) {

  list<string> words;
  string modded_line = "", word;

  // First, let's get rid of empty lines! Remember: we already formatted the
  // line, so this corner case catches comments, empty lines, etc...

  if(line == "")
    return line;

  // Now we split the line along spaces and stores it's words in a list called
  // words.

  words = split_string(" ", line);

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
