#include <reg52.h>
#include <stdio.h>

//==================================================
// HARDWARE PIN DEFINITIONS
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
void show_vote_confirmation(unsigned char candidate);

//==================================================
// GLOBAL VARIABLES
//==================================================
int vote_count[4] = {0, 0, 0, 0}; // Vote counts for P1–P4

//==================================================
// MAIN FUNCTION
//==================================================
void main() {
    char key;

    lcd_init();
    

    // Startup Message
    lcd_goto_xy(0, 2);
    lcd_string("Electronic");
    lcd_goto_xy(1, 1);
    lcd_string("Voting Machine");
    delay_ms(2000);

    while(1) {
        lcd_send_command(0x01); // Clear screen
        lcd_goto_xy(0, 0);
        lcd_string("P1(1) P2(2)");
        lcd_goto_xy(1, 0);
        lcd_string("P3(3) P4(4) #=Res"); // Physical '#' shows results

        key = get_key(); // Wait for key press

        switch(key) {
            case '1': // Physical key '1'
                vote_count[0]++;
                show_vote_confirmation(1);
                break;
            case '2': // Physical key '2'
                vote_count[1]++;
                show_vote_confirmation(2);
                break;
            case '3': // Physical key '3'
                vote_count[2]++;
                show_vote_confirmation(3);
                break;
            case '4': // Physical key '4'
                vote_count[3]++;
                show_vote_confirmation(4);
                break;
            case '#': // Physical key '#'
                show_results();
                while(1); // Halt after showing results
            default:
                continue; // Ignore other keys (A, B, C, D, *, 0, 5, 6, 7, 8, 9)
        }
    }
}



void show_vote_confirmation(unsigned char candidate) {
    lcd_send_command(0x01); // Clear screen
    lcd_goto_xy(0, 0);
    lcd_string("Vote Recorded for:");
    lcd_goto_xy(1, 6);
    lcd_string("P");
    lcd_send_data(candidate + '0');

    
    delay_ms(1200);
}

//==================================================
// APPLICATION: SHOW RESULTS
//==================================================
void show_results() {
    int i, max_votes = -1, winner = 0;
    bit tie = 0;

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

    delay_ms(3500);

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
    if (max_votes <= 0) {
        lcd_string("No Votes Cast!");
    } else if (tie == 1) {
        lcd_string("Result is a TIE!");
    } else {
        lcd_string("Winner is: P");
        lcd_send_data(winner + '0');
    }

    
}

//==================================================
// KEYPAD SCAN (blocking)
//==================================================
char get_key(void) {
    const unsigned char key_map[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    delay_ms(50); // Small debounce

    while(1) {
        // Row 1 active low
        KEY_R1 = 0; KEY_R2 = 1; KEY_R3 = 1; KEY_R4 = 1;
        if (KEY_C1 == 0) { while (KEY_C1 == 0); return key_map[0][0]; } // Physical '1' -> '1'
        if (KEY_C2 == 0) { while (KEY_C2 == 0); return key_map[0][1]; } // Physical '2' -> '2'
        if (KEY_C3 == 0) { while (KEY_C3 == 0); return key_map[0][2]; } // Physical '3' -> '3'
        if (KEY_C4 == 0) { while (KEY_C4 == 0); return key_map[0][3]; } // Physical 'A' -> 'A'

        // Row 2
        KEY_R1 = 1; KEY_R2 = 0; KEY_R3 = 1; KEY_R4 = 1;
        if (KEY_C1 == 0) { while (KEY_C1 == 0); return key_map[1][0]; } // Physical '4' -> '4'
        if (KEY_C2 == 0) { while (KEY_C2 == 0); return key_map[1][1]; } // Physical '5' -> '5'
        if (KEY_C3 == 0) { while (KEY_C3 == 0); return key_map[1][2]; } // Physical '6' -> '6'
        if (KEY_C4 == 0) { while (KEY_C4 == 0); return key_map[1][3]; } // Physical 'B' -> 'B'

        // Row 3
        KEY_R1 = 1; KEY_R2 = 1; KEY_R3 = 0; KEY_R4 = 1;
        if (KEY_C1 == 0) { while (KEY_C1 == 0); return key_map[2][0]; } // Physical '7' -> '7'
        if (KEY_C2 == 0) { while (KEY_C2 == 0); return key_map[2][1]; } // Physical '8' -> '8'
        if (KEY_C3 == 0) { while (KEY_C3 == 0); return key_map[2][2]; } // Physical '9' -> '9'
        if (KEY_C4 == 0) { while (KEY_C4 == 0); return key_map[2][3]; } // Physical 'C' -> 'C'

        // Row 4
        KEY_R1 = 1; KEY_R2 = 1; KEY_R3 = 1; KEY_R4 = 0;
        if (KEY_C1 == 0) { while (KEY_C1 == 0); return key_map[3][0]; } // Physical '*' -> '*'
        if (KEY_C2 == 0) { while (KEY_C2 == 0); return key_map[3][1]; } // Physical '0' -> '0'
        if (KEY_C3 == 0) { while (KEY_C3 == 0); return key_map[3][2]; } // Physical '#' -> '#'
        if (KEY_C4 == 0) { while (KEY_C4 == 0); return key_map[3][3]; } // Physical 'D' -> 'D'
    }
}

//==================================================
// LCD DRIVER FUNCTIONS (8-BIT MODE)
//==================================================
void lcd_init(void) {
    delay_ms(20);
    lcd_send_command(0x38); // 8-bit, 2 lines, 5x7 font
    delay_ms(1);
    lcd_send_command(0x0C); // Display ON, Cursor OFF
    delay_ms(1);
    lcd_send_command(0x01); // Clear
    delay_ms(2);
    lcd_send_command(0x06); // Increment cursor
    delay_ms(1);
}

void lcd_send_command(unsigned char cmd) {
    LCD_DATA_PORT = cmd;
    LCD_RS = 0; LCD_RW = 0;
    LCD_E  = 1; delay_ms(1);
    LCD_E  = 0; delay_ms(2);
}

void lcd_send_data(unsigned char data_val) {
    LCD_DATA_PORT = data_val;
    LCD_RS = 1; LCD_RW = 0;
    LCD_E  = 1; delay_ms(1);
    LCD_E  = 0; delay_ms(2);
}

void lcd_string(char *str) {
    while (*str != '\0') lcd_send_data(*str++);
}

void lcd_goto_xy(unsigned char row, unsigned char col) {
    unsigned char address = (row == 0) ? 0x80 + col : 0xC0 + col;
    lcd_send_command(address);
}

void lcd_int(unsigned int num) {
    char buffer[6];
    sprintf(buffer, "%u", num);
    lcd_string(buffer);
}

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++); // calibrated for ~12MHz
}.          Explain this code for evm using 8051 in c in detail line to line
