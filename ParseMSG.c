#include "header.h"

void parseMSG(char *messageLine)
{
    if(strlen(messageLine) == 0)
    {
        // Do nothing if line is empty
    }
    else if (startsWith(messageLine, "-about"))
        aboutParse();
    else if (startsWith(messageLine, "-error"))
        errorParse(messageLine);
    else if (startsWith(messageLine, "-gpio"))
        gpioParse(messageLine);
    else if (startsWith(messageLine, "-help"))
        helpParse(messageLine);
    else if (startsWith(messageLine, "-memr"))
        memoryReadParse(messageLine);
    else if (startsWith(messageLine, "-print"))
        printParse(messageLine);
    else if (startsWith(messageLine, "-timer"))
        timerParse(messageLine);
    else // Invalid operation
    {
        char output[MSGLEN];
        sprintf(output, "Operation \"%s\" not valid.\n\r\n\r", messageLine);
        UART_write(Glo.uart, &output, strlen(output));
        Glo.errorCounts[INVALID_OPERATION]++;
    }
}

void aboutParse(UART_Handle uart)
{
    char output[MSGLEN];
    sprintf(output, "Author: %s\n\r", AUTHOR);
    UART_write(Glo.uart, &output, strlen(output));
    sprintf(output, "Version: %s\n\r", VERSION);
    UART_write(Glo.uart, &output, strlen(output));
    sprintf(output, "Assignment: %d\n\r", ASSIGNMENT);
    UART_write(Glo.uart, &output, strlen(output));
    sprintf(output, "Compile Timestamp: %s %s\n\r", __TIME__, __DATE__);
    UART_write(Glo.uart, &output, strlen(output));
}


void errorParse(char* input)
{

    char noSpaces[MSGLEN];

    removeSpaces(noSpaces, input);
    int lenAfterCommand = strlen(noSpaces) - strlen("-error");
    char afterCommand[MSGLEN];
    strncpy(afterCommand, noSpaces + strlen("-error"), lenAfterCommand);

    if (strlen("-error") == strlen(noSpaces))
    {

        errorBase();
    }
    else if (startsWith(afterCommand, "c"))
    {
        int error;
        for (error = 0; error < NUMERRORS; error++)
        {
            Glo.errorCounts[error] = 0;
        }
        errorBase();
    }
    else
    {
        char output[MSGLEN];
        char *endPtr;
        int error = strtol(afterCommand, &endPtr, 10);
        if (endPtr == afterCommand) // No number (nor c) is given after -error
        {
            char output [MSGLEN];
            sprintf(output, "A number or \"c\" was expected after -error\n\r");
            UART_write(Glo.uart, &output, strlen(output));
            Glo.errorCounts[WRONG_TYPE]++;
        }
        else if (error == MSG_OVERFLOW)
        {
            sprintf(output, "%d. Message Overflow: %d\n\r", MSG_OVERFLOW, Glo.errorCounts[MSG_OVERFLOW]);
            UART_write(Glo.uart, &output, strlen(output));
        }
        else if (error == INVALID_OPERATION)
        {
            sprintf(output, "%d. Invalid Operation: %d\n\r", INVALID_OPERATION, Glo.errorCounts[INVALID_OPERATION]);
            UART_write(Glo.uart, &output, strlen(output));
        }
        else if (error == WRONG_TYPE)
        {
            sprintf(output, "%d. Wrong Type: %d\n\r", WRONG_TYPE, Glo.errorCounts[WRONG_TYPE]);
            UART_write(Glo.uart, &output, strlen(output));
        }
        else if (error == BAD_NUMBER)
        {
            sprintf(output, "%d. Bad Number: %d\n\r", BAD_NUMBER, Glo.errorCounts[BAD_NUMBER]);
            UART_write(Glo.uart, &output, strlen(output));
        }
        else
        {
            sprintf(output, "Only error numbers 0 - %d are supported.\n\r", NUMERRORS);
            UART_write(Glo.uart, &output, strlen(output));
            Glo.errorCounts[BAD_NUMBER]++;
        }
    }
}

