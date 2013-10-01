#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

#include <bitset>
#include <stack>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <cmath>

#define WORSTCASE_NUMBER_OF_STATES 100000000

class MemoryManager
{
private:
  /* DFS memory structures */
  std::bitset<WORSTCASE_NUMBER_OF_STATES> accessed; 
  std::stack<unsigned int> searchStack;
  
  /* FSM parameters */
  int numberOfStateMachines;
  std::vector<int> numbits;
  std::vector<int> offset;
  std::vector<int> accumulator;
  
  /* Conversion functions */
  unsigned int EncodedStateToIndex(unsigned int encodedState);
  unsigned int IndexToEncodedState(unsigned int index);

public:
  MemoryManager(std::vector<int> & numStates);
  bool GetBit(unsigned int encodedState);
  void SetBit(unsigned int encodedState);
  unsigned int PopOffStack(void);
  void PushOnStack(unsigned int encodedState);
  unsigned int GetSizeOfStack(void);
  bool IsStackEmpty(void);
};
#endif