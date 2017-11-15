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
#include "coroutine.h"

#include <assert.h>
#include <iostream>

Coroutine::Coroutine(const char* name, int debug) {
    this->debug = debug;
    this->name   = name;
    this->parent = 0;
    this->top    = 0;
    this->bottom = 0;
    this->state  = 0;
    this->dbg    = debug;
    if (debug > 0)
        std::cerr << "coroutine " << this->name << " created" << std::endl;
}

void Coroutine::pop_and_delete() {
    if (this->top) {
        Coroutine *was_top = this->top;
        this->top = this->top->parent;
        if (was_top != this) {
            delete was_top;
        }
    }
}

bool Coroutine::step() {    // this it the method of the bottom Coroutine object
    // initially the top is NULL
    if (!this->top) {
        assert(!this->parent);
        this->top = this;
    }
    try {
        // resume top of the stack, passing its state
        if (debug > 1)
            std::cerr << "Resume coroutine: " << this->top->name << ":"<<this->top->state<< std::endl;
        if (this->top->run(this->top->state)) {
            // top is done
            int res = this->top->result;
            if (this->top->parent) {
                // we are not at the bottom
                pop_and_delete();   // this it the method of the bottom Coroutine object
                this->top->result = res;    // pass result to the new top
                // and thus not yet done
                return false;
            } else {
                // we are at the bottom so we're done now, delete may be done by the caller.
                this->result = this->top->result; // pass result to bottom
                return true;
            }
        } else {
            return false;
        }
    } catch(Abort e) {
        std::cerr << "coroutine " << this->top->name << " aborted in state "
            << this->top->state << ": " << e.why() << std::endl;
        abort();
        // we are done
        return true;
    }
}

// The run() method has to return one of these functions to signal the bottom->step() function
// the result of this step
bool Coroutine::suspend(int state) {
    if (debug > 0)
        std::cerr << "coroutine " << this->name << " suspended:" << state << std::endl;
    this->state = state;
    // we are not yet done
    return false;
}

// Start a new Coroutine, make it a bottom object. Set top only for the bottom
// object it is called in the periodicaly task: bottom->step() calles bottom->top->run()
void Coroutine::start(void)
{
    this->top    = this;
    this->bottom = this;
}

// call() extends the list with a new Coroutine object:
//
// stack:  bottom/first/sec/../parent.call( new other)
//                             { other->parent = this
//                               other->bottom = parent->bottom // pass bottom along the list
//                               bottom->top   = other  // set the new top to be called next
//                             }
// run cyclic: bottom->top->run()
bool Coroutine::call(int state, Coroutine *other) { // call is allways the parent function
    if (debug > 0)
        std::cerr << "coroutine " << this->name << " calling " << other->name << std::endl;
    this->state = state;
    this->bottom->top = other;
    other->parent = this;
    other->bottom = this->bottom;
    // we are not yet done
    return false;
}

bool Coroutine::done(int result) {
    if(this->parent != NULL)
        this->parent->result = result; // give result back to the parent
    else
        this->result = result; // bottom done, set bottom->result
    if (debug > 0)
        std::cerr << "coroutine " << this->name << " returned" << std::endl;
    // there is nothing more to do
    return true;
}