void errorBase(UART_Handle uart)
{
    char output[MSGLEN];
    sprintf(output, "%d. Message Overflow: %d\n\r", MSG_OVERFLOW, Glo.errorCounts[MSG_OVERFLOW]);
    UART_write(Glo.uart, &output, strlen(output));

    sprintf(output, "%d. Invalid Operation: %d\n\r", INVALID_OPERATION, Glo.errorCounts[INVALID_OPERATION]);
    UART_write(Glo.uart, &output, strlen(output));

    sprintf(output, "%d. Wrong Type: %d\n\r", WRONG_TYPE, Glo.errorCounts[WRONG_TYPE]);
    UART_write(Glo.uart, &output, strlen(output));

    sprintf(output, "%d. Bad Number: %d\n\r", BAD_NUMBER, Glo.errorCounts[BAD_NUMBER]);
    UART_write(Glo.uart, &output, strlen(output));
}


////////////////////////////////////////
// GPIO Parsing


void gpioParse(char* input)
{
    char noSpaces[MSGLEN];

    removeSpaces(noSpaces, input);
    int lenAfterCommand = strlen(noSpaces) - strlen("-gpio");
    char afterCommand[MSGLEN];
    strncpy(afterCommand, noSpaces + strlen("-gpio"), lenAfterCommand);
    if (strlen("-gpio") == strlen(noSpaces) || startsWith(afterCommand, "r"))
    {
        gpioBase();
        return;
    }

    char* remainderPtr;
    char remainder[MSGLEN];
    int gpio = strtol(afterCommand, &remainderPtr, 10);
    if (gpio < 0 || gpio >= NUMGPIOS)
    {
        char output[MSGLEN];
        sprintf(output, "Only GPIOs 0 - %d are available.\n\r", NUMGPIOS - 1);
        UART_write(Glo.uart, &output, strlen(output));
        Glo.errorCounts[BAD_NUMBER]++;
        return;
    }
    strncpy(remainder, afterCommand + 1, lenAfterCommand - 1); // assumes gpio int is 1 digit

    if (*remainderPtr == 0)
    {
        readGPIOprint(gpio);
    }
    else if(startsWith(remainder, "r"))
    {
        readGPIOprint(gpio);
    }
    else if(startsWith(remainder, "w"))
    {
        writeGPIOprint(gpio, remainder);
    }
    else if (startsWith(remainder, "t"))
    {
        GPIO_toggle(gpio);
        readGPIOprint(gpio);
    }
    else
    {
        char output[MSGLEN];
        sprintf(output, "Operation \"%s\" not valid.\n\r", input);
        UART_write(Glo.uart, &output, strlen(output));
        Glo.errorCounts[INVALID_OPERATION]++;
    }
    return;
}


void gpioBase()
{
    int gpio = 0;
    for (gpio = 0; gpio < NUMGPIOS; gpio++)
    {
       readGPIOprint(gpio);
    }
}

void readGPIOprint(int gpio)
{
    char output[MSGLEN];
    int gpioState = GPIO_read(gpio);
    sprintf(output, "\tgpio %d: %d\n\r", gpio, gpioState);
    UART_write(Glo.uart, &output, strlen(output));
}


void writeGPIOprint(int gpioNum, char* remainder)
{
    char *endPtr;
    char gpioString[MSGLEN];
    strncpy(gpioString, remainder + 1, 1);
    gpioString[1] = 0;
    int gpioState = strtol(gpioString, &endPtr, 10);
    if(gpioString == endPtr) // no number is given after w
    {
        char output [MSGLEN];
        sprintf(output, "State was expected after -gpio w\n\r");
        UART_write(Glo.uart, &output, strlen(output));
        Glo.errorCounts[WRONG_TYPE]++;
    }
    else if (gpioState != 0 && gpioState != 1)
    {
        char output [MSGLEN];
        sprintf(output, "GPIO state can only be one or zero.\n\r");
        UART_write(Glo.uart, &output, strlen(output));
        Glo.errorCounts[BAD_NUMBER]++;
    }
    else
    {
        GPIO_write(gpioNum, gpioState);
        readGPIOprint(gpioNum);
    }
}


// End GPIO Parsing
////////////////////////////////////////


