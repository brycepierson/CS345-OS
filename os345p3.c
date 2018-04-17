// os345p3.c - Jurassic Park
// ***********************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the CS345 projects.          **
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>
#include "os345.h"
#include "os345park.h"

// ***********************************************************************
// project 3 variables

// Jurassic Park
extern JPARK myPark;
extern Semaphore* parkMutex;						// protect park access
extern Semaphore* fillSeat[NUM_CARS];			// (signal) seat ready to fill
extern Semaphore* seatFilled[NUM_CARS];		// (wait) passenger seated
extern Semaphore* rideOver[NUM_CARS];			// (signal) ride over
extern Semaphore* tics10thsec;
extern TCB tcb[];							// task control block

//MY SEMAPHORES
Semaphore* dcChange;  					// delta clock changing mutex
Semaphore* dcMutex;
Semaphore* getPassenger;
Semaphore* seatTaken;
Semaphore* passengerSeated;
Semaphore* needDriverMutex;
Semaphore* wakeupDriver;
Semaphore* vts[NUM_VISITORS];
Semaphore* dts[NUM_DRIVERS];
Semaphore* cts[NUM_CARS];
Semaphore* peopleInPark;
Semaphore* peopleInMuseum;
Semaphore* peopleInGiftShop;
Semaphore* sittingDown; //unused
Semaphore* mailbox;
Semaphore* mailboxFlag;
Semaphore* mailboxMutex;
Semaphore* tickets;
Semaphore* mailReady;
Semaphore* mailAcquired;
Semaphore* needPassenger;
Semaphore* needDriver;
Semaphore* needTicketSeller;
Semaphore* ticketSold;
Semaphore* carIDMutex;
Semaphore* requestCarID;
Semaphore* carIDDelivered;
Semaphore* needCarID;
Semaphore* callDriver;
Semaphore* carIDAcquired;
Semaphore* buyTicket;

int gcarID;

typedef struct DC
{
	int time;
	Semaphore* sem;
}DC;

int numDeltaClock = 0;
DC deltaClock[MAX_TASKS];
int timeTaskID;
Semaphore* event[MAX_TASKS];

// ***********************************************************************
// project 3 functions and tasks
void CL3_project3(int, char**);
void CL3_dc(int, char**);
int dcMonitorTask(int, char**);
int visitorRoutine(int, char**);
int carRoutine(int, char**);
int driverRoutine(int, char**);
int timeTask(int, char**);
int dcRoutine(int, char**);
void insertDeltaClock(int, Semaphore*);
void printDeltaClock();





