/* Zuph / AVRQueue
 * Queueing Library for AVR and Arduino
 * Copyright (c) 2012 Brad Luyster
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * https://github.com/Zuph/AVRQueue/tree/master/Arduino
 * Copied from github on 2016Jan23. Some small changes were made.
 */

// Modified by Allan Inda to use String class instead of char* buffers.
#ifndef QUEUE_H
#define QUEUE_H

#include <Arduino.h>

typedef int (*queuedFunction)(unsigned long);

const int QUEUE_SCHEDULE_SIZE = 11; /*lint -e551 Don't warning about not accessing this variable */
const int QUEUE_RETURNCODE_NOERROR = 0;
const int QUEUE_RETURNCODE_ERROR = -1;

struct queueItem {
	queuedFunction fPtr;
	unsigned long next;
	unsigned long recur;
	String itemName;
	//char itemName[ITEM_NAME_BUFFER_SIZE];
};

class Queue {
private:
	unsigned int _queueStart;
	unsigned int _queueEnd;
	unsigned int _itemsInQueue;
	queueItem _schedule[QUEUE_SCHEDULE_SIZE];

	int _queueGetTop(queueItem &item);
	int _addToQueue(queueItem item);

public:
	Queue();

	int scheduleFunction(queuedFunction func, String id, unsigned long initialRun, unsigned long recur);
	int scheduleRemoveFunction(String id);
	int scheduleChangeFunction(String id, unsigned long nextRunTime, unsigned long newRecur);

	int Run(unsigned long now);
	/* data */
};

#endif
