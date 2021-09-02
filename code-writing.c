#include <stdio.h>
#include <string.h>
#include <inttypes.h>

//lookup tables for getting index which correlates to the use_index
static const uint8_t lookup_table[] = "\0\0\0\0abcdefghijklmnopqrstuvwxyz1234567890\n\0\0\t -=[]\\#;'`,./";
static const uint8_t shift_table[] = "\0\0\0\0ABCDEFGHIJKLMNOPQRSTUVWXYZ!\"£$%%^&*()\0\0\0\0\0_+{}|~:@¬<>?";

//the key has to be released after every press if normal keys
void release_key()
{
    uint8_t nullArr[] = "\0\0\0\0\0\0\0\0"; //code sent
    FILE *fp;
    fp = fopen("/dev/hidg0", "wb"); //writes to file which is what's "sent" to pc
    fwrite(nullArr, 8, 1, fp);
    fclose(fp);
    return;
}

//press the windows key + any other
void win_key(uint8_t other_key)
{
    uint8_t winArr[] = "\x08\0\0\0\0\0\0\0"; //code sent
    FILE *fp;
    fp = fopen("/dev/hidg0", "wb"); //writes to file which is what's "sent" to pc
    fwrite(winArr, 8, 1, fp);
    fclose(fp);
    release_key();
    return;
}

//put correct values into the array being sent to the target
void get_array(uint8_t character, uint8_t charArr[])
{
    uint8_t index;
    for (index = 0; index < sizeof(lookup_table); index++)
    {
        if (lookup_table[index] == character)
        {
            charArr[2] = index;
            return;
        }
    }
    for (index = 0; index < sizeof(shift_table); index++)
    {
        if (shift_table[index] == character)
        {
            charArr[0] = 32;
            charArr[2] = index;
            return;
        }
    }
    return;
}

//write a single character into hidg0
void write_character(uint8_t character)
{
    uint8_t lookup_index;
    uint8_t charArr[] = "\0\0\0\0\0\0\0\0";
    get_array(character, charArr);
    FILE *fp;
    fp = fopen("/dev/hidg0", "wb");
    fwrite(charArr, 8, 1, fp);
    fclose(fp);
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
    write_string("yes sir");
    win_key('r');
    return 0;
}