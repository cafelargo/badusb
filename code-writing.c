#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

//lookup tables for getting index which correlates to the use_index
static const uint8_t g_lookupTable[] = "\0\0\0\0abcdefghijklmnopqrstuvwxyz1234567890\r\x1b\x7f\t -=[]\\#;'`,./";
static const uint8_t g_shiftTable[] = "\0\0\0\0ABCDEFGHIJKLMNOPQRSTUVWXYZ!\"\x9c$\x25^&*()\n\0\b\0\0_+{}|~:@\xaa<>?";
#define CTRL 0b00000001
#define SHFT 0b00000010
#define ALT 0b00000100
#define GUI 0b00001000
#define ESC 0x1b
#define DEL 0x7f

//modified code taken from https://c-for-dummies.com/blog/?p=69 to wait for x number of milliseconds
void delay(useconds_t milliseconds)
{
    usleep(milliseconds * 1000);
    return;
}

void send_code(uint8_t hidCode[9])
{
    for (int i = 0; i < 8; i++)
    {
        printf("|%d|", hidCode[i]);
    }
    printf("\n");
    FILE *fp;
    fp = fopen("/dev/hidg0", "wb"); //writes to file which is what's "sent" to pc
    fwrite(hidCode, 8, 1, fp);
    fclose(fp);
    return;
}

//the key has to be released after every press if normal keys
void release_key()
{
    uint8_t nullArr[9] = "\0\0\0\0\0\0\0\0"; //code sent
    send_code(nullArr);
    return;
}

//get the selector id for a character (lower as shift requires modifier)
uint8_t get_selector_val(uint8_t character)
{
    uint8_t index;
    for (index = 0; index < sizeof(g_lookupTable); index++) //read thru lookup table for lower (non shift) chars
    {
        if (g_lookupTable[index] == character)
        {
            return index;
        }
    }
    return 255;
}

//iterates through the string given, applying relevant operations on the HID code depending on what keys are passed in
void hold_keys(uint8_t keysHeld[])
{
    uint8_t tmp_key[5] = "\0\0\0\0\0";
    uint8_t char_count = 0;
    uint8_t selector_val = 0;
    uint8_t hidCode[9] = "\0\0\0\0\0\0\0\0";
    uint8_t currentChar;
    for (int x = 0; x <= strlen(keysHeld); x++) //iterate through given string
    {
        currentChar = keysHeld[x];
        if(currentChar == ' ' || currentChar  == '\0') //delimited by spaces - so once finds space or \0 then record the key and does operation
        {
            char_count = 0;
            if (strcmp(tmp_key, "CTRL") == 0) //if string matches one of these codes, perform bitwise or to add modifier bit to the modifier value in hidCode
            {
                hidCode[0] = hidCode[0] | CTRL;
            }
            else if (strcmp(tmp_key, "SHFT") == 0)
            {
                hidCode[0] = hidCode[0] | SHFT;
            }
            else if (strcmp(tmp_key, "ALT") == 0)
            {
                hidCode[0] = hidCode[0] | ALT;
            }
            else if (strcmp(tmp_key, "GUI") == 0)
            {
                hidCode[0] = hidCode[0] | GUI;
            }
            else
            {
                selector_val = get_selector_val(tmp_key[0]); //get the selector val for any keys entered - in current condition only one specifier keypress can be done at any one time
                if (selector_val == 255)
                {
                    printf("There was an incorrect string (%s) sent to the hold_keys function, exiting...\n", tmp_key);
                    for (int i = 0; i < strlen(tmp_key); i++)
                    {
                        printf("<%d>",tmp_key[i]);
                    }
                    exit(1);
                }
                hidCode[2] = selector_val;
            }
            for (int i = 0; i < 5; i++)
            {
                tmp_key[i] = '\0';
            }
            x++;
        }
        tmp_key[char_count++] = keysHeld[x];
    }
    send_code(hidCode);
    return;
}

//put correct values into the array being sent to the target
void get_array(uint8_t character, uint8_t hidCode[])
{
    uint8_t index;
    for (index = 0; index < sizeof(g_lookupTable); index++) //read thru lookup table for lower (non shift) chars
    {
        if (g_lookupTable[index] == character)
        {
            hidCode[2] = index;
            return;
        }
    }
    for (index = 0; index < sizeof(g_shiftTable); index++) //read through lookup table for upper (shift) chars
    {
        if (g_shiftTable[index] == character)
        {
            hidCode[0] = 32;
            hidCode[2] = index;
            return;
        }
    }
    return;
}

//write a single character into hidg0
void write_character(uint8_t character)
{
    uint8_t hidCode[9] = "\0\0\0\0\0\0\0\0";
    get_array(character, hidCode);
    send_code(hidCode);
    release_key();
    return;
}

//iterate through a string and write each letter into hidg0
void write_string(char strIn[])
{
    for (int i; i < strlen(strIn); i++)
    {
        write_character(strIn[i]);
    }
    return;
}

int main()
{
    hold_keys("GUI r");
    release_key();
    delay(500);
    write_string("https://www.youtube.com/watch?v=dQw4w9WgXcQ\n");
    return 0;
}