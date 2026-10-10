#include <LPC21xx.h>
#include <string.h>

/* LCD: RS=P0.12, E=P0.13, D4-D7=P0.14-P0.17 */
#define LCD_RS (1 << 12)
#define LCD_EN (1 << 13)
#define LCD_DATA (0x0FU << 14)
#define LCD_PINS (LCD_RS | LCD_EN | LCD_DATA)

/* LED: P0.20  */
#define LED (1 << 20)

/* Keypad: Rows P1.16-P1.19, columns P1.20-P1.23 */
#define ROW_MASK (0x0F << 16)
#define COL_MASK (0x0F << 20)


unsigned int tableNo = 0;
char foodKey = 0;
unsigned int state = 0;

/* Keypad layout */
const char keypad[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

/* ---------------- DELAY FUNCTION ---------------- */

void DelayMs(unsigned int ms)
{
T0PR=15000-1;
T0TCR=0x1;
while(T0TC<ms);
T0TCR=0x3;
T0TCR=0x0;
}


/* ---------------- UART0 FUNCTIONS ---------------- */

void UART0_Init(void)
{
    /* Select UART0 functions on P0.0 and P0.1 */
    PINSEL0 &= ~0x0000000F;
    PINSEL0 |= 0x00000005;

    /* Enable divisor latch */
    U0LCR = 0x83;

    /* PCLK = 15 MHz, baud = 9600 */
    U0DLL = 97;
    U0DLM = 0;

    /* 8 data bits, 1 stop bit, no parity */
    U0LCR = 0x03;

    /* Enable and reset UART FIFOs */
    U0FCR = 0x07;
}

void UART0_SendChar(char ch)
{
    while (!(U0LSR & 0x20U))
    {
        /* Wait until transmit holding register is empty */
    }

    U0THR = ch;
}

void UART0_SendString(const char *str)
{
    while (*str)
    {
        UART0_SendChar(*str);
        str++;
    }
}

void SendNewLine(void)
{
    UART0_SendChar('\r');
    UART0_SendChar('\n');
}

/* Non-blocking line reader.
   Returns 1 when a complete non-empty line is received. */
int Bluetooth_ReadLine(char *buffer, unsigned int size)
{
    static unsigned int index = 0;
    char ch;

    while (U0LSR & 0x01U)
    {
        ch = U0RBR;

        if (ch == '\r' || ch == '\n')
        {
            if (index > 0)
            {
                buffer[index] = '\0';
                index = 0;
                return 1;
            }
        }
        else
        {
            if (index < size - 1)
            {
                buffer[index++] = ch;
            }
            else
            {
                /* Reset if the received command is too long */
                index = 0;
            }
        }
    }

    return 0;
}

/* ---------------- LCD FUNCTIONS ---------------- */

void LCD_Pulse(void)
{
    IOSET0 = LCD_EN;
    DelayMs(1);

    IOCLR0 = LCD_EN;
    DelayMs(1);
}

void LCD_Nibble(unsigned char data)
{
    /* Clear LCD data pins */
    IOCLR0 = LCD_DATA;

    /* Put the four-bit value on P0.14-P0.17 */
    IOSET0 = ((unsigned long)(data & 0x0F) << 14);

    LCD_Pulse();
}

void LCD_Command(unsigned char cmd)
{
    IOCLR0 = LCD_RS;

    LCD_Nibble(cmd >> 4);
    LCD_Nibble(cmd & 0x0F);

    if (cmd == 0x01 || cmd == 0x02)
        DelayMs(2);
    else
        DelayMs(1);
}

void LCD_Data(unsigned char data)
{
    IOSET0 = LCD_RS;

    LCD_Nibble(data >> 4);
    LCD_Nibble(data & 0x0F);

    DelayMs(1);
}

void LCD_String(const char *str)
{
    while (*str)
    {
        LCD_Data(*str);
        str++;
    }
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
    DelayMs(2);
}

void LCD_Cursor(unsigned char row, unsigned char col)
{
    if (row == 0)
        LCD_Command(0x80 + col);
    else
        LCD_Command(0xC0 + col);
}

void LCD_Init(void)
{
    /* Configure P0.12-P0.17 as GPIO */
    PINSEL0 &= ~((3U << 24) | (3U << 26) |
                 (3U << 28) | (3U << 30));

    PINSEL1 &= ~0x0000000F;

    IODIR0 |= LCD_PINS;

    IOCLR0 = LCD_PINS;

    DelayMs(20);

    /* Initialise LCD in 4-bit mode */
    IOCLR0 = LCD_RS;

    LCD_Nibble(0x03);
    DelayMs(5);

    LCD_Nibble(0x03);
    DelayMs(1);

    LCD_Nibble(0x03);
    DelayMs(1);

    LCD_Nibble(0x02);

    LCD_Command(0x28); /* 4-bit, 2 lines, 5x8 font */
    LCD_Command(0x0C); /* Display ON, cursor OFF */
    LCD_Command(0x06); /* Cursor moves right */
    LCD_Clear();
}

/* ---------------- TITLE SCROLLING ---------------- */

void ScrollTitle(void)
{
    const char title[] =
        " BLUETOOTH BASED WIRELESS RESTAURANT ORDERING SYSTEM ";

    unsigned int i, j;
    unsigned int length = strlen(title);

    for (i = 0; i <= length - 16; i++)
    {
        LCD_Command(0x01);
		DelayMs(5);
		LCD_Command(0x80);
        for (j = 0; j < 16; j++)
        {
            LCD_Data(title[i + j]);
        }

        DelayMs(500);
    }
	LCD_Clear();
}

/* ---------------- KEYPAD FUNCTIONS ---------------- */

void Keypad_Init(void)
{
    /* Rows are outputs; columns are inputs */
    IODIR1 |= ROW_MASK;
    IODIR1 &= ~COL_MASK;

    /* Set rows HIGH */
    IOSET1 = ROW_MASK;

}

char Keypad_GetKey(void)
{
    unsigned int row, col;
    unsigned long columnValue;

    for (row = 0; row < 4; row++)
    {
        /* Set all rows HIGH */
        IOSET1 = ROW_MASK;

        /* Pull the selected row LOW */
        IOCLR1 = (1 << (16 + row));

        DelayMs(1);

        columnValue = IO1PIN & COL_MASK;

        for (col = 0; col < 4; col++)
        {
            /* A pressed key pulls the column LOW */
            if (!(columnValue & (1 << (20 + col))))
            {
                DelayMs(20); /* Debounce */

                columnValue = IOPIN1 & COL_MASK;

                if (!(columnValue & (1 << (20 + col))))
                {
                    /* Wait for key release */
                    while ((IOPIN1 & (1 << (20 + col))) == 0)
                    {
                        DelayMs(1);
                    }

                    IOSET1 = ROW_MASK;
                    return keypad[row][col];
                }
            }
        }
    }

    IOSET1 = ROW_MASK;
    return 0;
}

/* ---------------- LED FUNCTIONS ---------------- */

void LED_Init(void)
{
    /* Configure P0.20 as GPIO */
    PINSEL1 &= ~(3U << 8);

    IODIR0 |= LED;
    IOCLR0 = LED;
}

void LED_On(void)
{
   IOCLR0 = LED; 
}

void LED_Off(void)
{
IOSET0 = LED;
    
}

/*FOOD FUNCTIONS */

const char *FoodName(char key)
{
    switch (key)
    {
        case 'A': return "BIRIYANI";
        case 'B': return "CURD RICE";
        case 'C': return "DOSA";
        case 'D': return "IDLY";
        default: return "UNKNOWN";
    }
}

void DisplayFoodOptions(void)
{
    LCD_Clear();

    LCD_Cursor(0, 0);
    LCD_String("A BIRI B CURD");

    LCD_Cursor(1, 0);
    LCD_String("C DOSA D IDLY");
}

/* ---------------- ORDER FUNCTIONS ---------------- */

void StartNewOrder(void)
{
    tableNo = 0;
    foodKey = 0;
    state = 0;

    LCD_Clear();
    LCD_Cursor(0, 0);
    LCD_String("SELECT TABLE");

    LCD_Cursor(1, 0);
    LCD_String("PRESS KEY 1-4");
}

void DisplaySelectedOrder(void)
{
    LCD_Clear();

    LCD_Cursor(0, 0);
    LCD_String("T");
    LCD_Data('0' + tableNo);
    LCD_String(" ");
    LCD_String(FoodName(foodKey));

    LCD_Cursor(1, 0);
    LCD_String("# OK * CANCEL");
}

void SendOrderToMobile(void)
{
    UART0_SendString("ORDER RECEIVED: TABLE ");
    UART0_SendChar('0' + tableNo);
    UART0_SendString(" - ");
    UART0_SendString(FoodName(foodKey));
    SendNewLine();
}

void DisplayBluetoothStatusMessage(void)
{
    LCD_Clear();

    LCD_Cursor(0, 0);
    LCD_String("SEND STATUS VIA");

    LCD_Cursor(1, 0);
    LCD_String("BLUETOOTH");
}

/* ---------------- MAIN PROGRAM ---------------- */

int main(void)
{
    char key;
    char command[32];
    unsigned int i;

    /* Initialise hardware */
    LCD_Init();
    UART0_Init();
    Keypad_Init();
    LED_Init();

    LED_Off();

    /* 1. Scroll project title */
    ScrollTitle();

    /* 2. Display waiting message for 2 seconds */
    LCD_Clear();

    LCD_Cursor(0, 0);
    LCD_String("WAITING FOR");

    LCD_Cursor(1, 0);
    LCD_String("NEW ORDER");

    DelayMs(2000);


    UART0_SendString("SYSTEM READY");
    SendNewLine();

    /* 3. Begin table selection */
    StartNewOrder();

    while (1)
    {
        /* STATE 0: SELECT TABLE */

        if (state == 0)
        {
            key = Keypad_GetKey();

            if (key >= '1' && key <= '4')
            {
                tableNo = key - '0';

                LCD_Clear();
                LCD_Cursor(0, 0);
                LCD_String("TABLE SELECTED");

                LCD_Cursor(1, 0);
                LCD_String("TABLE ");
                LCD_Data(key);

                DelayMs(1000);

                DisplayFoodOptions();
                state = 1;
            }
        }

        /* STATE 1: SELECT FOOD  */

        else if (state == 1)
        {
            key = Keypad_GetKey();

            if (key >= 'a' && key <= 'd')
                key = key - ('a' - 'A');

            if (key == 'A' || key == 'B' ||
                key == 'C' || key == 'D')
            {
                foodKey = key;

                DisplaySelectedOrder();
                state = 2;
            }
            else if (key == '*')
            {
                StartNewOrder();
            }
        }

        /* STATE 2: CONFIRM ORDER  */

        else if (state == 2)
        {
            key = Keypad_GetKey();

            if (key == '#')
            {
                /* Send confirmed order to the mobile */
                SendOrderToMobile();

                LCD_Clear();
                LCD_Cursor(0, 0);
                LCD_String("ORDER RECEIVED");

                LCD_Cursor(1, 0);
                LCD_String("TABLE ");
                LCD_Data('0' + tableNo);

                DelayMs(1500);

                DisplayBluetoothStatusMessage();
                state = 3;
            }
            else if (key == '*')
            {
                /* Cancel current order */
                StartNewOrder();
            }
        }

        /* STATE 3: WAIT FOR BLUETOOTH STATUS  */

        else if (state == 3)
        {
            if (Bluetooth_ReadLine(command, sizeof(command)))
            {
                /* Convert command to uppercase */
                for (i = 0; command[i] != '\0'; i++)
                {
                    if (command[i] >= 'a' && command[i] <= 'z')
                    {
                        command[i] =
                            command[i] - ('a' - 'A');
                    }
                }

                /* Phone sends READY */
                if (strcmp(command, "READY") == 0)
                {
                    LED_On();

                    LCD_Clear();
                    LCD_Cursor(0, 0);
                    LCD_String("ORDER READY");

                    LCD_Cursor(1, 0);
                    LCD_String("TABLE ");
                    LCD_Data('0' + tableNo);

                    UART0_SendString("STATUS: READY");
                    SendNewLine();
                }

                /* Phone sends DISPATCHED */
                else if (strcmp(command, "DISPATCHED") == 0)
                {
                    LED_Off();

                    LCD_Clear();
                    LCD_Cursor(0, 0);
                    LCD_String("ORDER DISPATCHED");

                    LCD_Cursor(1, 0);
                    LCD_String("TABLE ");
                    LCD_Data('0' + tableNo);

                    UART0_SendString("STATUS: DISPATCHED");
                    SendNewLine();

                    DelayMs(2000);

                    /* Start accepting the next order */
                    StartNewOrder();
                }

                /* Handle any unsupported status command */
                else
                {
                    UART0_SendString("INVALID STATUS");
                    SendNewLine();
                }
            }
        }
    }
}


