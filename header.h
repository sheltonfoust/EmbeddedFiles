
#ifndef LIBRARY_H_
#define LIBRARY_H_

#define VERSION "4.3"
#define ASSIGNMENT 4
#define AUTHOR "Shelton Foust"
#define MSGLEN 99
#define NUMGPIOS 8
#define NUMCALLBACKS 4
#define NUMMSGS 10
#define NUMPAYLOADS 32

#define MSG_OVERFLOW 0
#define INVALID_OPERATION 1
#define WRONG_TYPE 2
#define BAD_NUMBER 3
#define PAYLOAD_Q_OVERFLOW 4
#define NUMERRORS 5


#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/gates/GateSwi.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/Timer.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <stdbool.h>


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

extern Task_Handle task0;
extern Task_Handle task1;
extern Task_Handle task2;
//extern Task_Handle task2;
extern Semaphore_Handle semaphore0;
extern Semaphore_Handle semaphore1;
extern Swi_Handle swi0;
//extern Swi_Handle swi1;
extern GateSwi_Handle gateSwi0;
extern GateSwi_Handle gateSwi1;
extern GateSwi_Handle gateSwi2;
extern GateSwi_Handle gateSwi3;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Structs


typedef struct _BiosStruct
{

    Task_Handle uartReadTask;
    Task_Handle interruptPayloadTask;
    Task_Handle uartPayloadTask;
    Semaphore_Handle triggerPayloadSemaphore;
    Semaphore_Handle uartPayloadSemaphore;
    Swi_Handle timer0Swi;
    //Swi_Handle switch0Swi;
    GateSwi_Handle callbackGate;
    GateSwi_Handle uartPayloadGate;
    GateSwi_Handle triggerPayloadGate;
    GateSwi_Handle msgWriteGate;

} BiosStruct;


typedef struct _Message
{
    int length;
    char str[MSGLEN];
} Message;


typedef struct _MessageQ
{
    int readingIndex;
    int writingIndex;
    Message messages[NUMMSGS];
} MessageQ;


typedef struct _PayloadQ
{
    int readingIndex;
    int writingIndex;
    char payloads[NUMPAYLOADS][MSGLEN];
} PayloadQ;


typedef struct _Callback
{
    int32_t index; // The index in the callback array
    int32_t count; // The number of times it goes off
    bool HWI_flag; // Used for voice
    char payload[MSGLEN];
} Callback;


typedef struct _GlobalStruct
{
    int timer0Period;
    int errorCounts[NUMERRORS];
    BiosStruct bios;
    UART_Handle uart;
    Timer_Handle timer0;
    Message uartMessage;
    PayloadQ triggerPayloadQ;
    PayloadQ uartPayloadQ;
    Callback callbacks[NUMCALLBACKS];
} GlobalStruct;


// Structs
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GlobalStruct Glo;



void aboutParse();
void errorParse(char* input);
void errorBase();
void gpioBase();
void helpBase();
void parseMSG(char *messageLine);
void memoryReadParse(char* input);
void printParse(char* input);
void readGPIOprint(int gpio);
void writeGPIOprint(int gpioNum, char* remainder);
void removeSpaces(char* noSpaces, const char* input);
void gpioParse(char* input);
void helpParse(char* input);
int startsWith(char *input, char *commandName);
void TaskUartRead();
void TaskUartPayload();
void SWI_switch1();
void SWI_timer0();
void TaskInterruptPayload();
void TaskUartPayload();
void Timer0Callback(Timer_Handle myTimerHandle, int_fast16_t status);
void SWITimer0(UArg arg0, UArg arg1);
int32_t addPayload(char* payload);
void timerParse(char* input);
void timerBase();


void __attribute__((weak)) NDK_hookInit(int32_t id) {}


#endif
