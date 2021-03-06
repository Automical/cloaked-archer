#ifndef _PAR_COMP_UTILITIES_H_
#define _PAR_COMP_UTILITIES_H_

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <algorithm>
#include <stack>
#include <bitset>
#include <cmath>
#include <map>
#include <assert.h>

#include "FSMDataStructures.h"
#include "MemoryManager.h"
#include "EventManager.h"
#include "StateEncoder.h"
#include "DjikstraMemoryManager.h"

#include "IO_Utilities.h"

enum Direction{
  FORWARD = 0,
  BACKWARD = 1
};

/*
 * @brief Multiply state spaces of all components
 * @return worst-case state space
 */
unsigned long int GetUpperBoundStateSpace(const std::vector<FSM_struct>& FSMArr);

/*
 * @brief Add up number of bits needed to encode each state
 * @return number of bits needed
 */
int GetNumberOfBitsToEncodeUncompressed( const std::vector<FSM_struct> & FSMArray );

/*
 * @brief Perform on-the-fly parComp, not storing copy of resulting FSM
 * @return Number of blocking states
 */
unsigned int OnTheFlyParComp( const std::vector<FSM_struct> & FSMArray );

/*
 * @brief The meat of the program. Perform DFS of ParComp
 *        composite state.
 * @return Number of states accessed
 */
unsigned int DepthFirstSearch( const std::vector<FSM_struct>& FSMArr, MemoryManager & memory, 
  EventManager & event, StateEncoder & encoder, unsigned int worstcase, 
  std::string outputDirectory, FSM_struct * fsm, Direction dir );

/*
 * @brief Perform on-the-fly accessibility search, storing copy
 *        in file, fsm_struct object, or both
 * @return number Of States accessed
 */
unsigned int FullParCompWithOutput( const std::vector<FSM_struct> & FSMArray,  std::string filepath , FSM_struct * fsm);

/*
 * @brief Performs accessibility search on graph, printing events which lead to blocking states
 * @note This is a specialized version of DFS
 */
void FindBlockingEvents( const std::vector<FSM_struct>& FSMArray, DjikstraMemoryManager & memory, EventManager & event, StateEncoder & encoder, MemoryManager & blockingStates);




#endif
