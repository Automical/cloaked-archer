
#include "ParCompUtilities.h"

using namespace std;

/*
 * @brief Multiply state spaces of all components
 * @return worst-case state space
 */
unsigned long int GetUpperBoundStateSpace ( vector<FSM_struct> & FSMArray )
{
  unsigned long int worstcase = 1;
  for(int i=0; i<FSMArray.size(); i++)
  {
    worstcase *= FSMArray[i].GetNumberOfStates();
  }
  return worstcase;
}

/*
 * @brief Perform on-the-fly parComp, not storing copy of resulting FSM
 * @return Number of blocking states
 */
unsigned int OnTheFlyParComp( vector<FSM_struct> & FSMArray )
{
  /******** Calculate Worstcase ****************************/
  unsigned int worstcase = GetUpperBoundStateSpace(FSMArray);
  /*********************************************************/
    
  /******** Accessibility Search  ***************************/
  cout << endl << "~~~~~~~~~~~~~" << endl;
  cout << "Starting accessibility search of " << worstcase << " states." << endl;
  
  MemoryManager memory(FSMArray);
  EventManager event(FSMArray);
  StateEncoder encoder(FSMArray);
  
  unsigned int totalStatesAccessed = DepthFirstSearch(FSMArray, memory, event, encoder, worstcase, "", NULL, FORWARD );
  
  cout << "~~~~~~~~~~~~~" << endl;
  memory.PrintStatistics();
  /*********************************************************/
  
  /**** Invert Transitions for CoAccessibility Search ******/
  cout << endl << "~~~~~~~~~~~~~" << endl;
  cout << "Inverting transitions in all sub-automata." << endl;
  vector<FSM_struct> InverseArray;
  InvertTransitions(FSMArray, InverseArray);
  FSMArray.clear();
  cout << "Done inverting transitions. " << endl;
  cout << "~~~~~~~~~~~~~" << endl;
  /*********************************************************/

  
  /******** CoAccessibility Search  ***************************/
  cout << endl << "~~~~~~~~~~~~~"<<endl;
  cout << "Starting coaccessibility search of " << totalStatesAccessed << " accessed states." << endl;
  
  //Reset certain components of memory in order to limit search space
  memory.FlipBits();
  memory.LimitSearchSpaceOfCreatedStates();
  memory.ClearStatistics();
  
  EventManager event_inv(InverseArray);
  StateEncoder encoder_inv(InverseArray);
  
  unsigned int totalStatesCoaccessed = DepthFirstSearch(InverseArray, memory, event_inv, encoder_inv, worstcase, "CO_ACC_", NULL , BACKWARD);
  
  memory.PrintStatistics();
  cout << "~~~~~~~~~~~~~" << endl;
   
  return memory.GetNumberOfUnsetBits(); //Number of blocking states
  /************************************************************/      
}

/*
 * @brief Perform on-the-fly accessibility search, storing copy
 *        in file, fsm_struct object, or both
 * @return number Of States accessed
 */
unsigned int FullParCompWithOutput( vector<FSM_struct> & FSMArray, string filepath, FSM_struct * fsm )
{
  //Declare necessary structures
  MemoryManager memory(FSMArray);
  EventManager event(FSMArray);
  StateEncoder encoder(FSMArray);

  /******** Calculate Worstcase ****************************/
  unsigned int worstcase = GetUpperBoundStateSpace(FSMArray);
  /*********************************************************/    
  unsigned int totalStatesAccessed = DepthFirstSearch(FSMArray, memory, event, encoder, worstcase, filepath, fsm , FORWARD );

  return totalStatesAccessed;
}


/*
 * @brief The meat of the program. Perform DFS of ParComp
 *        composite state.
 * @return Number of states accessed
 */