// ***********************************************************************
// ***********************************************************************
// project3 command
int P3_project3(int argc, char* argv[])
{
	srand(time(0));							SWAP;
	char buf[32];							SWAP;
	char buf2[32];							SWAP;
	char* newArgv[2];							SWAP;

	// start park
	sprintf(buf, "jurassicPark");							SWAP;
	newArgv[0] = buf;							SWAP;
	createTask( buf,				// task name
		jurassicTask,				// task
		MED_PRIORITY,				// task priority
		1,								// task count
		newArgv);				SWAP;// task argument

	//create SEMAPHORES
	dcChange = createSemaphore("Delta Clock Change", BINARY, 0);							SWAP;
	dcMutex = createSemaphore("Delta Clock Mutex", BINARY, 1);							SWAP;
	getPassenger = createSemaphore("Get Passenger", BINARY, 0);							SWAP;
	seatTaken = createSemaphore("Seat Taken", BINARY, 0);							SWAP;
	passengerSeated = createSemaphore("PassengerSeated", BINARY, 0);							SWAP;
	needDriverMutex = createSemaphore("Need Driver Mutex", BINARY, 1);							SWAP;
	wakeupDriver = createSemaphore("Wake up Driver", BINARY, 0);							SWAP;
	peopleInPark = createSemaphore("People in Park", COUNTING, MAX_IN_PARK);							SWAP;
	peopleInMuseum = createSemaphore("People in Museum", COUNTING, MAX_IN_MUSEUM);							SWAP;
	peopleInGiftShop = createSemaphore("People in Gift Shop", COUNTING, MAX_IN_GIFTSHOP);							SWAP;
	mailAcquired = createSemaphore("something in the mailbox", BINARY, 0);							SWAP;
	mailboxMutex = createSemaphore("mailbox mutex", BINARY, 1);							SWAP;
	needPassenger = createSemaphore("need Passenger", BINARY, 0);							SWAP;
	mailReady = createSemaphore("mail is ready", BINARY, 0);							SWAP;
	tickets = createSemaphore("tickets", COUNTING, MAX_TICKETS);							SWAP;
	needDriver = createSemaphore("driver needed", BINARY, 0);							SWAP;
	needTicketSeller = createSemaphore("ticket seller needed", BINARY, 0);							SWAP;
	ticketSold = createSemaphore("ticket sold", BINARY, 0);							SWAP;
	carIDMutex = createSemaphore("car ID mutex", BINARY, 1);							SWAP;
	requestCarID = createSemaphore("driver requests car ID", BINARY, 0);							SWAP;
	carIDDelivered = createSemaphore("car ID is delivered", BINARY, 0);							SWAP;
	needCarID = createSemaphore("need car ID", BINARY, 0);							SWAP;
	callDriver = createSemaphore("calling for driver", BINARY, 0);							SWAP;
	carIDAcquired = createSemaphore("car id acquired", BINARY, 0);							SWAP;
	buyTicket = createSemaphore("buying ticket", BINARY, 0);					SWAP;
	// wait for park to get initialized...
	while (!parkMutex) SWAP;
	printf("\nStart Jurassic Park...");
	sprintf(buf, "Delta Clock");
	createTask( buf,
		dcRoutine,
		HIGH_PRIORITY,
		argc,
		argv);			SWAP;

	//?? create car, driver, and visitor tasks here
	for (int i = 0; i < NUM_CARS; i++){
		int argc = 1;									SWAP;
		char* argv[argc];									SWAP;
		sprintf(buf2, "%d", i);									SWAP;
		argv[0] = buf2;										SWAP;
		sprintf(buf, "Car Task%d", i);					SWAP;
		createTask(
			buf,
			carRoutine,
			MED_PRIORITY,
			argc,
			argv
		);									SWAP;
	}

	for (int i = 0; i < NUM_DRIVERS; i++){
		//create driver task
		int argc = 1;							SWAP;
		char* argv[argc];							SWAP;
		sprintf(buf2, "%d", i);							SWAP;
		argv[0] = buf2;							SWAP;
		sprintf(buf, "Driver %d", i);							SWAP;
		dts[i] = createSemaphore(buf, BINARY, 0);									SWAP;
		sprintf(buf, "Driver Task%d", i);							SWAP;
		createTask(
			buf,
			driverRoutine,
			MED_PRIORITY,
			argc,
			argv
		);									SWAP;
	}

	for (int i = 0; i < NUM_VISITORS; i++){
		//create visitor task
		int argc = 1;									SWAP;
		char* argv[argc];									SWAP;
//		sprintf(argv[0], "%d", i);									SWAP;
		sprintf(buf, "Visitor%d", i);									SWAP;
		vts[i] = createSemaphore(buf, BINARY, 0);									SWAP;
		sprintf(buf, "Visitor Task%d", i);						SWAP;
		sprintf(buf2, "%d", i);							SWAP;
		argv[0] = buf2;							SWAP;
		createTask(
			buf,
			visitorRoutine,
			MED_PRIORITY,
			argc,
			argv
		);									SWAP;


	}


	return 0;
} // end project3


