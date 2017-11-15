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
#ifndef COROUTINE_H
#define COROUTINE_H

#include <string>

// A Coroutine is like a procedure with no return value: you can call it with
// the right number and type of parameters, and return from it. In addition,
// you can suspend it (control goes back to the caller) or pass control to
// another Coroutine. Communicating results to the caller can be done via
// reference parameters.
class Coroutine {
public:
    // constructor
    Coroutine(const char* name, int debug=0);

    // some outside machinery must call this periodically or
    // whenever something interesting happens;
    // returns whether the Coroutine is done (has returned)
    bool step();

protected:

    // to be implemented in derived classes;
    virtual bool run(int state) = 0;

    // these methods are for use in the run() method above:

    // give up control, will resume at the given state
    bool suspend(int state);

    // pass control to another Coroutine, will resume at the given state;
    // the Coroutine gets automatically deleted when it is done
    bool call(int state, Coroutine *other);

    // return from the Coroutine
    bool done();

    // virtual destructor; needed because we delete Coroutine (base class)
    // objects and we want derived classes' destructors (if any) to be called
    virtual ~Coroutine() {}

    // our name (for debug output)
    const char *name;

    // debug level
    int debug;

private:
    // Implementation note: we are deliberately not using an std::stack object
    // (or something equivalent) here since that would force us to handle
    // its lifetime, which only complicates everything.

    // pop the top-most Coroutine off the stack and delete it
    void pop_and_delete();
    // our caller or NULL if there is none
    Coroutine *parent;
    // top of stack
    Coroutine *top;
    // remember where we are (instruction pointer so to speak)
    int state;
};

// Exception class; cought by Coroutine::step() so it can clean up the stack
class Abort {
public:
    Abort(std::string why) { this->reason = why; }
    virtual std::string why() { return reason; }
private:
    std::string reason;
};

// convenience macros, giving the illusion of a language extension
// (if you like that kind of stuff, it's purely optional)
#define SUSPEND(st)     return suspend(st)
#define CALL(st,cr)     return call(st,cr)
#define RETURN          return done()

#endif // COROUTINE_H
