#include "header.h"


int main(void)
{
    Glo.bios.uartReadTask = task0;
    Glo.bios.uartPayloadTask = task1;
    Glo.bios.interruptPayloadTask = task2;
    Glo.bios.uartPayloadGate = gateSwi0;
    Glo.bios.msgWriteGate = gateSwi1;
    Glo.bios.uartPayloadSemaphore = semaphore0;
    Glo.bios.triggerPayloadSemaphore = semaphore1;
    Glo.bios.timer0Swi = swi0;
    Glo.bios.callbackGate = gateSwi2;
    Glo.bios.triggerPayloadGate = gateSwi3;


    int index;
    for (index = 0; index < NUMERRORS; index++)
    {
        Glo.errorCounts[index] = 0;
    }

    Glo.uartPayloadQ.readingIndex = 0;
    Glo.uartPayloadQ.writingIndex = 0;
    Glo.triggerPayloadQ.readingIndex = 0;
    Glo.triggerPayloadQ.writingIndex = 0;

    for(index = 0; index < NUMPAYLOADS; index++)
    {
        Glo.uartPayloadQ.payloads[index][0] = 0;
        Glo.triggerPayloadQ.payloads[index][0] = 0;
    }
    for (index = 0; index < NUMCALLBACKS; index++)
    {
        Glo.callbacks[index].index = index;
        Glo.callbacks[index].count = 0;
        Glo.callbacks[index].HWI_flag = false;
        Glo.callbacks[index].payload[0] = 0;
    }
    Glo.timer0Period = 1000000;


    GPIO_init();
    UART_init();
    Timer_init();


    GPIO_write(0, 1); // CONFIG_GPIO_0, CONFIG_GPIO_LED_ON
    GPIO_write(1, 1);
    GPIO_write(2, 1);
    GPIO_write(3, 1);


    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.baudRate = 115200;

    Glo.uart = UART_open(0, &uartParams); // CONFIG_UART_0, &uartParams

    if (Glo.uart == NULL)
    {
        while (1); // UART_open() failed
    }

    Timer_Handle timer0;
    Timer_Params TimerParams;

    Timer_Params_init(&TimerParams);
    TimerParams.period = Glo.timer0Period;
    TimerParams.periodUnits = Timer_PERIOD_US;
    TimerParams.timerCallback = Timer0Callback;
    TimerParams.timerMode = Timer_CONTINUOUS_CALLBACK;

    timer0 = Timer_open(0, &TimerParams); //Timer_open(CONFIG_TIMER_0, &TimerParams)

    if (timer0 == NULL)
    {
        while(1); // Timer_open() failed
    }

    if(Timer_start(timer0) == Timer_STATUS_ERROR)
    {
        while(1); // Timer_start() failed
    }

    Glo.timer0 = timer0;

    BIOS_start();

    return (0);
}