int visitorRoutine(int argc, char* argv[]){
	int visitorID = atoi(argv[0]);							SWAP;
//	printf("visitor ID: %d", visitorID);						SWAP;
	insertDeltaClock((rand() % 90) + 10, vts[visitorID]);			SWAP;
	semWait(vts[visitorID]);							SWAP;
	semWait(parkMutex);							SWAP;
	myPark.numOutsidePark++;							SWAP;
	semSignal(parkMutex);							SWAP;
	//try to enter park

	insertDeltaClock((rand() % WAIT_TIME) + 10, vts[visitorID]);							SWAP;
	semWait(vts[visitorID]);							SWAP;
	semWait(peopleInPark);							SWAP;
	semWait(parkMutex);							SWAP;
	myPark.numOutsidePark--;							SWAP;
	myPark.numInPark++;							SWAP;
	myPark.numInTicketLine++;				SWAP;
	semSignal(parkMutex);							SWAP;

	//get tickets
	insertDeltaClock((rand() % WAIT_TIME) + 10, vts[visitorID]);							SWAP;
	semWait(vts[visitorID]);							SWAP;
	semWait(tickets);							SWAP;
	semWait(needDriverMutex);							SWAP;

	semSignal(needTicketSeller);							SWAP;
	semSignal(wakeupDriver);							SWAP;
	insertDeltaClock((rand() % WAIT_TIME) + 10, vts[visitorID]);							SWAP;
	semWait(vts[visitorID]);							SWAP;
	semSignal(buyTicket);							SWAP;
	semWait(ticketSold);							SWAP;

	semSignal(needDriverMutex);							SWAP;
	semWait(parkMutex);							SWAP;
	myPark.numInTicketLine--;							SWAP;
	myPark.numInMuseumLine++;							SWAP;
	semSignal(parkMutex);							SWAP;


	//go to Museum
	insertDeltaClock((rand() % WAIT_TIME) + 10, vts[visitorID]);							SWAP;
	semWait(vts[visitorID]);							SWAP;
	//wake up driver

	semWait(peopleInMuseum);							SWAP;
	semWait(parkMutex);							SWAP;
	myPark.numInMuseumLine--;							SWAP;
	myPark.numInMuseum++;							SWAP;
	semSignal(parkMutex);							SWAP;
	insertDeltaClock((rand() % WAIT_TIME) + 10, vts[visitorID]);							SWAP;
	semWait(vts[visitorID]);							SWAP;
	semWait(parkMutex);							SWAP;
	myPark.numInMuseum--;							SWAP;
	myPark.numInCarLine++;							SWAP;
	semSignal(parkMutex);							SWAP;
	semSignal(peopleInMuseum);				SWAP;


	//go on ride
	insertDeltaClock((rand() % WAIT_TIME) + 10, vts[visitorID]);							SWAP;
	semWait(vts[visitorID]);							SWAP;
	semWait(getPassenger);					SWAP;
	semWait(parkMutex);				SWAP;
	myPark.numInCarLine--;				SWAP;
	myPark.numInCars++;				SWAP;
	semSignal(parkMutex);				SWAP;
	semSignal(seatTaken);						SWAP;

	semWait(mailboxMutex);			 SWAP;	// wait for mailbox
	semWait(needPassenger); 				SWAP;	// wait for passenger request
	mailbox = vts[visitorID]; 		SWAP;	// put semaphore in mailbox
	semSignal(mailReady); 				SWAP;	// raise the mailbox flag
	semWait(mailAcquired);				 SWAP;	// wait for delivery
	semSignal(mailboxMutex); 				SWAP;	// release mailbox
	semWait(parkMutex);							SWAP;
	myPark.numTicketsAvailable++;							SWAP;
	semSignal(parkMutex);							SWAP;
	semSignal(tickets);							SWAP;

	semWait(vts[visitorID]);					SWAP;

	//go to gift shop

	semWait(parkMutex);							SWAP;
	myPark.numInCars--;							SWAP;
	myPark.numInGiftLine++;							SWAP;
	semSignal(parkMutex);							SWAP;
	insertDeltaClock((rand() % WAIT_TIME) + 10, vts[visitorID]);							SWAP;
	semWait(vts[visitorID]);							SWAP;
	semWait(peopleInGiftShop);							SWAP;
	semWait(parkMutex);							SWAP;
	myPark.numInGiftLine--;							SWAP;
	myPark.numInGiftShop++;							SWAP;
	semSignal(parkMutex);							SWAP;
	insertDeltaClock((rand() % WAIT_TIME) + 10, vts[visitorID]);							SWAP;
	semWait(vts[visitorID]);							SWAP;
	semWait(parkMutex);							SWAP;
	myPark.numInPark--;							SWAP;
	myPark.numExitedPark++;							SWAP;
	myPark.numInGiftShop--;							SWAP;
	semSignal(parkMutex);							SWAP;
	semSignal(peopleInGiftShop);							SWAP;
	semSignal(peopleInPark);							SWAP;

	//exit park
	return 0;
}

