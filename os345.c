// os345.c - OS Kernel	09/12/2013
// ***********************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the BYU CS345 projects.      **
// ** It comes "as is" and "unwarranted."  As such, when you use part   **
// ** or all of the code, it becomes "yours" and you are responsible to **
// ** understand any algorithm or method presented.  Likewise, any      **
// ** errors or problems become your responsibility to fix.             **
// **                                                                   **
// ** NOTES:                                                            **
// ** -Comments beginning with "// ??" may require some implementation. **
// ** -Tab stops are set at every 3 spaces.                             **
// ** -The function API's in "OS345.h" should not be altered.           **
// **                                                                   **
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// ***********************************************************************

//#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>

#include "os345.h"
#include "os345signals.h"
#include "os345config.h"
#include "os345lc3.h"
#include "os345fat.h"

// **********************************************************************
//	local prototypes
//
void pollInterrupts(void);
static int scheduler(void);
void assignTime(int, int);
static int dispatcher(void);

//static void keyboard_isr(void);
//static void timer_isr(void);

int sysKillTask(int taskId);
static int initOS(void);

// **********************************************************************
// **********************************************************************
// global semaphores

Semaphore* semaphoreList;			// linked list of active semaphores

Semaphore* keyboard;				// keyboard semaphore
Semaphore* charReady;				// character has been entered
Semaphore* inBufferReady;			// input buffer ready semaphore

Semaphore* tics1sec;				// 1 second semaphore
Semaphore* tics10thsec;				// 1/10 second semaphore
Semaphore* tics10sec;				// 10 second semaphore for p2


// **********************************************************************
// **********************************************************************
// global system variables

TCB tcb[MAX_TASKS];					// task control block
Semaphore* taskSems[MAX_TASKS];		// task semaphore
jmp_buf k_context;					// context of kernel stack
jmp_buf reset_context;				// context of kernel stack
volatile void* temp;				// temp pointer used in dispatcher

int scheduler_mode;					// scheduler mode
int superMode;						// system mode
int curTask;						// current task #
long swapCount;						// number of re-schedule cycles
char inChar;						// last entered character
int charFlag;						// 0 => buffered input
int inBufIndx;						// input pointer into input buffer
char inBuffer[INBUF_SIZE+1];		// character input buffer
//Message messages[NUM_MESSAGES];		// process message buffers

int pollClock;						// current clock()
int lastPollClock;					// last pollClock
bool diskMounted;					// disk has been mounted

time_t oldTime1;					// old 1sec time
time_t oldTime10;					// old 10sec time
clock_t myClkTime;
clock_t myOldClkTime;
struct pqueue rq;							// ready priority queue


// **********************************************************************
// **********************************************************************
// OS startup
//
// 1. Init OS
// 2. Define reset longjmp vector
// 3. Define global system semaphores
// 4. Create CLI tasknextTaskIndex
// 5. Enter scheduling/idle loop
//
int main(int argc, char* argv[])
{
	// save context for restart (a system reset would return here...)
	int resetCode = setjmp(reset_context);
	superMode = TRUE;						// supervisor mode

	switch (resetCode)
	{
		case POWER_DOWN_QUIT:				// quit
			powerDown(0);
			printf("\nGoodbye!!");
			return 0;

		case POWER_DOWN_RESTART:			// restart
			powerDown(resetCode);
			printf("\nRestarting system...\n");

		case POWER_UP:						// startup
			break;

		default:
			printf("\nShutting down due to error %d", resetCode);
			powerDown(resetCode);
			return resetCode;
	}

	// output header message
	printf("%s", STARTUP_MSG);

	// initalize OS
	if ( resetCode = initOS()) return resetCode;

	// create global/system semaphores here
	//?? vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

	charReady = createSemaphore("charReady", BINARY, 0);
	inBufferReady = createSemaphore("inBufferReady", BINARY, 0);
	keyboard = createSemaphore("keyboard", BINARY, 1);
	tics1sec = createSemaphore("tics1sec", BINARY, 0);
	tics10thsec = createSemaphore("tics10thsec", BINARY, 0);
	tics10sec = createSemaphore("tics10sec", COUNTING, 0);

	//?? ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// schedule CLI task
	createTask("myShell",			// task name
					P1_shellTask,	// task
					MED_PRIORITY,	// task priority
					argc,			// task arg count
					argv);			// task argument pointers

	// HERE WE GO................

	// Scheduling loop
	// 1. Check for asynchronous events (character inputs, timers, etc.)
	// 2. Choose a ready task to schedule
	// 3. Dispatch task
	// 4. Loop (forever!)

	while(1)									// scheduling loop
	{
		// check for character / timer interrupts
		pollInterrupts();

		// schedule highest priority ready tasknextTaskIndex
		if ((curTask = scheduler()) < 0) continue;

		// dispatch curTask, quit OS if negative return
		if (dispatcher() < 0) break;
	}											// end of scheduling loop

	// exit os
	longjmp(reset_context, POWER_DOWN_QUIT);
	return 0;
} // end main



