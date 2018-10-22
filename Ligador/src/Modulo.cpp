#include "Modulo.hpp"
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <regex>
#include <iomanip>
#include <algorithm>

#define DEBUG 0

typedef enum {
  NONE,
  USE_TABLE,
  DEFINITIONS_TABLE,
  RELATIVE,
  CODE
}Section;

using namespace std;

Modulo::Modulo(string t_obj_name)
{
  obj_name = t_obj_name;
  this->openStream();
}

Modulo::~Modulo()
{
  if(obj_file.is_open()){
    obj_file.close();
  }
}

void Modulo::openStream()
{
  if(obj_name != ""){
    obj_file.open(obj_name + ".obj", ios::in);
    if (!obj_file.is_open()) {
      cout << "Erro: arquivo " << obj_name << ".obj não existe!" << endl;
      exit(2);
    }
  }
}

void Modulo::parse()
{
  string file_line, label;
  int address;

  regex table_definition_regex("^TABLE DEFINITION$");
  regex table_use_regex("^TABLE USE$");
  regex relative_regex("^RELATIVE$");
  regex code_regex("^CODE$");
  regex blank_line_regex("^[ \t]*$");
  regex label_address_regex("^([A-Za-z_][A-Za-z_\\d]*) (\\d+)$");
  //regex multiple_numbers_regex("^(\\d+)+$");

  smatch search_matches;

  Section section = NONE;

  // Iterates over all .obj lines
  while(getline(obj_file, file_line)) {

    if(DEBUG >= 2){
      cout << file_line << endl;
    }
    // Finds TABLE DEFINITION
    if(regex_search(file_line, search_matches, table_definition_regex)) {
      section = DEFINITIONS_TABLE;
    }
    // Finds TABLE USE
    else if(regex_search(file_line, search_matches, table_use_regex)) {
      section = USE_TABLE;
    }
    // Finds RELATIVE
    else if(regex_search(file_line, search_matches, relative_regex)) {
      section = RELATIVE;
    }
    // Finds CODE
    else if(regex_search(file_line, search_matches, code_regex)) {
      section = CODE;
    }
    // Finds a blank line
    else if(regex_search(file_line, search_matches, blank_line_regex) || file_line == ""){
      section = NONE;
    }
    else { // Adds selectively to each data structure
      switch(section){
      case DEFINITIONS_TABLE:
        // Reads Label and address
        if(regex_search(file_line, search_matches, label_address_regex)){
          label = search_matches[1].str();
          address = stoi(search_matches[2].str());
          definitions_table[label] = address;
        } else {
          cout << "Erro: arquivo .obj corrompido" << endl;
          exit(3);
        }
        break;
      case USE_TABLE:
        // Reads Label and address
        if(regex_search(file_line, search_matches, label_address_regex)) {
          label = search_matches[1].str();
          address = stoi(search_matches[2].str());
          use_table[label].push_back(address);
        } else {
          cout << "Erro: arquivo .obj corrompido" << endl;
          exit(3);
        }
        break;
      case RELATIVE:
        // Splits line on spaces and gets all relative addresses
        relative = splitStringToInts(file_line);
        break;
      case CODE:
        // Splits line on spaces and gets each byte of the machine code
        code = splitStringToInts(file_line);
        break;
      default:
        cout << "Erro: Linha inválida!" << endl;
        break;
      }
    }
  }
  if (DEBUG >= 2) {
    cout << endl;
  }
}

void Modulo::fixCrossReferences(map<string, int> gdt)
{
  string label;

  for(auto const& item : use_table){
    label = item.first;
    for(auto const& address : item.second) {
      code[address] += (int) gdt[label];
      corrected.push_back(address);
    }
  }
}

void Modulo::fixRelativeAddresses(int correction)
{
  sort(corrected.begin(), corrected.end());
  for (auto const& address : relative) {
    if(!binary_search(corrected.begin(), corrected.end(), address)) {
      code[address] += (int) correction;
    }
  }
}

// Auxiliary methods

vector<int> Modulo::splitStringToInts(string phrase)
{
  istringstream line_stream(phrase);
  vector<string> splitted(istream_iterator<string>{line_stream}, istream_iterator<string>());
  vector<int> ints;
  for (auto const &s : splitted) {
    ints.push_back(stoi(s));
  }
  return ints;
}

/*
vector<int> Modulo::splitStringToUChars(string phrase)
{
  vector<int> ints;
  vector<unsigned char> chars;
  ints = splitStringToInts(phrase);
  for(auto const& i : ints) {
    chars.push_back((unsigned char)i);
  }
  return chars;
}
*/

// Getters

map<string, vector<int>> Modulo::getUseTable()
{
  return use_table;
}

map<string, int> Modulo::getDefinitionsTable()
{
  return definitions_table;
}

vector<int> Modulo::getRelativeAddresses()
{
  return relative;
}

vector<int> Modulo::getCode()
{
  return code;
}

int Modulo::getCodeSize()
{
  size_t bytes = code.size();
  return (int) bytes;
}

// Debug methods

void Modulo::printAllData()
{
  cout << obj_name << endl << endl;
  cout << "TABLE USE" << endl;
  printUseTable(use_table);
  cout << "TABLE DEFINITION" << endl;
  printTable(definitions_table);
  cout << "RELATIVE" << endl;
  printVectorInt(relative);
  cout << "CODE" << endl;
  printVectorInt(code);
}

void Modulo::printTable(map<string, int> table)
{
  size_t label_size, max = 5;
  string label;
  int address;
  for(auto const& line : table) {
    label_size = line.first.length();
    if(label_size > max)
      max = label_size;
  }
  cout << "| LABEL";
  for(size_t i = 5; i < max; i++) {
    cout << " ";
  }
  cout << " | ADDRESS |" << endl;
  for(auto const& line : table) {
    label = line.first;
    address = line.second;
    cout << "| " << label;
    for(size_t i = label.length(); i < max; i++) {
      cout << " ";
    }
    cout << " | " << setw(7) << setfill(' ') << address << " |" << endl;
  }
  cout << endl;
}

void Modulo::printUseTable(map<string, vector<int>> table)
{
  size_t label_size, max = 5;
  string label;
  vector<int> addresses;
  for (auto const &line : table)
  {
    label_size = line.first.length();
    if (label_size > max)
      max = label_size;
  }
  cout << "| LABEL";
  for (size_t i = 5; i < max; i++)
  {
    cout << " ";
  }
  cout << " | ADDRESS |" << endl;
  for (auto const &line : table)
  {
    label = line.first;
    addresses = line.second;
    for(auto const& address : addresses){
      cout << "| " << label;
      for (size_t i = label.length(); i < max; i++)
      {
        cout << " ";
      }
      cout << " | " << setw(7) << setfill(' ') << address << " |" << endl;
    }
  }
  cout << endl;
}

void Modulo::printVectorInt(vector<int> items)
{
  for(auto const& i : items) {
    cout << i << " ";
  }
  cout << endl << endl;
}

/*
void Modulo::printVectorUChar(vector<int> items)
{
  vector<int> ints;
  for(auto const& i : items) {
    ints.push_back((unsigned char) i);
  }
  printVectorInt(ints);
}
*/
