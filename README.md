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

