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
#include <iostream>
#include <typeinfo>
#include <stdio.h>
#include "statestack.h"

/* How to use:

    - start a statset by call 'callFunc(myStateStack)', myStateStack will be init
      with isState=0

    - always start StateStack with 0 as init state! Init your private variables here

    - finish a statset - and remove from stack, by returnFunc()

    - when throw an other than StateError, catched ouside the StateStack call
      clear() to remove all from the stack. To restart call callFunc to set
      stack to inital StateStack.
*/
StateReturnValue::StateReturnValue(int ret,int retVal,StateSet *n)
{
    this->done = ret;
    this->retVal = retVal;
    this->next = n;
}

StateSet::StateSet(int dbg)
{
    this->dbg = dbg;
    this->isState = 0;
}

void StateSet::setReturn(int ret)
{
    returnValue = ret;
    return;
}

StateStack::StateStack(int debug=0)
{
    retVal  = 0;
    dbg     = (debug>0)?debug:0;
}

/* get return value from last finished stateFunc */
int StateStack::getReturn()
{
    return retVal;
}

StateSet *StateStack::getTop(void)
{
    return stStack.back();
}

bool StateStack::empty(void)
{
    return stStack.empty();
}

/* set StateStack to be called next to top of the stack*/
void StateStack::call(StateSet* next)
{
    if(dbg>0) printf("callFunc push '%s,%d' (%p)\n",next->stateId,next->isState,next);
    stStack.push_back(next);
}

void StateStack::pop(void)
{
    StateSet *s = stStack.back();
    stStack.pop_back();
    delete(s);
}

/* run top of stack run function */
int StateStack::run(int dbg)
{
    StateSet *s;
    StateReturnValue stRet = StateReturnValue(0,0,NULL);
    if(!stStack.empty()) {
        s = stStack.back();
        int isStateOld = s->isState;
        try{
            if(dbg>0) printf("run '%s,%d' (%p)\n",s->stateId,s->isState,s);
            stRet = s->runFunc(dbg);
        }
        catch(StateError e) {
            fprintf(stderr,"State Exception in: %s:%d : '%s', clear Stack\n",s->stateId,s->isState,e.what().c_str());
            while(!stStack.empty() )
               pop();
            retVal = 1;      // the Exception sets StateStack::retVal to indicate an error
            return 1;
        }
        if(s->isState != isStateOld) {
            if(dbg>0) printf("%s: %d -> %d\n",s->stateId,isStateOld,s->isState);
        }
        if(stRet.done) {      // Nested called StateFunc done, remove from stack.
            retVal = stRet.retVal;
            pop();
            if(!stStack.empty()) {
                s = stStack.back();
                s->setReturn(retVal);
            }
        }
        else if(stRet.next != NULL){
            call(stRet.next);
        }

    }
    else
        return -1;
    return 0;
}

StateError::StateError(const char *err)
    :errStr(err)
{
}

std::string StateError::what()
{
    return errStr;
}

