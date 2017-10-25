# coroutine

* Non blocking implementation of a state machine or sequence as coroutine,

* Special feature: nested coroutines to be used as a function calls.

## What is it useful for:

The intention was to perform a state machine device polled via modbus tcp. The device is polled in 
a special task and so it is useful to initiate actions, means sequences, to the device in the 
main control program and continue it in the poll task to be shure you have imediate access to the 
answer of the device.

## Implementation

* **cass StateSet**: The base class for the sequence / state machine. Derived classes have to implement 
 the run() function, means the coroutine

* **class StateStack**: Manager for the nested StateSet classes

* **class StateReturnValue**: Return value for all cases of the run() function to pass data to the 
 StateStack

## How to use it shown in an example StateSet:

The Example class gets 2 arguments, that have to be persistent between the calls of the run() function.

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

The run function is non waiting, it has to perform all transition checks and actions. In case of a transition
it has to set the new state  (variable isState) and than return to the caller immediatly.

The run() function may call nested StateMachine (see case 2:)

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
             return StateReturnValue(1,ret,NULL);  // Successfully done, return ret to caller
};
```

Run the state machine by create an StateSet object and add it to the stack. The StateStack.run() function
has to be called periodically in a loop or task to perform the actions. The stateStack.run function will
allways call the stack.back function if the stack is not empty().

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
