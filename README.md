# coroutine

Author: Bernhard Kuner

* Non blocking implementation of a state machine or sequence as coroutine,

* Special feature: function calls by nested coroutines.

## What is it useful for:

The idea came from a device that has to be accessed via modbus-tcp and polling. In order to 
do this we implemented a state machine.

The device is polled in a special task. We want to initiate actions in the main control program
and continue with the poll task to ensure answers of the device are handled immediately.

## Implementation

* **class Coroutine**: The base class for the sequence / state machine. Derived classes have to 
implement `run()`, which is the coroutine. The Coroutine class implements also the stack!

* **struct Abort**: Exception. Throw this to return and clear the stack. 

How to use it is shown in this example StateSet

The Example class gets 2 arguments that have to be persistent between the calls of runFunc().

class myStateSet : Coroutine
{
     int arg1,arg2;
     myStateSet(a1,a2,dbg);
     bool run(int state);
};

myStateSet(a1,a2,dbg=0)
: Coroutine("myStateSet",dbg)
{
     arg1 = a1;      // Store the arguments
     arg2 = a2;
}

The run function must never block. It has to perform all transition checks and actions. In case of a transition it has to set the new state in isState and return immediatly.

run() may call a nested StateMachine, see case 2.

run(int state)
{
     int ret;

     switch(state)
     case 1: doSomething();
             return suspend(2);      // set next state and return
     case 2: if( somethingIsDone() ) {
                 return call(3,(StateSet *) new yourStates()); // call nested StateSet 'yourStates'
             }
             else {
                 throw StateError("ERROR something is not done: Finish all");
             }
     case 3:
             ret = cleanUpThis();
             return done(ret);  // Done, return ret to caller
};

Run the state machine by creating a StateSet object and add it to the stack. The StateStack.run() function has to be called periodically in a loop or task to perform the actions. The stateStack.run function will allways call the stack.back function if the stack is not empty.

    Coroutine *st = (Coroutine *)new  myStateSet(nrOfWaves,size,&data,dbg);
    st->start();    // set this as bottom object before run it!
    
    do {
        if( st->step() )
            break;
        printf("%d\n",data);
        sleep(1);
    }
    while(1);
    printf("END, result: %d\n",tri->getResult());

