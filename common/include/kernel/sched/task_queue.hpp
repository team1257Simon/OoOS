#ifndef __TASK_QUEUE
#define __TASK_QUEUE
#include "bits/stl_queue.hpp"
#include "sched/task.h"
/**
 * Each priority level will have its own queue. When a scheduling event fires (e.g. the current process' timeslice expires),
 * if there are waiting processes, its "next" pointer is updated to the next process "in line" based on the following:
 *  1: If the queue with priority "system" has any active tasks, that queue will be accessed first.
 *  2: Otherwise, the highest numeric priority takes precedence. The "times skipped" value of the chosen non-system process is reset to zero at this point.
 *  3: If the process was selected from a queue of higher priority than its base value, it moves to the back of the queue one loweer than its current queue.
 *  4: Otherwise, the task queue element is "popped" and the pointer moves to the next element (or to the beginning if it is at the end). 
 *  5: The "times skipped" value of the front process in each queue of lower priority than the one accessed is incremented.
 *  6: If the above value exceeds the threshold (tbd), the process is moved to the back of the queue above its current queue.
 */
#endif