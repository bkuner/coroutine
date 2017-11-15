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
    this->name = name;
    this->parent = 0;
    this->top = 0;
    this->state = 0;
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

bool Coroutine::step() {
    // initially the top is NULL
    if (!this->top) {
        assert(!this->parent);
        this->top = this;
    }
    try {
        // resume top of the stack, passing its state
        if (debug > 1)
            std::cerr << "resuming coroutine " << this->top->name << std::endl;
        if (this->top->run(this->top->state)) {
            // top is done
            if (this->top->parent) {
                // we are not at the bottom
                pop_and_delete();
                // and thus not yet done
                return false;
            } else {
                // we are at the bottom so we're done now
                return true;
            }
        } else {
            return false;
        }
    } catch(Abort e) {
        std::cerr << "coroutine " << this->top->name << " aborted in state "
            << this->top->state << ": " << e.why() << std::endl;
        while (this->top) {
            pop_and_delete();
        }
        // we are done
        return true;
    }
}

bool Coroutine::suspend(int state) {
    if (debug > 0)
        std::cerr << "coroutine " << this->name << " suspended" << std::endl;
    this->state = state;
    // we are not yet done
    return false;
}

bool Coroutine::call(int state, Coroutine *other) {
    if (debug > 0)
        std::cerr << "coroutine " << this->name
            << " calling " << other->name << std::endl;
    this->state = state;
    this->top = other;
    other->parent = this;
    other->top = other;
    // we are not yet done
    return false;
}

bool Coroutine::done() {
    if (debug > 0)
        std::cerr << "coroutine " << this->name << " returned" << std::endl;
    // there is nothing more to do
    return true;
}
