#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

//lookup tables for getting index which correlates to the use_index
static const uint8_t g_lookupTable[] = "\0\0\0\0abcdefghijklmnopqrstuvwxyz1234567890\r\x1b\x7f\t -=[]\\#;'`,./";
static const uint8_t g_shiftTable[] = "\0\0\0\0ABCDEFGHIJKLMNOPQRSTUVWXYZ!\"\x9c$\x25^&*()\n\0\b\0\0_+{}|~:@\xaa<>?";

enum ModifierKey
{
    NONE = 0,
    CTRL = 1 << 0,
    SHIFT = 1 << 1,
    ALT = 1 << 2,
    GUI = 1 << 3,
};

static enum ModifierKey get_modifier(const uint8_t* input, size_t len)
{
    if (len != 5) { return NONE; }
    uint8_t modifiers[4][5] =    {
            "CTRL", // 1 << 0 = 1
            "SHFT", // 1 << 1 = 2
            "ALT\0", // 1 << 2 = 4
            "GUI\0", // 1 << 3 = 8
    };
    uint8_t mask = NONE | CTRL | SHIFT | ALT | GUI;
    for (uint8_t i = 0; i < 5; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            mask &= ~((input[i] != modifiers[j][i]) << j);
        }
    }
    // Must be a single flag because there are no equal strings
    return (enum ModifierKey) mask;
}

static void delay(long milliseconds)
{
    struct timespec waitTime;
    waitTime.tv_sec = milliseconds / 1000;
    waitTime.tv_nsec = (milliseconds % 1000) * 1000000;
    while (-1 == nanosleep(&waitTime, &waitTime));
}

static void send_code(uint8_t hidCode[9])
{
    for (int i = 0; i < 8; i++)
    {
        printf("|%d|", hidCode[i]);
    }
    printf("\n");
    //writes to file which is what's "sent" to pc
    FILE* fp;
    fp = fopen("/dev/hidg0", "wb");
    if (fp == NULL)
    {
        printf("Failed to open /dev/hidg0\n");
        exit(EXIT_FAILURE);
    }
    fwrite(hidCode, 8, 1, fp);
    fclose(fp);
}

/// the key has to be released after every press if normal keys
static void release_key()
{
    uint8_t nullArr[9] = "\0\0\0\0\0\0\0\0"; //code sent
    send_code(nullArr);
}

/// get the selector id for a character (lower as shift requires modifier)
static uint8_t get_selector_val(uint8_t character)
{
    uint8_t index;
    //read thru lookup table for lower (non shift) uint8_ts
    for (index = 0; index < (uint8_t) sizeof(g_lookupTable); index++)
    {
        if (g_lookupTable[index] == character)
        {
            return index;
        }
    }
    return 255;
}

/// iterates through the string given, applying relevant operations on the HID code depending on what keys are passed in
static void hold_keys(char keysHeld[])
{
    uint8_t tmp_key[5] = "\0\0\0\0\0";
    uint8_t char_count = 0;
    uint8_t hidCode[9] = "\0\0\0\0\0\0\0\0";
    //iterate through given string
    for (unsigned long x = 0; x < strlen(keysHeld); x++)
    {
        char current_char = keysHeld[x];
        //delimited by spaces, so once finds space or \0 then record the key and does operation
        if (current_char == ' ' || current_char == '\0')
        {
            char_count = 0;
            // add modifier bit to the modifier value in hidCode
            enum ModifierKey modifier = get_modifier(tmp_key, 5);
            hidCode[0] |= modifier;
            if (modifier == NONE)
            {
                //get the selector val for any keys entered - in current condition only one specifier keypress can be done at any one time
                uint8_t selector_val = get_selector_val(tmp_key[0]);
                if (selector_val == 255)
                {
                    printf("There was an incorrect string (%s) sent to the hold_keys function, exiting...\n", tmp_key);
                    for (int i = 0; i < 5; i++)
                    {
                        printf("<%d>", tmp_key[i]);
                    }
                    exit(EXIT_FAILURE);
                }
                hidCode[2] = selector_val;
            }
            for (int i = 0; i < 5; i++)
            {
                tmp_key[i] = '\0';
            }
            x++;
        }
        tmp_key[char_count++] = (uint8_t)keysHeld[x];
    }
    send_code(hidCode);
}

//put correct values into the array being sent to the target
static void get_array(char character, uint8_t hidCode[])
{
    uint8_t index;
    for (index = 0;
         index < (uint8_t) sizeof(g_lookupTable); index++) //read thru lookup table for lower (non shift) uint8_ts
    {
        if (g_lookupTable[index] == character)
        {
            hidCode[2] = index;
            return;
        }
    }
    for (index = 0;
         index < (uint8_t) sizeof(g_shiftTable); index++) //read through lookup table for upper (shift) uint8_ts
    {
        if (g_shiftTable[index] == character)
        {
            hidCode[0] = 32;
            hidCode[2] = index;
            return;
        }
    }
}

//write a single character into hidg0
static void write_character(char character)
{
    uint8_t hidCode[9] = "\0\0\0\0\0\0\0\0";
    get_array(character, hidCode);
    send_code(hidCode);
    release_key();
}

//iterate through a string and write each letter into hidg0
static void write_string(char strIn[])
{
    for (size_t i = 0; i < strlen(strIn); i++)
    {
        write_character(strIn[i]);
    }
}

int main()
{
    delay(100);
    hold_keys("GUI r");
    release_key();
    delay(500);
    write_string("https://www.youtube.com/watch?v=dQw4w9WgXcQ");
    delay(50);
    write_character('\n');
    return EXIT_SUCCESS;
}