void helpParse(char* input)
{
    char afterCommand[MSGLEN];
    char output[MSGLEN];
    char noSpaces[MSGLEN];

    removeSpaces(noSpaces, input);

    if (strlen("-help") == strlen(noSpaces))
    {
        helpBase();
        return;
    }
    int lenAfterCommand = strlen(noSpaces) - strlen("-help");
    strncpy(afterCommand, noSpaces + strlen("-help"), lenAfterCommand);
    afterCommand[lenAfterCommand] = 0;

    if      (startsWith(afterCommand, "-about") || startsWith(afterCommand, "about"))
    {
        sprintf(output, "\tDisplays compile time, compile date, author, version, and homework number.\n\r");
        UART_write(Glo.uart, &output, strlen(output));
    }
    else if (startsWith(afterCommand, "-gpio") || startsWith(afterCommand, "gpio"))
    {
        sprintf(output, "\tAllows to read, write, and toggle gpios.\n\r");
        UART_write(Glo.uart, &output, strlen(output));
        sprintf(output, "\tgpio 0 to 3: LEDs.\n\r");
        UART_write(Glo.uart, &output, strlen(output));
        sprintf(output, "\tgpio 4: PK5\n\r");
        UART_write(Glo.uart, &output, strlen(output));
        sprintf(output, "\tgpio 5: PD4\n\r");
        UART_write(Glo.uart, &output, strlen(output));
        sprintf(output, "\tgpio 6-7 switches on the sides of the board\n\r");
        UART_write(Glo.uart, &output, strlen(output));
    }
    else if (startsWith(afterCommand, "-error") || startsWith(afterCommand, "error"))
    {
        sprintf(output, "\tPrints out counts of each kind of error.\n\r");
        UART_write(Glo.uart, &output, strlen(output));
    }
    else if (startsWith(afterCommand, "-help") || startsWith(afterCommand, "help"))
    {
        sprintf(output, "\tLists commands and displays informations about commands.\n\r");
        UART_write(Glo.uart, &output, strlen(output));
    }
    else if (startsWith(afterCommand, "-memr") || startsWith(afterCommand, "memr"))
    {
            sprintf(output, "\tDisplays memory contents of four locations by the address.\n\r");
            UART_write(Glo.uart, &output, strlen(output));
            sprintf(output, "\t0x00000000 to 0x000FFFFF: Flash\n\r");
            UART_write(Glo.uart, &output, strlen(output));
            sprintf(output, "\t0x20000000 to 0x2003FFFF: SRAM\n\r");
            UART_write(Glo.uart, &output, strlen(output));
            sprintf(output, "\t0x40000000 to 0x44054FFF: Peripherals (use caution)\n\r");
            UART_write(Glo.uart, &output, strlen(output));

    }
    else if (startsWith(afterCommand, "-print") || startsWith(afterCommand, "print"))
    {
        sprintf(output, "\tPrints out string after the command.\n\r");
        UART_write(Glo.uart, &output, strlen(output));
    }
    else if (startsWith(afterCommand, "-timer") || startsWith(afterCommand, "timer"))
    {
        sprintf(output, "\t-timer\n\r\t\tPrints out the period in us.\n\r");
        UART_write(Glo.uart, &output, strlen(output));
        sprintf(output, "\t-timer [period]\n\r\t\tSets the period to value in us\n\r");
        UART_write(Glo.uart, &output, strlen(output));
    }
    else
    {
        sprintf(output, "Operation \"%s\" not valid.\n\r", afterCommand);
        UART_write(Glo.uart, &output, strlen(output));
    }
    sprintf(output, "\n\r");
    UART_write(Glo.uart, &output, strlen(output));
}


void helpBase()
{
    char output[MSGLEN];
    sprintf(output, "\t%s\n\r", "-about");
    UART_write(Glo.uart, &output, strlen(output));
    sprintf(output, "\t%s\n\r", "-error");
    UART_write(Glo.uart, &output, strlen(output));
    sprintf(output, "\t%s\n\r", "-gpio [gpioNum] [action]");
    UART_write(Glo.uart, &output, strlen(output));
    sprintf(output, "\t%s\n\r", "-help [command]");
    UART_write(Glo.uart, &output, strlen(output));
    sprintf(output, "\t%s\n\r", "-memr [address]");
    UART_write(Glo.uart, &output, strlen(output));
    sprintf(output, "\t%s\n\r", "-print [string]");
    UART_write(Glo.uart, &output, strlen(output));
    sprintf(output, "\t%s\n\r", "-timer [period]");
    UART_write(Glo.uart, &output, strlen(output));
    sprintf(output, "\n\r");
    UART_write(Glo.uart, &output, strlen(output));
    return;
}


