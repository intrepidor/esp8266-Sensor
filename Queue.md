AVRQueue
========

AVRQueue is a task queuing library for AVR and Arduino processors.  Included here are two different branches, **AVR** and **Arduino**.

This is a simple queuing library.  Add functions to the queue, along with a unique identifier, an initial run time, and a recurrence time.  If your time scale is in seconds, an item with an initial run time of 1, and a recurrence of 5 will run after time 1 second, and every 5 seconds thereafter.

Functions are not guaranteed to execute deterministic when they're scheduled.  This is a very primitive library.  The run function simply waits for a function in the queue to reach its run time, runs the function, then continues through the queue.  If queued functions take a very long time to execute, then functions may not execute precisely when desired.  For this reason, a "now" variable is passed into every function in order to let it know when it actually runs.

AVR
---

The AVR library is oriented towards enthusiasts.  Compile **queue.c** with your project, and include **queue.h** in any files that need to add, modify, remove, or execute functions from the queue.

All settings are in **queue.h**:

* ***QUEUE_DEPTH***:  The number of functions that you can store in the queue.  More functions = more RAM required. (_Default:_ 10)

* ***NAME_LIMIT***: The number of characters used to store unique IDs for tasks. (_Default:_ 8)

* ***queue_time_t***: Typedef for the storage of timing data.  Use the native type for whatever time system you're using. (If you're using AVR timers, change this to uint_16t). (_Default:_ uint_32t)

### Usage

* ***Queued Functions***: All queued functions must have the following C signature:
> int queued\_function(queue\_time\_t now)<br />
> The _now_ variable will contain the time the function was called.

* int scheduleFunction(queuedFunction **pFunction**, const char **pId**, queue\_time\_t **pInitialRun**, queue\_time\_t **pRecur**)
> Add a function to the queue.<br/>
> ***Return Values:***<br/>
>> **0** if successful.<br/>
>> **-1** if unsuccessful (queue is full).<br/>

 * **pFunction**: Function to put in the queue.
 * **pId**: Up to 8 character unique identifier. (Unless you change the length).
 * **pInitialRun**: Initial run time, in whatever time units you feed into **run**.
 * **pRecur**: Recurrence period, in whatever time units you feed into **run**.

* int scheduleRemoveFunction(const char **pId**)
> Remove a function with given unique ID from the queue.<br />
> ***Return Values:***<br/>
>> **0** if successful. <br/>
>> **-1** if unsuccessful.

 * **pId**: Unique id 

* int scheduleChangeFunction(const char **pId**, queue_time_t **pNewNext**, queue_time_t **pNewRecur**)
> Change function in the queue.<br/>
> ***Return Values:***<br/>
>> **0** if successful. <br/>
>> **-1** if unsuccessful.

 * **pId**: Unique identifier.
 * **pNewNext**: Next time to run the function.
 * **pNewRecur**: New recurrence time.

* int scheduleRun(queue_time_t **pNow**)
> Run all the queued items.  Run this in a loop to execute the queue.<br/>
> ***Return Values:***<br/>
>> **-1** if unsuccessful.
>> **Number of queue items successfully ran** otherwise.

 * **pNow**: the current time.

Arduino
-------

The Arduino library is much easier to use.  Extract to <Arduino Root>/Libraries/Queue/, restart the IDE, and start using.

* Maximum of 10 items in the queue.
* Unique IDs are up to 8 characters, no more.
* Use with ```millis();```

### Usage

* ***Queued Functions***: All queued functions must have a C signature (either not class-members, or static class-members) of:
> int queued\_function(unsigned long now)<br />
> The _now_ variable will contain the time the function was called.

* ***Creating a Queue***: Create a queue like this: ```Queue myQueue;```

* int Queue::scheduleFunction(queuedFunction **pFunction**, const char **pId**, unsigned long **pInitialRun**, unsigned long **pRecur**)
> Add a function to the queue.<br/>
> ***Return Values:***<br/>
>> **0** if successful.<br/>
>> **-1** if unsuccessful (queue is full).<br/>

 * **pFunction**: Function to put in the queue.
 * **pId**: Up to 8 character unique identifier. (Unless you change the length).
 * **pInitialRun**: Initial run time, in milliseconds.
 * **pRecur**: Recurrence period, in milliseconds.

* int Queue::scheduleRemoveFunction(const char **pId**)
> Remove a function with given unique ID from the queue.<br />
> ***Return Values:***<br/>
>> **0** if successful. <br/>
>> **-1** if unsuccessful.

 * **pId**: Unique id 

* int Queue::scheduleChangeFunction(const char **pId**, unsigned long **pNewNext**, unsigned long **pNewRecur**)
> Change function in the queue.<br/>
> ***Return Values:***<br/>
>> **0** if successful. <br/>
>> **-1** if unsuccessful.

 * **pId**: Unique identifier.
 * **pNewNext**: Next time to run the function.
 * **pNewRecur**: New recurrence time.

* int Queue::scheduleRun(unsigned long **pNow**)
> Run all the queued items.  Run this in a loop to execute the queue.<br/>
> ***Return Values:***<br/>
>> **-1** if unsuccessful.
>> **Number of queue items successfully ran** otherwise.

 * **pNow**: the current time, in Milliseconds.

 License
 -------

 Copyright (c) 2012 Brad Luyster

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.