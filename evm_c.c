#include <reg52.h>
#include <stdio.h>

//==================================================
// HARDWARE PIN DEFINITIONS (Based on your final wiring)
//==================================================

// LCD Connections (8-Bit Mode)
#define LCD_DATA_PORT P2      // Data Port for 8-bit mode
sbit LCD_RS = P3^2;           // Register Select pin
sbit LCD_RW = P3^3;           // Read/Write pin
sbit LCD_E  = P3^4;           // Enable pin

// Keypad Connections
sbit KEY_R1 = P1^0;
sbit KEY_R2 = P1^1;
sbit KEY_R3 = P1^2;
sbit KEY_R4 = P1^3;
sbit KEY_C1 = P1^4;
sbit KEY_C2 = P1^5;
sbit KEY_C3 = P1^6;
sbit KEY_C4 = P1^7;

// Buzzer Connection (NEW)
sbit BUZZER = P3^5;           // Buzzer connected to P3.5

//==================================================
// FUNCTION PROTOTYPES
//==================================================
void delay_ms(unsigned int ms);
void lcd_send_command(unsigned char cmd);
void lcd_send_data(unsigned char data_val);
void lcd_init(void);
void lcd_string(char *str);
void lcd_goto_xy(unsigned char row, unsigned char col);
void lcd_int(unsigned int num);
char get_key(void);
void show_results(void);
void beep(void); // NEW: Prototype for the buzzer function

//==================================================
// GLOBAL VARIABLES
//==================================================
int vote_count[4] = {0, 0, 0, 0}; // P1, P2, P3, P4

//==================================================
// MAIN FUNCTION
//==================================================
void main() {
    char key;

    lcd_init();
    BUZZER = 0; // Ensure buzzer is off at startup

    // Startup Message
    lcd_goto_xy(0, 2);
    lcd_string("Electronic");
    lcd_goto_xy(1, 1);
    lcd_string("Voting Machine");
    delay_ms(2000);

    while(1) {
        // Main Voting Loop
        lcd_send_command(0x01); // Clear screen
        lcd_goto_xy(0, 0);
        lcd_string("P1(1) P2(2)");
        lcd_goto_xy(1, 0);
        lcd_string("P3(3) P4(4) #=Res");

        key = get_key(); // Wait for a key press

        switch(key) {
            case '1': vote_count[0]++; break;
            case '2': vote_count[1]++; break;
            case '3': vote_count[2]++; break;
            case '4': vote_count[3]++; break;
            case '#':
                show_results();
                while(1); // Halt after showing results
            default:
                continue; // Ignore other keys
        }

        // Acknowledge the vote
        beep(); // NEW: Beep to provide audio feedback
        lcd_send_command(0x01);
        lcd_goto_xy(0, 3);
        lcd_string("Thank You!");
        lcd_goto_xy(1, 1);
        lcd_string("Please Wait...");
        delay_ms(1500);
    }
}

//==================================================
// APPLICATION-SPECIFIC FUNCTIONS
//==================================================

// UPDATED: This function now generates a square wave to drive a
// passive buzzer. It rapidly toggles the pin ON and OFF.
void beep(void) {
    unsigned int i;
    // Loop for about 150ms to control the duration of the beep
    for(i = 0; i < 150; i++) {
        BUZZER = 1;     // Set pin HIGH
        delay_ms(1);    // Wait 1ms
        BUZZER = 0;     // Set pin LOW
        delay_ms(1);    // Wait 1ms
    }
}


void show_results() {
    int i, max_votes = -1, winner = 0, tie = 0;

    lcd_send_command(0x01);
    lcd_goto_xy(0, 0);
    lcd_string("P1  P2  P3  P4");
    lcd_goto_xy(1, 0);

    lcd_int(vote_count[0]);
    lcd_goto_xy(1, 4);
    lcd_int(vote_count[1]);
    lcd_goto_xy(1, 8);
    lcd_int(vote_count[2]);
    lcd_goto_xy(1, 12);
    lcd_int(vote_count[3]);
    delay_ms(4000);

    for (i = 0; i < 4; i++) {
        if (vote_count[i] > max_votes) {
            max_votes = vote_count[i];
            winner = i + 1;
            tie = 0;
        } else if (vote_count[i] == max_votes && max_votes > 0) {
            tie = 1;
        }
    }

    lcd_send_command(0x01);
    lcd_goto_xy(0, 2);
    if (tie == 1) {
        lcd_string("It's a TIE!");
    } else {
        lcd_string("Winner is: P");
        lcd_send_data(winner + '0');
    }
}

