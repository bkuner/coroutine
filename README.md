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

* **cass StateSet**: The base class for the sequence / state machine. Derived classes have to 
implement `runFunc()`, which is the coroutine.

* **class StateStack**: Manager for the nested StateSet classes.

* **struct StateReturnValue**: Return value of `runFunc()` to pass data to the StateStack.

* **struct StateError**: Exception. Throw this to return and clear the stack. 

## How to use it is shown in this example StateSet

The Example class gets 2 arguments that have to be persistent between the calls of runFunc().

```
class myStateSet : StateSet
{
     int arg1,arg2;
     myStateSet(a1,a2,dbg);
     runFunc();
};

myStateSet(a1,a2,dbg=0)
: StateSet(dbg)
{
     arg1 = a1;      // Store the arguments
     arg2 = a2;
}
```

The run function must never block. It has to perform all transition checks and actions. In case of a transition
it has to set the new state in `isState` and return immediatly.

runFunc() may call a nested StateMachine, see `case 2`.

```
runFunc()
{
     int ret;

     switch(isState)
     case 1: doSomething();
             isState=2;
             return StateReturnValue(0,0,NULL);      // set next state and return
     case 2: if( somethingIsDone() ) {
                 isState = 3;
                 return StateReturnValue(0,0,(StateSet *) new yourStates()); // call nested StateSet 'yourStates'
             }
             else {
                 throw StateError("ERROR something is not done: Finish all");
             }
     case 3:
             ret = cleanUpThis();
             return StateReturnValue(1,ret,NULL);  // Done, return ret to caller
};
```

Run the state machine by creating a StateSet object and add it to the stack. The `StateStack.run()` function
has to be called periodically in a loop or task to perform the actions. The stateStack.run function will
allways call the `stack.back` function if the stack is not empty.

```
StateStack myStack;
StateSet *myS = (StateSet *) new myStateSet(12,34,1);

myStack.call(myS);           // set new stateSet (won't run it)

while( !myStack.empty() ) {  // or any other cyclic task
     myStack.run()
     sleep(1);
}
printf("RESULT: %d\n", myStack.getReturn() );
```