// **********************************************************************
// **********************************************************************
// scheduler
//
static int scheduler()
{
	struct pqElement nextTask;
	int nextTaskIndex = -1;
	// ?? Design and implement a scheduler that will select the next highest
	// ?? priority ready task to pass to the system dispatcher.

	// ?? WARNING: You must NEVER call swapTask() from within this function
	// ?? or any function that it calls.  This is because swapping is
	// ?? handled entirely in the swapTask function, whnextTaskIndexich, in turn, may
	// ?? call this function.  (ie. You would create an infinite loop.)

	// ?? Implement a round-robin, preemptive, prioritized scheduler.

	// ?? This code is simply a round-robin scheduler and is just to get
	// ?? you thinking about scheduling.  You must implement code to handle
	// ?? priorities, clean up dead tasks, and handle semaphores appropriately.

	if(scheduler_mode == 0){
		if(!empty(&rq))
		{
			nextTask = dequeue(&rq, -1);
			nextTaskIndex = nextTask.tid;

			// mask sure nextTask is valid
			enqueue(&rq, nextTask);
		}

		if(tcb[nextTaskIndex].signal & mySIGSTOP)
			return -1;
		return nextTaskIndex;
	}
	else if(scheduler_mode == 1){
		if(empty(&rq)){
			//printf("\nrq empty");
			assignTime(0, FSS_TOT_TIME);
		}
		nextTask = dequeue(&rq, -1);
		nextTaskIndex = nextTask.tid;
		if(tcb[nextTaskIndex].taskTime > 0){
			//printf("\ntask:%d time:%d", nextTaskIndex, tcb[nextTaskIndex].taskTime);
			//print(&rq);
			tcb[nextTaskIndex].taskTime--;
			enqueue(&rq, nextTask);
		}
		return nextTaskIndex;
	}
	// schedule next task
	/*
	// mask sure nextTaskIndex is valid
	while (!tcb[nextTaskIndex].name)
	{
		if (++nextTaskIndex >= MAX_TASKS) nextTaskIndex = 0;
	}
	if (tcb[nextTaskIndex].signal & mySIGSTOP) return -1;

	return nextTaskIndex;*/
} // end scheduler

void assignTime(int tid, int remainingTime){
	int childCount = 0;
	struct pqElement task;
	int children[MAX_TASKS];
	for(int i = 0; i < MAX_TASKS; i++){
		if(tcb[i].name && (tcb[i].parent == tid)){
			children[childCount] = i;
			childCount++;
		}
	}
	tcb[tid].taskTime = (remainingTime / (childCount + 1)) + (remainingTime % (childCount + 1));
	task.tid = tid;
	task.priority = tcb[tid].priority;
	enqueue(&rq, task);
	for(int i = 0; i < childCount; i++){
		assignTime(children[i], remainingTime / (childCount + 1));
	}

}

