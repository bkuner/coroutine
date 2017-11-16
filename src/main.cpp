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

    Coroutine and StateStack Example: Create curve like this, with call of rectangle- 
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
#include "coroutine.h"

// Set high < 8 to demonstrate Abort
int high=8;

class Rectangle: Coroutine
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
    bool run(int state);
};

Rectangle::Rectangle(int *data, int dbg)
    : Coroutine("Rectangle",dbg)
{
    size = 2;
    nrOfRectangles = 2;
    periode = 3;
    periodeCount = 0;
    this->data = data;
}

bool Rectangle::run(int state){
    switch(state) {
    case INIT:  *data += size;
                if(*data >= high)
                    throw Abort("High limit: exceeded!");
                return suspend(HIGH);
    case HIGH:  if(++periodeCount >= periode) {
                    *data -= size;
                    periodeCount = 0;
                    return suspend(LOW);
                }
                return suspend(HIGH);
    case LOW:   if( ++periodeCount >= periode) {
                    periodeCount = 0;
                    if(--nrOfRectangles) {
                        *data += size;
                        return suspend(HIGH);  // next Rectangle
                    }
                    else {
                        *data -= 1;
                        return done(0);  // Done
                    }
                }
                return suspend(LOW);
    default: throw Abort("Rectangle: Illegal state");
    }
}
class Triangle: Coroutine
{
    enum states{INC,DEC};
    int size;
    int nrOfTriangles;
    int *data;     // this stands for the date to operate with, has to come from outside

public:
    Triangle(int nrOfTriangles, int size, int *data, int dbg);
    ~Triangle(){};
    bool run(int state);
};

Triangle::Triangle(int nrOfTriangles, int size, int *data, int dbg)
    : Coroutine("Triangle",dbg)
{
    this->size = size;
    this->nrOfTriangles = nrOfTriangles;
    this->data = data;
}

bool Triangle::run(int state){
    switch(state) {
    case INC:   *data += 1;
                if(*data >= size) {
                    return call(DEC,(Coroutine*) new Rectangle(data,dbg-1));
                }
                return suspend(INC);;
    case DEC:   *data -= 1;
                if(*data <= 0) {
                    if(--nrOfTriangles) {
                        return suspend(INC);  // next Triangle
                    }
                    else
                        return done(0);  // Done
                }
                return suspend(DEC);
    default: throw Abort("Triangle: Illegal state");
    }
}

int main(int argc, char **arg)
{
    int nrOfWaves = 2;
    int size = 5;
    int dbg = 0;
    int data = 0;

    Coroutine *tri = (Coroutine *)new  Triangle(nrOfWaves,size,&data,dbg);
    tri->start();
    
    // after tri is done, start next
    Coroutine *next = (Coroutine*) new Rectangle(&data,dbg);
    startNext(&tri,next);
    do {
        if(dbg) 
            printf("%s:%d\t",tri->getTopName(),tri->getTopState());
        if( tri->step() )
            break;
        if(dbg) {
            printf("%d\n",data);
            sleep(1);
        }
        else
            printf("%d ",data);
    }
    while(1);

    int result = tri->getResult();
    if(result)
        printf("Evil END: %d Abort!\n",result);
    else
        printf("END: %d OK!\n",result);
    delete(tri);
    return 0;
    
}