int driverRoutine(int argc, char* argv[]){
	int driverID = atoi(argv[0]);							SWAP;
	while(1)
	{
		semWait(parkMutex);							SWAP;
		myPark.drivers[driverID] = 0;							SWAP;
		semSignal(parkMutex);							SWAP;
		semWait(wakeupDriver);							SWAP;
		if(semTryLock(needTicketSeller)){
			semWait(parkMutex);							SWAP;
			myPark.numTicketsAvailable--;							SWAP;
			myPark.drivers[driverID] = -1;							SWAP;
			semSignal(parkMutex);							SWAP;
			semWait(buyTicket);							SWAP;
			semSignal(ticketSold);							SWAP;
		}
		else{
			semWait(callDriver);							SWAP;
			//drive
			semSignal(needCarID);						SWAP;
			semWait(carIDDelivered);						SWAP;
			semWait(parkMutex);						SWAP;
			myPark.drivers[driverID] = gcarID + 1;						SWAP;
			semSignal(parkMutex);						SWAP;
			semSignal(carIDAcquired);						SWAP;
			//send dts to car;
			// pass semaphore to car (1 at a time)
			semWait(mailboxMutex); SWAP;	// wait for mailbox
			semWait(needDriver); SWAP;	// wait for passenger request
			mailbox = dts[driverID]; SWAP;	// put semaphore in mailbox
			semSignal(mailReady); SWAP;	// raise the mailbox flag
			semWait(mailAcquired); SWAP;	// wait for delivery
			semSignal(mailboxMutex); SWAP;	// release mailbox

			semWait(dts[driverID]);						SWAP;
		}
	}
	return 0;
}

int carRoutine(int argc, char* argv[]){
	int carID = atoi(argv[0]);							SWAP;
	Semaphore* pRideFinished[NUM_SEATS];							SWAP;
	Semaphore* dRideFinished;							SWAP;
	while(1){
		for(int i = 0; i < NUM_SEATS; i++){
			semWait(fillSeat[carID]);						SWAP;
			semSignal(getPassenger);						SWAP;
			semWait(seatTaken);									SWAP;


			//save passenger ride over semaphores
			semSignal(needPassenger); SWAP;
			semWait(mailReady); SWAP;	// wait for mail
			pRideFinished[i] = mailbox; SWAP;	// get mail
			semSignal(mailAcquired); SWAP;	// put flag down

			semSignal(passengerSeated);					SWAP;
			if(i == NUM_SEATS - 1)
			{
				semWait(needDriverMutex);					SWAP;
				semSignal(callDriver);						SWAP;
				semSignal(wakeupDriver);					SWAP;

				//save driver ride over semaphore
				semWait(carIDMutex);						SWAP;
				semWait(needCarID);						SWAP;
				gcarID = carID;						SWAP;
				semSignal(carIDDelivered);						SWAP;
				semWait(carIDAcquired);						SWAP;
				semSignal(carIDMutex);						SWAP;

				semSignal(needDriver);						SWAP;
				semWait(mailReady);						SWAP;
				dRideFinished = mailbox;						SWAP;
				semSignal(mailAcquired);						SWAP;

				semSignal(needDriverMutex);				SWAP;
			}
			semSignal(seatFilled[carID]);				SWAP;
		}
		semWait(rideOver[carID]);							SWAP;
		//release passengers and driver
		for(int i = 0; i < NUM_SEATS; i++){
			semSignal(pRideFinished[i]);							SWAP;
		}
		semSignal(dRideFinished);							SWAP;
	}

	return 0;
}

//DELTA CLOCK INSERT AND REMOVE

void insertDeltaClock(int time, Semaphore* sem){
	semWait(dcMutex);							SWAP;
	int i = 0;							SWAP;
	for(i = 0;i < numDeltaClock; i++){
		time = time - deltaClock[i].time;							SWAP;
		if(time < 0)
		{
			break;							SWAP;
		}

	}

	if(time >= 0)
	{
		//insert element at the end of the clock
		deltaClock[i].time = time;							SWAP;
		deltaClock[i].sem = sem;							SWAP;
	}
	else
	{
		//move all elements up one block
		for(int j = numDeltaClock; j > i; j--){
			deltaClock[j] = deltaClock[j - 1];							SWAP;
		}
		//insert new element in middle of structure
		deltaClock[i].time = time + deltaClock[i+1].time;							SWAP;
		deltaClock[i].sem = sem;							SWAP;
		//update element directly following
		deltaClock[i + 1].time = abs(time);							SWAP;
	}
	//update number of elements in deltaClock
	numDeltaClock++;							SWAP;
	semSignal(dcChange);							SWAP;
	semSignal(dcMutex);							SWAP;
}

