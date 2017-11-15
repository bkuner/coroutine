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

// Example
// -------

#include <iostream>
#include <random>
#include <stdlib.h>
#include <unistd.h>
#include "coroutine.h"

class Inner : public Coroutine
{
    double &result;
    int counter;
public:
    Inner(double &result) : Coroutine("Inner", 1), result(result) {}
    // we use the macros here to illustrate their use
    bool run(int state) {
        switch (state) {
        case 0:
            counter = 0;
            SUSPEND(1);
        case 1:
            counter++;
            if (counter >= 2) {
                result = 123.456;
                RETURN;
            } else {
                SUSPEND(1);
            }
        default:
            throw Abort("impossible");
        }
    }
};

class Outer : public Coroutine
{
    int arg;
    double resultOfInner;
    double &result;
public:
    Outer(int arg, double &result) : Coroutine("Outer", 1), result(result) {
        this->arg = arg;
    }
    // The run() method is usually implemented as a switch statement that
    // dispatches on the state, which it gets passed as a parameter. When we yield
    // control, we pass the next state to the suspend or call method and return
    // immediately.
    bool run(int state) {
        int r;
        std::random_device rd;
        std::uniform_int_distribution<> dis(1, 2*arg);
        switch(state) {
        case 0:
            std::cerr << "start doing something with " << arg << std::endl;
            return suspend(1); // next state will be 1
        case 1:
            r = dis(rd);
            std::cerr << "r = " << r << std::endl;
            if (r < arg) {
                // call another Coroutine; when it finishes,
                // continue at state 2
                return call(2, new Inner(resultOfInner));
            } else {
                throw Abort("ERROR: something is not done");
            }
        case 2:
            result = resultOfInner;
            std::cerr << "cleaning up..." << std::endl;
            return done();
        default:
            throw Abort("impossible");
        }
    }
};

int main () {
    double result;
    // create outer Coroutine:
    Outer ex = Outer(42, result);

    // the step() method has to be called periodically or
    // event triggered to actually do anything
    int counter = 0;
    do {
        std::cerr << "step " << counter << std::endl;
        if (counter) sleep(1);
        counter++;
    } while (!ex.step());
    std::cerr << "RESULT: " << result << std::endl;
    return 0;
}