// **********************************************************************
// **********************************************************************
// dispatch curTask
//
static int dispatcher()
{
	int result;

	// schedule task
	switch(tcb[curTask].state)
	{
		case S_NEW:
		{
			// new task
			printf("\nNew Task[%d] %s", curTask, tcb[curTask].name);
			tcb[curTask].state = S_RUNNING;	// set task to run state

			// save kernel context for task SWAP's
			if (setjmp(k_context))
			{
				superMode = TRUE;					// supervisor mode
				break;								// context switch to next task
			}

			// move to new task stack (leave room for return value/address)
			temp = (int*)tcb[curTask].stack + (STACK_SIZE-8);
			SET_STACK(temp);
			superMode = FALSE;						// user mode

			// begin execution of new task, pass argc, argv
			result = (*tcb[curTask].task)(tcb[curTask].argc, tcb[curTask].argv);

			// task has completed
			if (result) printf("\nTask[%d] returned %d", curTask, result);
			else printf("\nTask[%d] returned %d", curTask, result);
			tcb[curTask].state = S_EXIT;			// set task to exit state

			// return to kernal mode
			longjmp(k_context, 1);					// return to kernel
		}

		case S_READY:
		{
			tcb[curTask].state = S_RUNNING;			// set task to run
		}

		case S_RUNNING:
		{
			if (setjmp(k_context))
			{
				// SWAP executed in task
				superMode = TRUE;					// supervisor mode
				break;								// return from task
			}
			if (signals()) break;
			longjmp(tcb[curTask].context, 3); 		// restore task context
		}

		case S_BLOCKED:
		{
			break;
		}

		case S_EXIT:
		{
			if (curTask == 0) return -1;			// if CLI, then quit scheduler
			// release resources and kill task
			sysKillTask(curTask);					// kill current task
			break;
		}

		default:
		{
			printf("Unknown Task[%d] State", curTask);
			longjmp(reset_context, POWER_DOWN_ERROR);
		}
	}
	return 0;
} // end dispatcher



// **********************************************************************
// **********************************************************************
// Do a context switch to next task.

// 1. If scheduling task, return (setjmp returns non-zero value)
// 2. Else, save current task context (setjmp returns zero value)
// 3. Set current task state to READY
// 4. Enter kernel mode (longjmp to k_context)

void swapTask()
{
	assert("SWAP Error" && !superMode);		// assert user mode

	// increment swap cycle counter
	swapCount++;

	// either save current task context or schedule task (return)
	if (setjmp(tcb[curTask].context))
	{
		superMode = FALSE;					// user mode
		return;
	}

	// context switch - move task state to ready
	if (tcb[curTask].state == S_RUNNING) tcb[curTask].state = S_READY;

	// move to kernel mode (reschedule)
	longjmp(k_context, 2);
} // end swapTask



// **********************************************************************
// **********************************************************************
// system utility functions
// **********************************************************************
// **********************************************************************

// **********************************************************************
// **********************************************************************
// initialize operating system
static int initOS()
{
	int i;

	// make any system adjustments (for unblocking keyboard inputs)
	INIT_OS

	// reset system variables
	curTask = 0;						// current task #
	swapCount = 0;						// number of scheduler cycles
	scheduler_mode = 0;					// default scheduler
	inChar = 0;							// last entered character
	charFlag = 0;						// 0 => buffered input
	inBufIndx = 0;						// input pointer into input buffer
	semaphoreList = 0;					// linked list of active semaphores
	diskMounted = 0;					// disk has been mounted

	// malloc ready queue
	rq.data = (struct pqElement*)malloc(MAX_TASKS * sizeof(struct pqElement));
	if (rq.data == NULL) return 99;
	// init ready queue
	initialize(&rq);
	// capture current time
	lastPollClock = clock();			// last pollClock
	time(&oldTime1);
	oldTime10 = oldTime1;


	// init system tcb's
	for (i=0; i<MAX_TASKS; i++)
	{
		tcb[i].name = NULL;				// tcb
		taskSems[i] = NULL;				// task semaphore
	}

	// init tcb
	for (i=0; i<MAX_TASKS; i++)
	{
		tcb[i].name = NULL;
	}

	// initialize lc-3 memory
	initLC3Memory(LC3_MEM_FRAME, 0xF800>>6);

	// ?? initialize all execution queues

	return 0;
} // end initOS



