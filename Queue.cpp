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

#include "Queue.h"

Queue::Queue() {
	_itemsInQueue = 0;
	_queueStart = 0;
	_queueEnd = 0;
	memset(_schedule, 0, sizeof(_schedule));
}

int Queue::scheduleFunction(queuedFunction func, String id, unsigned long initialRun, unsigned long recur) {
	queueItem newItem;
	newItem.fPtr = func;
	newItem.itemName = id;
	newItem.recur = recur;
	newItem.next = initialRun;
	return _addToQueue(newItem);
}

int Queue::scheduleRemoveFunction(String id) {
//l int --e{534}    ignore warning about ignoring return value from _addToQueue()
	queueItem target;
	int rv = QUEUE_RETURNCODE_ERROR;
	for (unsigned int i = 0; i < _itemsInQueue; ++i) {
		if (_queueGetTop(target) == 0) {
			if (target.itemName == id) {
				rv = QUEUE_RETURNCODE_NOERROR;
			}
			else {
				return _addToQueue(target);
			}
		}
		else {
			rv = QUEUE_RETURNCODE_ERROR;
			break;
		}
	}

	return rv;
}

int Queue::scheduleChangeFunction(String id, unsigned long nextRunTime, unsigned long newRecur) {
//l int --e{534}    ignore warning about ignoring return value from _addToQueue()
	queueItem target;
	int rv = QUEUE_RETURNCODE_ERROR;
	for (unsigned int i = 0; i < _itemsInQueue; ++i) {
		if (_queueGetTop(target) == 0) {
			if (target.itemName == id) {
				target.next = nextRunTime;
				target.recur = newRecur;
				rv = QUEUE_RETURNCODE_NOERROR;
			}
			return _addToQueue(target);
		}
		else {
			rv = QUEUE_RETURNCODE_ERROR;
			break;
		}
	}

	return rv;
}

int Queue::Run(unsigned long now) {
//l int --e{534}    ignore warning about ignoring return value from _addToQueue()
	queueItem target;
	int rv = QUEUE_RETURNCODE_NOERROR;
	if (_itemsInQueue == 0) {
		rv = QUEUE_RETURNCODE_ERROR;
	}
	for (unsigned int i = 0; i < _itemsInQueue; ++i) {
		if (_queueGetTop(target) == 0) {
			if (target.next <= now) {
				int tRv;
				tRv = (target.fPtr)(now);
				if (tRv == 0) {
					rv++;
				}
				if (target.recur != 0) {
					target.next = now + target.recur;
					return _addToQueue(target);
				}
			}
			else {
				return _addToQueue(target);
			}
		}
		else {
			rv = QUEUE_RETURNCODE_ERROR;
			break;
		}
	}

	return rv;
}

int Queue::_queueGetTop(queueItem &item) {
	int rv = QUEUE_RETURNCODE_NOERROR;
	//Remove the top item, stuff it into item
	if (_queueEnd != _queueStart) {
		queueItem tempQueueItem = _schedule[_queueStart];
		//This Algorithm also from Wikipedia.
		_queueStart = (_queueStart + 1) % QUEUE_SCHEDULE_SIZE;
		item = tempQueueItem;
		_itemsInQueue--;
	}
	else {
		//if the buffer is empty, return an error code
		rv = QUEUE_RETURNCODE_ERROR;
	}
	return rv;
}

int Queue::_addToQueue(queueItem item) {
//lint --e{1746}    ignore information about making item a const reference
	//This is just a circular buffer, and this algorithm is stolen from wikipedia
	int rv = QUEUE_RETURNCODE_NOERROR;
	if ((_queueEnd + 1) % QUEUE_SCHEDULE_SIZE != _queueStart) {
		_schedule[_queueEnd] = item;
		_queueEnd = (_queueEnd + 1) % QUEUE_SCHEDULE_SIZE;
		_itemsInQueue++;
	}
	else {
		//if buffer is full, error
		rv = QUEUE_RETURNCODE_ERROR;
	}
	return rv;
}

unsigned long Queue::getTimeInterval(String id) {
	for (int i = 0; i < QUEUE_SCHEDULE_SIZE; ++i) {
		if (_schedule[i].itemName == id) {
			return _schedule[i].recur;
		}
	}
	return QUEUE_RETURNCODE_ID_NOT_FOUND;
}

unsigned long Queue::getTimeTillRun(String id) {
	for (int i = 0; i < QUEUE_SCHEDULE_SIZE; ++i) {
		if (_schedule[i].itemName == id) {
			return _schedule[i].next;
		}
	}
	return QUEUE_RETURNCODE_ID_NOT_FOUND;
}

