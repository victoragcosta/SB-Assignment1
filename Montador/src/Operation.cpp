#include "Operation.hpp"

Operation::Operation(unsigned int t_opcode, unsigned int t_size, unsigned int t_nParameters)
{
  m_opcode = t_opcode;
  m_size = t_size;
  m_nParameters = t_nParameters;
}

Operation::~Operation()
{
}

unsigned int Operation::getOpcode()
{
  return m_opcode;
}

unsigned int Operation::getSize()
{
  return m_size;
}

unsigned int Operation::getNParameters()
{
  return m_nParameters;
}
