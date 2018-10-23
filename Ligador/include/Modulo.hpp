#ifndef MODULO_HPP_
#define MODULO_HPP_

#include <map>
#include <fstream>
#include <vector>

using namespace std;

class Modulo
{
private:
  string obj_name;
  fstream obj_file;
  map<string, vector<int>> use_table;
  map<string, int> definitions_table;
  vector<int> relative;
  vector<int> code;
  vector<int> splitStringToInts(string phrase);
  //vector<unsigned char> splitStringToUChars(string phrase);
  vector<int> corrected; // Stores addresses that were corrected
public:
  Modulo(string t_obj_name);
  ~Modulo();
  void openStream();
  void parse();
  void fixCrossReferences(map<string, int> gdt);
  void fixRelativeAddresses(int correction_table);
  // Getters
  map<string, vector<int>> getUseTable();
  map<string, int> getDefinitionsTable();
  vector<int> getRelativeAddresses();
  vector<int> getCode();
  int getCodeSize();
  // Debug
  void printAllData();
  void printTable(map<string, int> table);
  void printUseTable(map<string, vector<int>> table);
  void printVectorInt(vector<int> items);
  //void printVectorUChar(vector<unsigned char> items);
};

#endif /* MODULO_HPP_ */
