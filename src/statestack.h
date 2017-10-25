/*************************************************************************
* Copyright (c) 2016 Helmholtz-Zentrum Berlin
*     fuer Materialien und Energie GmbH (HZB), Berlin, Germany.
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/
#ifndef STATESET_H
#define STATESET_H

#include <list>
#include <string>
#include <time.h>

struct StateReturnValue;
/* StateSet is the base class for a state machine.
 * A derived class that implements the state machine has to define a constructor with its
 * required arguments and a run function tha perform all actions. See Example for details.
*/
class StateSet {
public:
    int isState;
    const char *stateId;    // object identifies itself, just for debug purposes
    int returnValue;        // return value of a nested StateSet, set via StateStack.run()
    int dbg;
    StateSet(int debug=0);
    virtual ~StateSet(){ }

    // StateStack.run() will get a StateReturnValue.retVal. By setReturn() it will be passed to
    // the caller StateSet and to the StateStack to be checked if the complete stateStack is done.
    void setReturn(int ret);

    // Derived class implements the state set by means of a coroutine.
    virtual StateReturnValue runFunc(int debug) = 0;
};


/* Exception to be thrown from the StateSet to signal the StateStack to clear
 * the stack and so break and finish complete operation. An alternative to the
 * exception for nested StateSets is to return an error value to the caller.
 */
struct StateError
{
    std::string errStr;
    StateError(const char *);
    std::string what();
};

class StateSet;

// StateSet class run fuction returns this to StateStack->runFunc()
struct StateReturnValue {
    int done;        // return from derived run() function:
                    //      0 = not finished, but check *next
                    //      1 = called function done, delete this stateSet
    int retVal;     // Set this value to StateStack.retVal when finish stateSet (done=1)
    StateSet *next; // if *next push this on top of stack
    StateReturnValue(int done,int retVal,StateSet *n);
};

/* Hold and manage the stack of StateSets
 */
class StateStack
{
public:
    StateStack(void);
    StateStack(int dbg);
    int run(int dbg);       // run the top of stack runFunc of the StateSet class
    void pop(void);         // remove last item from stack and delete() it!
    int  getReturn(void);   // get the return value of finished StateSet, used for the initial caller if the stateStack gets empty 
    void call(StateSet *s); // Set a new StateSet. Nested StateSets are put on stack if set in StateReturnValue.next
    bool empty(void);       // Check if any stateSet active
    StateSet *getTop(void);
private:
    int retVal;
    int dbg;
    std::list<StateSet *> stStack;
};

#endif // STATESET_H

