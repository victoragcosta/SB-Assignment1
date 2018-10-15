#ifndef OPERATION_HPP_
#define OPERATION_HPP_

class Operation
{
private:
  unsigned int m_opcode, m_size, m_nParameters;
public:
  Operation(unsigned int t_opcode, unsigned int t_size, unsigned int t_nParameters);
  ~Operation();
  unsigned int getOpcode();
  unsigned int getSize();
  unsigned int getNParameters();
};

#endif /* OPERATION_HPP_ */