int dcRoutine(int argc, char** argv){
	for(int i = 0; i < MAX_TASKS; i++){
		deltaClock[i].time = 0;							SWAP;
	}
	numDeltaClock = 0;
	while(1){
		semWait(dcMutex);							SWAP;
		semWait(tics10thsec);							SWAP;
		if(numDeltaClock > 0){
			deltaClock[0].time--;							SWAP;
			while(deltaClock[0].time <= 0 && numDeltaClock > 0){
				semSignal(deltaClock[0].sem);							SWAP;
				for(int i = 0; i < numDeltaClock; i++)
				{
					deltaClock[i] = deltaClock[i + 1];							SWAP;
				}
				numDeltaClock--;							SWAP;
				semSignal(dcChange);							SWAP;
			}
		}
		semSignal(dcMutex);							SWAP;
	}
}


// ***********************************************************************
// ***********************************************************************
/*
// delta clock command
int P3_dc(int argc, char* argv[])
{
	printf("\nDelta Clock");
	// ?? Implement a routine to display the current delta clock contents
	printf("\nTo Be Implemented!");
	return 0;
} // end CL3_dc
*/


// ***********************************************************************
// ***********************************************************************
// ***********************************************************************
// ***********************************************************************
// ***********************************************************************
// ***********************************************************************
// delta clock command

int P3_dc(int argc, char* argv[])
{
	// ?? Implement a routine to display the current delta clock contents
	//printf("\nTo Be Implemented!");
	printDeltaClock();
	return 0;
} //end CL3_dc


// ***********************************************************************
// display all pending events in the delta clock list
void printDeltaClock(void)
{
	printf("\nDelta Clock\nsize:%d", numDeltaClock);

	int i;
	for (i=0; i<numDeltaClock; i++)
	{
		printf("\n%4d%4d  %-20s", i, deltaClock[i].time, deltaClock[i].sem->name);
	}
	return;
}


// ***********************************************************************
// test delta clock
int P3_tdc(int argc, char* argv[])
{
	dcChange = createSemaphore("Delta Clock Change", BINARY, 0);
	dcMutex = createSemaphore("Delta Clock Mutex", BINARY, 1);
	createTask( "Delta Clock",
		dcRoutine,
		HIGH_PRIORITY,
		argc,
		argv);

	createTask( "DC Test",			// task name
		dcMonitorTask,		// task
		10,					// task priority
		argc,					// task arguments
		argv);

	timeTaskID = createTask( "Time",		// task name
		timeTask,	// task
		10,			// task priority
		argc,			// task arguments
		argv);
	return 0;
} // end P3_tdc



// ***********************************************************************
// monitor the delta clock task
int dcMonitorTask(int argc, char* argv[])
{
	int i, flg;
	char buf[32];
	// create some test times for event[0-9]
	int ttime[10] = {
		90, 300, 50, 170, 340, 300, 50, 300, 40, 110	};

	for (i=0; i<10; i++)
	{
		sprintf(buf, "event[%d]", i);
		event[i] = createSemaphore(buf, BINARY, 0);
		insertDeltaClock(ttime[i], event[i]);
	}
	printDeltaClock();
	while (numDeltaClock > 0)
	{
		semWait(dcChange);
		flg = 0;
		for (i=0; i<10; i++)
		{
			if (event[i]->state ==1)			{
					printf("\n  event[%d] signaled", i);
					event[i]->state = 0;
					flg = 1;
				}
		}
		if (flg) printDeltaClock();
	}
	printf("\nNo more events in Delta Clock");

	// kill dcMonitorTask
	tcb[timeTaskID].state = S_EXIT;
	return 0;
} // end dcMonitorTask


extern Semaphore* tics1sec;

// ********************************************************************************************
// display time every tics1sec
int timeTask(int argc, char* argv[])
{
	char svtime[64];						// ascii current time
	while (1)
	{
		semWait(tics1sec);
		printf("\nTime = %s", myTime(svtime));
	}
	return 0;
} // end timeTask