// **********************************************************************
// **********************************************************************
// Causes the system to shut down. Use this for critical errors
void powerDown(int code)
{
	int i;
	printf("\nPowerDown Code %d", code);

	// release all system resources.
	printf("\nRecovering Task Resources...");

	// kill all tasks
	for (i = MAX_TASKS-1; i >= 0; i--)
		if(tcb[i].name) sysKillTask(i);

	// delete all semaphores
	while (semaphoreList)
		deleteSemaphore(&semaphoreList);

	// free ready queue
	free(rq.data);

	// ?? release any other system resources
	// ?? deltaclock (project 3)

	RESTORE_OS
	return;
} // end powerDown

// **********************************************************************
// **********************************************************************
// Priority Queue for Scheduling
// Put a task into the queue;
void initialize(pqueue *p)
{
    p->rear=-1;
    p->front=-1;
}

int empty(pqueue *p)
{
    if(p->rear==-1)
        return(1);

    return(0);
}

int full(pqueue *p)
{
    if((p->rear+1)%MAX_TASKS==p->front)
        return(1);

    return(0);
}

void enqueue(pqueue *p, pqElement x)
{
    int i;
    if(full(p)){
        //printf("\nOverflow");
			}
    else
    {
        if(empty(p))
        {
            p->rear=p->front=0;
            p->data[0]=x;
        }
        else
        {
            i=p->rear;

            while(x.priority>p->data[i].priority)
            {
                p->data[(i+1)%MAX_TASKS]=p->data[i];
                i=(i-1+MAX_TASKS)%MAX_TASKS; //anticlockwise movement inside the queue
                if((i+1)%MAX_TASKS==p->front)
                    break;
            }

            //insert x
            i=(i+1)%MAX_TASKS;
            p->data[i]=x;

            //re-adjust rear
            p->rear=(p->rear+1)%MAX_TASKS;
        }
    }
}

struct pqElement dequeue(pqueue *p, int tid)
{
    struct pqElement x;
		x.tid = -1;
		if(tid == -1)
		{
	    if(empty(p))
	    {

	    }
	    else
	    {
	        x=p->data[p->front];
	        if(p->rear==p->front)   //delete the last element
	            initialize(p);
	        else
	            p->front=(p->front +1)%MAX_TASKS;
	    }
		}
		else
		{
			bool found = TRUE;
			if(empty(p)) found = FALSE;
			int i = p->rear;
			while(p->data[i].tid != tid)
			{
				i=(i-1+MAX_TASKS)%MAX_TASKS; //anticlockwise movement inside the queue
				if((i+1)%MAX_TASKS==p->front)
				{
					found = FALSE;
					break;
				}
			}
			if(found)
			{


				x = p->data[(i - 1 + MAX_TASKS)%MAX_TASKS];
				while(i != p->front)
				{
					p->data[i] = p->data[(i-1+MAX_TASKS)%MAX_TASKS];
					i = (i - 1 + MAX_TASKS)%MAX_TASKS;
				}


				if(p->rear==p->front)   //delete the last element
				{
						initialize(p);
				}
				else
						p->front=(p->front +1)%MAX_TASKS; //move the front 1 spot up
			}
		}
		return x;
}

void print(pqueue *p)
{
    int i;
		struct pqElement x;
    if(empty(p))
    {
        printf("\nQueue is empty..");
    }
    else
    {
        i=p->front;

        while(i!=p->rear)
        {
            x=p->data[i];
		        printf("\ntid:%d, priority:%d",x.tid, x.priority);
						if(i == p->front)
							printf(" <- FRONT");
            i=(i+1)%MAX_TASKS;
        }


        //prints the last element
        x=p->data[i];
        printf("\ntid:%d, priority:%d",x.tid, x.priority);
				printf(" <- REAR");
				printf("\nFront = %d, Rear = %d", p->front, p->rear);
    }
}

/*
void print(pqueue *p)
{
    int i;
		struct pqElement x;
    if(empty(p))
    {
        printf("\nQueue is empty..");
    }
    else
    {
        i=p->front;

				printf("\np->rear:%d", p->rear);
				printf("\np->front:%d", p->front);
        while(i!=p->rear)
        {
            x=p->data[i];
						printf("\ntid:%d priority:%d",x.tid, x.priority);
            i=(i+1)%MAX_TASKS;
        }

        //prints the last element
        x=p->data[i];
        printf("\ntid:%d priority:%d",x.tid, x.priority);
    }
}
*/