void memoryReadParse(char* input)
{
    long int address;
    char noSpaces[MSGLEN];
    char output[MSGLEN];

    removeSpaces(noSpaces, input);

    input = noSpaces;

    if (strlen("-memr") == strlen(input))
    {
        address = 0;
    }
    else
    {
       input += strlen("-memr");
       char* endPtr;
       address = strtol(input, &endPtr, 16);
       address = address / 4;
       address = address * 4;


       if ((address > 0x000FFFFF && address < 0x20000000)
                || (address > 0x2003FFFF && address < 0x40000000))
        {
            sprintf(output, "Address 0x%010X is out of range.\n\r", address);
            UART_write(Glo.uart, &output, strlen(output));
            Glo.errorCounts[BAD_NUMBER]++;
            return;
        }
        else if (address > 0x44054FFF)
        {
            sprintf(output, "Address is out of range.\n\r");
            UART_write(Glo.uart, &output, strlen(output));
            Glo.errorCounts[BAD_NUMBER]++;
            return;
        }
        if (*endPtr != 0)
        {
            sprintf(output, "Address must only contain hexadecimal digits.\n\r");
            UART_write(Glo.uart, &output, strlen(output));
            Glo.errorCounts[WRONG_TYPE]++;
            return;
        }
    }

    int value;
    sprintf(output, "0x%010X 0x%010X 0x%010X 0x%010X\n\r", address + 0xC, address + 0x8, address + 0x4, address);
    UART_write(Glo.uart, &output, strlen(output));

    value = *(int *) (address + 0xC);
    sprintf(output, "0x%010X ", value);
    UART_write(Glo.uart, &output, strlen(output));

    value = *(int *) (address + 0x8);
    sprintf(output, "0x%010X ", value);
    UART_write(Glo.uart, &output, strlen(output));

    value = *(int *) (address + 0x4);
    sprintf(output, "0x%010X ", value);
    UART_write(Glo.uart, &output, strlen(output));

    value = *(int *) (address + 0x0);
    sprintf(output, "0x%010X\n\r", value);
    UART_write(Glo.uart, &output, strlen(output));
    return;

}


void printParse(char* input)
{
    char output[MSGLEN];

    if (strlen("-print") == strlen(input))
    {
        sprintf(output, "\n\r");
        UART_write(Glo.uart, &output, strlen(output));
        return;
    }
    input += strlen("-print");
    if (input[0] == ' ')
    {
        if (strlen(input) == 1)
        {
            sprintf(output, "\n\r");
            UART_write(Glo.uart, &output, strlen(output));
            return;
        }
        input++;
    }
   sprintf(output, "%s\n\r", input);
   UART_write(Glo.uart, &output, strlen(output));
   return;
}


void timerParse(char* input)
{
    char output[MSGLEN];
    char noSpaces[MSGLEN];
    removeSpaces(noSpaces, input);
    input = noSpaces;
    if (strlen(input) == strlen("-timer"))
    {
        timerBase();
    }
    else
    {
        char *endPtr;
        input += strlen("-timer");
        long period = strtol(input, &endPtr, 10);
        if (*endPtr != 0)
        {
            sprintf(output,"Period must be an integer.\n\r");
            UART_write(Glo.uart, &output, strlen(output));
            Glo.errorCounts[WRONG_TYPE]++;
            return;
        }
        else if (period < 0)
        {
            sprintf(output, "Period must be positive.\n\r");
            UART_write(Glo.uart, &output, strlen(output));
            Glo.errorCounts[BAD_NUMBER]++;
            return;
        }
        else if (period > 2000000000)
        {
            sprintf(output, "Period must be less than two billion.\n\r");
            UART_write(Glo.uart, &output, strlen(output));
            Glo.errorCounts[BAD_NUMBER]++;
            return;
        }
        else
        {
            Glo.timer0Period = period;
            timerBase();
        }
    }

}


void timerBase()
{
    char output[MSGLEN];
    sprintf(output, "Timer period: %d us\n\r", Glo.timer0Period);
    UART_write(Glo.uart, &output, strlen(output));
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// String Manipulation Functions


int startsWith(char *input, char *commandName)
{
    if(strlen(input) >= strlen(commandName))
    {
        char substring[MSGLEN];
        strncpy(substring, input, strlen(commandName));
        substring[strlen(commandName)] = 0;

        return (!strcmp(substring, commandName));
    }
    return 0;
}

void removeSpaces(char* noSpaces, const char* input)
{
    int inputIndex;
    int noSpacesIndex = 0;
    for (inputIndex = 0; inputIndex < strlen(input); inputIndex++)
    {
        char character = input[inputIndex];
        if (character != ' ')
        {
            noSpaces[noSpacesIndex] = character;
            noSpacesIndex++;
        }
    }
    noSpaces[noSpacesIndex] = 0;
}


// End String Manipulation Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
