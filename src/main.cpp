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
/*  compile with ' g++ -o coroutineTest main.cpp statestack.cpp'

    StateSet and StateStack Example: Create curve like this, with call of rectangle- 
     from triangle class.

     - coroutines: triangle, nested rectangele

          XXX   XXX   XXX       XXX   XXX   XXX
         X   XXX   XXX   X   XXX   XXX   XXX   XXX
        X                 X X                     X 
       X                   X                       X END 

    Output:
        1
        2
        3
        4
        5
        7
        7
        7
        5
        5
        5
        7
        7
        7
        5
        5
        5
        4
        3
        2
        1
        0
        1
        2
        3
        4
        5
        7
        7
        7
        5
        5
        5
        7
        7
        7
        5
        5
        5
        4
        3
        2
        1
        0
        END
*/
#include <stdio.h>
#include <unistd.h>
#include "statestack.h"

class Rectangle: StateSet
{
    enum states{INIT,HIGH,LOW};
    int size;
    int nrOfRectangles;
    int periode;
    int periodeCount;
    int *data;

public:
    Rectangle(int *data, int dbg);
    ~Rectangle(){}
    StateReturnValue runFunc(int dbg);
};

Rectangle::Rectangle(int *data, int dbg)
    : StateSet(dbg)
{
    size = 2;
    nrOfRectangles = 2;
    periode = 3;
    periodeCount = 0;
    this->data = data;
}

StateReturnValue Rectangle::runFunc(int dbg){
    switch(isState) {
    case INIT:  *data += size;
                isState = HIGH;
                return StateReturnValue(0,0,NULL);
    case HIGH:  if(++periodeCount >= periode) {
                    *data -= size;
                    periodeCount = 0;
                    isState = LOW;
                }
                return StateReturnValue(0,0,NULL);
    case LOW:   if( ++periodeCount >= periode) {
                    periodeCount = 0;
                    if(--nrOfRectangles) {
                        *data += size;
                        isState = HIGH;
                        return StateReturnValue(0,0,NULL);  // next Rectangle
                    }
                    else {
                        *data -= 1;
                        return StateReturnValue(1,0,NULL);  // Done
                    }
                }
                return StateReturnValue(0,0,NULL);
    }
}
class Triangle: StateSet
{
    enum states{INC,DEC};
    int size;
    int nrOfTriangles;
    int *data;     // this stands for the date to operate with, has to come from outside

public:
    Triangle(int nrOfTriangles, int size, int *data, int dbg);
    ~Triangle(){};
    StateReturnValue runFunc(int dbg);
};

Triangle::Triangle(int nrOfTriangles, int size, int *data, int dbg)
    : StateSet(dbg)
{
    this->size = size;
    this->nrOfTriangles = nrOfTriangles;
    this->data = data;
}

StateReturnValue Triangle::runFunc(int dbg){
    switch(isState) {
    case INC:   *data += 1;
                if(*data >= size) {
                    isState = DEC;
                    return StateReturnValue(0,0,(StateSet*) new Rectangle(data,dbg-1));
                }
                return StateReturnValue(0,0,NULL);
    case DEC:   *data -= 1;
                if(*data <= 0) {
                    isState = INC;
                    if(--nrOfTriangles) {
                        return StateReturnValue(0,0,NULL);  // next Triangle
                    }
                    else
                        return StateReturnValue(1,0,NULL);  // Done
                }
                return StateReturnValue(0,0,NULL);
    }
}


int main(int argc, char **arg)
{
    int nrOfWaves = 2;
    int size = 5;
    int dbg = 0;
    int data = 0;
    StateStack *stStack = new StateStack(0);
    stStack->call( (StateSet *)new  Triangle(nrOfWaves,size,&data,dbg) );
    do {
        stStack->run(dbg);
        printf("%d\n",data);
        sleep(1);
    }
    while(! stStack->empty());
    printf("END\n");
    return 0;
    
}

