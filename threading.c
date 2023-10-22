#include "header.h"

void TaskUartRead()
{
    uint32_t payloadGateKey;
    char inputChar;
    int newMessagePutIn = 0;
    Message uartMessage;
    uartMessage.length = 0;
    uartMessage.str[0] = 0;
    while (1)
    {
        UART_read(Glo.uart, &inputChar, 1);
        UART_write(Glo.uart, &inputChar, 1);

        if(uartMessage.length + 2 > MSGLEN)
        {
            char output[] = "\n\rMessage OVERFLOW\n\r";
            Glo.errorCounts[MSG_OVERFLOW]++;
            UART_write(Glo.uart, &output, strlen(output));
            uartMessage.length = 0;
            uartMessage.str[0] = 0;
        }
        else if(inputChar == '\n' || inputChar == '\r')
        {
            char output[MSGLEN];
            sprintf(output, "\n\r");
            UART_write(Glo.uart, &output, strlen(output));

            payloadGateKey = GateSwi_enter(Glo.bios.uartPayloadGate);

            int payloadIndex = Glo.uartPayloadQ.writingIndex;

            strcpy(Glo.uartPayloadQ.payloads[payloadIndex], uartMessage.str);
            uartMessage.str[0] = 0;
            uartMessage.length = 0;
            newMessagePutIn = 1;
            if(Glo.uartPayloadQ.writingIndex == NUMPAYLOADS - 1)
            {
                Glo.uartPayloadQ.writingIndex = 0;
            }
            else
            {
                Glo.uartPayloadQ.writingIndex++;
            }
            if (Glo.uartPayloadQ.writingIndex == Glo.uartPayloadQ.readingIndex)
            {
                sprintf(output, "Overload of messages to process\n\r");
                UART_write(Glo.uart, &output, strlen(output));
                //Glo.errorCounts[PAYLOAD_OVERFLOW]++;
            }
            GateSwi_leave(Glo.bios.uartPayloadGate, payloadGateKey);
        }
        else if(inputChar == '\b' || inputChar == 127)
        {
            if(uartMessage.length > 0)
            {
                uartMessage.str[uartMessage.length] = 0;
                uartMessage.length--;
            }
        }
        else
        {
            uartMessage.str[uartMessage.length] = inputChar;
            uartMessage.str[uartMessage.length + 1] = 0;
            uartMessage.length++;
        }

        if (newMessagePutIn == 1)
        {
            newMessagePutIn = 0;
            Semaphore_post(Glo.bios.uartPayloadSemaphore);
        }
    }
}


void TaskUartPayload()
{
    int nextPayload;

    int readingIndex;
    char *payload;
    uint32_t gateKey;

    while(1)
    {
        Semaphore_pend(Glo.bios.uartPayloadSemaphore, BIOS_WAIT_FOREVER);
        readingIndex = Glo.uartPayloadQ.readingIndex;
        payload = Glo.uartPayloadQ.payloads[readingIndex];
        parseMSG(payload);
        gateKey = GateSwi_enter(Glo.bios.uartPayloadGate);
        nextPayload = readingIndex + 1;
        if (nextPayload >= NUMPAYLOADS)
        {
            nextPayload = 0;
        }
        Glo.uartPayloadQ.readingIndex = nextPayload;
        GateSwi_leave(Glo.bios.uartPayloadGate, gateKey);
    }
}


void Timer0Callback(Timer_Handle myTimerHandle, int_fast16_t status)
{
    const int32_t index = 0;
    Glo.callbacks[index].HWI_flag = false;

    Swi_post(Glo.bios.timer0Swi);
}


void SWITimer0(UArg arg0, UArg arg1)
{
    const int32_t index = 0;
    uint32_t gateKey;

    gateKey = GateSwi_enter(Glo.bios.callbackGate);
    if (Glo.callbacks[index].count != 0)
    {
        if (Glo.callbacks[index].count > 0)
            Glo.callbacks[index].count--;
        if (Glo.callbacks[index].HWI_flag == false)
        {
            addPayload(Glo.callbacks[index].payload);
        }
    }
    GateSwi_leave(Glo.bios.callbackGate, gateKey);
}


int32_t addPayload(char* payload)
{
    int32_t nextPayload;
    int32_t index;
    uint32_t gateKey;
    char output[MSGLEN];

    if(!payload || payload[0] == 0)
        return -1;

    index = Glo.triggerPayloadQ.writingIndex;
    gateKey = GateSwi_enter(Glo.bios.triggerPayloadGate);

    nextPayload = index + 1;
    if(nextPayload >= NUMPAYLOADS)
        nextPayload = 0;

    if (nextPayload == Glo.triggerPayloadQ.readingIndex)
    {
        sprintf(output, "Overload of messages to process\n\r");
        UART_write(Glo.uart, &output, strlen(output));
        //Glo.errors[PAYLOAD_OVERFLOW]++;
    }
    else
    {
        strcpy(Glo.triggerPayloadQ.payloads[index], payload);
        Glo.triggerPayloadQ.writingIndex = nextPayload;
    }
    GateSwi_leave(Glo.bios.triggerPayloadGate, gateKey);
    Semaphore_post(Glo.bios.triggerPayloadSemaphore);
    return index;
}


void TaskInterruptPayload()
{
    int nextPayload;

    int readingIndex;
    char *payload;
    uint32_t gateKey;

    while(1)
    {
        Semaphore_pend(Glo.bios.triggerPayloadSemaphore, BIOS_WAIT_FOREVER);
        readingIndex = Glo.triggerPayloadQ.readingIndex;
        payload = Glo.triggerPayloadQ.payloads[readingIndex];
        parseMSG(payload);
        gateKey = GateSwi_enter(Glo.bios.triggerPayloadGate);
        nextPayload = readingIndex + 1;
        if (nextPayload >= NUMPAYLOADS)
        {
            nextPayload = 0;
        }
        Glo.triggerPayloadQ.readingIndex = nextPayload;
        GateSwi_leave(Glo.bios.triggerPayloadGate, gateKey);
    }
}