unsigned int DepthFirstSearch(vector<FSM_struct>& FSMArray, MemoryManager & memory, EventManager & event, StateEncoder & encoder,
  unsigned int worstcase, string outputDirectory,  FSM_struct * fsm, Direction dir)
{
  int numberFSMs = FSMArray.size();

  /********* Open file if one is included *************/
  ofstream outfile;
  if( !outputDirectory.empty() )
  {
    //~~~~Generate fsmName~~~~~~~~~~~~~~~~~~~~~/
    string filename;
    for(int i=0; i<numberFSMs; i++){
      filename += (FSMArray[i].fsmName + ",");
    }
    filename.erase(filename.end()-1);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/
    
    //~~~~~Open file~~~~~~~~~~~~~~~~~~~~~~~~~~~/
    string filepath = outputDirectory + filename + ".fsm";
    outfile.open(filepath.c_str());
    if(!outfile.is_open()){
      cerr << "File "<< filepath << " failed to open!" << endl;
    }
    else
      outfile << setw(8) << "CORRUPT" << endl << endl;
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/
  }
  /******************************************************/
  
  /********** Add name to FSM object ******************/  
  if( fsm )
  {
    string name;
    for(int i=0; i<numberFSMs; i++){
      name += (FSMArray[i].fsmName + ",");
    }
    name.erase(name.end()-1);
    fsm->fsmName = name;    
  }
  /******************************************************/
  
  //Count the number of intially set bits
  unsigned int InitialBitCount = memory.GetNumberOfSetBits();
  
	//Initialize search stack for depth-first search, add initial state
	//Note that this assumes the 0th state in each file is the marked state
	unsigned currentState = 0;
	unsigned int counter = 0;
	unsigned int maxStackSize = 0;

  //Exception states to be accounted for after search
  map<unsigned int, State> specialStates;
  const pair<string,string> specialEvent = make_pair((dir == FORWARD)?"DDC":"DC", (dir == FORWARD)?"_DDC":"_DC");
	
	//Initialize Stack
	memory.PushOnStack( currentState , DEFAULT_EVENT_MASK );	
	
	while( !memory.IsStackEmpty() )
	{	
		//Get current state off top of stack
		pair<unsigned int, EventTypeMask> currentPair = memory.PopOffStack();
		
		currentState = currentPair.first;
		EventTypeMask currentMask = currentPair.second;
		bool currentMarked = false; 
    string titleAppend = "";
	
	
	  //Assign mask and marked status
	  switch(currentMask)
	  {	
	    case DEFAULT_EVENT_MASK:
		    //  If doing accessibility search,  and if ALL states are marked ("proper" states),
		    //  consider all events from all states, assign mask ALL_EVENTS_MASK.
		    //  Otherwise, assign mask VARIABLE_EVENT, only considering lowercase events 
		    //  In a coaccessibility search, the same rules apply, but DDC should never be allowed
		    //  without being preceded by a DC
		    currentMarked = !encoder.CheckForUnmarkedStates(currentState);
		    if(dir == FORWARD)
		      currentMask = ( currentMarked ? ALL_EVENTS_MASK : VARIABLE_EVENT );
		    else if(dir == BACKWARD)
		      currentMask = ( currentMarked ? NOT_DDC_MASK : DC_AND_VAR_MASK );	      
		    break;
		 
		  case DC_EVENT:
		    //  This state was reached in an accessibility search by event DDC,
		    //  only event DC is allowed to occur.
		    currentMarked = false;
		    titleAppend = "_DDC";
		    break;
		    
		  case DDC_EVENT:
		    //  This state was reached in a co-accessibility search by event DC,
		    //  only event DDC is allowed to occur
		    currentMarked = false;
		    titleAppend = "_DC";
		    break;
		  
		  default:
		    //  This state should not be reached in the current implementation
		    cerr << "An unsupported EventType (\""<<(int)currentMask<<"\")";
		    cerr << "was pulled off the stack at state " << currentState<<". Exiting.";
		    exit(1);
		}
		
		// Loop through each FSM, examining the current state's transitions, subject to mask restrictions
    for(int i=0; i<numberFSMs; i++)
    {
	    //Extract state index from encoded composite state
	    int stateIndex = encoder.FindStateIndex(currentState, i);
	    
      //Add events, passing currentMask to filter events
      event.AddTransitions( FSMArray[i].states[stateIndex], i, currentMask);        
    }
        
		//Get next states
		vector<Trans> nextStates;
    event.GetNextStates( currentState, nextStates );
  
    //Push states onto stack
    if(dir == FORWARD)
    {
      //Push next states onto stack
      for( int i=0; i<nextStates.size(); i++)
      {
        if(!nextStates[i].event.compare("DDC"))
        {

          memory.PushOnStack(nextStates[i].dest, DC_EVENT);
          nextStates[i].mask = DC_EVENT;
        }
        else
        {
          memory.PushOnStack(nextStates[i].dest, DEFAULT_EVENT_MASK);
        }
      }     
    }
    else if(dir == BACKWARD)
    {
      //Push next states onto stack
      unsigned int numberOfStates = nextStates.size(); 
      for( int i=0; i<numberOfStates; i++)
      {
        if(!nextStates[i].event.compare("DC"))
        {
          //Spilt destination into two different states
          memory.PushOnStack(nextStates[i].dest, DDC_EVENT);
          memory.PushOnStack(nextStates[i].dest, DEFAULT_EVENT_MASK);  
          Trans newTrans = nextStates[i];
          newTrans.mask = DDC_EVENT;
          nextStates.push_back(newTrans);    
        }
        else
        {
          memory.PushOnStack(nextStates[i].dest, DEFAULT_EVENT_MASK); 
        }
      } 
    }      
    
    //Write current state to .FSM file
    if( outfile.is_open() )
    {
      WriteStateToFile( currentState, nextStates, currentMarked, outfile, encoder, specialEvent, titleAppend );
    }
        
    //Add state to new FSM
    /*if( fsm )
    {
      AddStateToFSM( currentState, nextStates, currentMarked, fsm, encoder, specialEvent, 0 );
    }*/
    
    //Update stack statistics
    counter++;
		if(!(counter % 1000))
		{
		  memory.UpdateStatistics(counter);
		  if(memory.GetSizeOfStack() > maxStackSize )
		  {
		    maxStackSize = memory.GetSizeOfStack();
		  }
			cout << counter << " states accessed. ["<< (float)(counter*100)/worstcase << "%]";
			cout << " Stack size: " << memory.GetSizeOfStack() << "         \r";
			cout.flush();
		}
	}
	
	//Print summary statistics
	cout << "Total states accessed :   " << counter;
	cout << "["<< (float)(counter*(unsigned)100)/worstcase << "%]                  "<<endl;
	cout << "Standard states visited: " << memory.GetNumberOfSetBits()-InitialBitCount << endl;
	cout << "States created:    " << memory.GetNumberOfCreatedStates() << endl;
	cout << "Max size of stack: "<< maxStackSize << endl;
	
	//Add number of states to the top of the file
	if(outfile.is_open())
  {
    outfile.seekp(0, ios::beg);
    outfile << setw(8) << counter;
    outfile.close();
  }  
  
	return counter;
}