char get_key(void) {
    // FIX: The key map has been re-arranged to match your specific
    // keypad wiring. This will correct the issue where pressing a key
    // gave the wrong input.
    const unsigned char key_map[4][4] = {
        {'1', '2', '3', 'A'},
        {'#', '5', '6', '3'},
        {'7', '8', '9', '2'},
        {'*', '0', '4', '1'}
    };
    delay_ms(100); // Debounce delay

    while(1) {
        // Scan Row 1
        KEY_R1=0; KEY_R2=1; KEY_R3=1; KEY_R4=1;
        if(KEY_C1==0) { while(KEY_C1==0); return key_map[0][0]; }
        if(KEY_C2==0) { while(KEY_C2==0); return key_map[0][1]; }
        if(KEY_C3==0) { while(KEY_C3==0); return key_map[0][2]; }
        if(KEY_C4==0) { while(KEY_C4==0); return key_map[0][3]; }

        // Scan Row 2
        KEY_R1=1; KEY_R2=0; KEY_R3=1; KEY_R4=1;
        if(KEY_C1==0) { while(KEY_C1==0); return key_map[1][0]; }
        if(KEY_C2==0) { while(KEY_C2==0); return key_map[1][1]; }
        if(KEY_C3==0) { while(KEY_C3==0); return key_map[1][2]; }
        if(KEY_C4==0) { while(KEY_C4==0); return key_map[1][3]; }

        // Scan Row 3
        KEY_R1=1; KEY_R2=1; KEY_R3=0; KEY_R4=1;
        if(KEY_C1==0) { while(KEY_C1==0); return key_map[2][0]; }
        if(KEY_C2==0) { while(KEY_C2==0); return key_map[2][1]; }
        if(KEY_C3==0) { while(KEY_C3==0); return key_map[2][2]; }
        if(KEY_C4==0) { while(KEY_C4==0); return key_map[2][3]; }

        // Scan Row 4
        KEY_R1=1; KEY_R2=1; KEY_R3=1; KEY_R4=0;
        if(KEY_C1==0) { while(KEY_C1==0); return key_map[3][0]; }
        if(KEY_C2==0) { while(KEY_C2==0); return key_map[3][1]; }
        if(KEY_C3==0) { while(KEY_C3==0); return key_map[3][2]; }
        if(KEY_C4==0) { while(KEY_C4==0); return key_map[3][3]; }
    }
}

//==================================================
// LCD DRIVER FUNCTIONS (8-BIT MODE)
//==================================================

void lcd_init(void) {
    delay_ms(20);
    lcd_send_command(0x38); // Function Set: 8-bit mode, 2 lines, 5x7 font
    lcd_send_command(0x0C); // Display ON, Cursor OFF
    lcd_send_command(0x01); // Clear display screen
    lcd_send_command(0x06); // Entry Mode: Increment cursor
}

void lcd_send_command(unsigned char cmd) {
    LCD_DATA_PORT = cmd;
    LCD_RS = 0; // RS=0 for command
    LCD_RW = 0; // RW=0 for write
    LCD_E  = 1; // Pulse Enable
    delay_ms(1);
    LCD_E  = 0;
    delay_ms(2);
}

void lcd_send_data(unsigned char data_val) {
    LCD_DATA_PORT = data_val;
    LCD_RS = 1; // RS=1 for data
    LCD_RW = 0; // RW=0 for write
    LCD_E  = 1; // Pulse Enable
    delay_ms(1);
    LCD_E  = 0;
    delay_ms(2);
}

void lcd_string(char *str) {
    while(*str != '\0') {
        lcd_send_data(*str);
        str++;
    }
}

void lcd_goto_xy(unsigned char row, unsigned char col) {
    unsigned char address;
    if (row == 0) {
        address = 0x80 + col;
    } else {
        address = 0xC0 + col;
    }
    lcd_send_command(address);
}

void lcd_int(unsigned int num) {
    char buffer[6];
    sprintf(buffer, "%u", num); // Use sprintf to convert int to string
    lcd_string(buffer);
}

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++); // Calibrated for ~12MHz crystal
    }
}

