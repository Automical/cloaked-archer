#include <FSMDataStructures.h>

using namespace std;

State::State(const std::string& str)
	:stateName(str)
{};


/*
 * FSM_struct constructor
 */
FSM_struct::FSM_struct(void)
	:numEvents(0)
{};

/*
 * Used to clear a struct of all information 
 */
void FSM_struct::Clear(void)
{
  this->fsmName.clear();;
  this->alphabet.clear();
  this->states.clear();
  this->numEvents = 0;
};

int FSM_struct::GetNumberOfStates(void) const
{
  return states.size();
};





/*
 * Add an event to the alphabet of the FSM.
 * If it already exists, do nothing.
 */
void FSM_struct::addEvent(string& str)
{
	//Linear search for events
	for (vector<string>::iterator it = alphabet.begin() ; it != alphabet.end(); it++)
	{
		if(!str.compare(*it))
		{
			return;
		}
	}
	
	//Event not found, add it	
	alphabet.push_back(str);
	numEvents++;
	return;
};


/*
 * Search for a state in the list of states
 * If found, return the index
 * If not found, make a state object and add it to the list, returning that index
 */
int FSM_struct::getStateIndex(string& str)
{
    	//Linear search for states
    	for (int i=0; i<states.size(); i++)
    	{
    		if(!str.compare(states[i].stateName))
    		{
    			return i;
    		}
    	}
    	
    	//Not found, add state to the end of the states vector
    	State newstate(str);
    	states.push_back(newstate);
    	return states.size()-1;    	
};


Event_obj::Event_obj()
	:valid(0),
	FSMcount(0) 
{};
		
		
Event_obj::Event_obj(string& name, int fsm, int dest)
	:valid(0),
	FSMcount(1),
	event(name)
{
	pair<int,int> mypair(fsm, dest);
	transitions.push_back(mypair);
};	
		
    	
