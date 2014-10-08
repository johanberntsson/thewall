/* (c) 2014 Johan Berntsson
 *
 * written for the cc65 cross compiler for 6502 computers,
 * tested on commodore 64
 *
 * Released under the GNU GENERAL PUBLIC LICENSE version 2
 */

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <peekpoke.h>

static unsigned char colors[] = { COLOR_RED, COLOR_GREEN, COLOR_YELLOW };

/* Sound effects from sid.c */
extern void initSid();
extern void playOneTone(char freqIndex);
extern void playThreeTones(char freqIndex0, char freqIndex1, char freqIndex2);
extern int play_melody(unsigned char __fastcall__ (*callback)(void));


/* C64 screen size defaults */
static unsigned char screen_width = 40, screen_height = 25;
static unsigned char game_width = 15, game_height = 15;
static unsigned char offset_x = 15, offset_y = 5;

void print_centered(unsigned char y, const char *text) {
    unsigned char n = strlen(text);
    unsigned char x = (screen_width - n)/ 2;
    gotoxy((screen_width - n)/ 2, y);
    cprintf(text);
}

unsigned char callback_pressanykey(void) {
    // 0 if no key pressed, 
    // otherwise 1... (# of keys in the buffer)
    return PEEK(198);
}

void show_intro(void) {
    unsigned char n1 = screen_height / 4;
    unsigned char n2 = screen_height / 2;
    unsigned char n3 = screen_height - n1;

    // Reset colours
    clrscr();
    bordercolor(COLOR_BLACK);
    bgcolor(COLOR_BLACK);

    textcolor(COLOR_WHITE);
    print_centered(n1, "The Wall");
    textcolor(COLOR_YELLOW);
    print_centered(n2, "By Johan Berntsson, 2014");
    textcolor(COLOR_RED);
    print_centered(n3, "Press any key");

    play_melody(callback_pressanykey);
    cgetc();
}

void draw_game(void) {
    unsigned char x, y;

    // Reset colours
    clrscr();
    bordercolor(COLOR_BLACK);
    bgcolor(COLOR_BLACK);

    textcolor(COLOR_WHITE);
    print_centered(0,  "The Wall");
    print_centered(2,  "Keys: a,s,w,z,space. q to quit");

    // draw frame
    textcolor(COLOR_BLUE);
    cvlinexy(offset_x - 1 , offset_y, game_height);
    cvlinexy(offset_x + game_width, offset_y, game_height);
    chlinexy(offset_x, offset_y - 1, game_width);
    chlinexy(offset_x, offset_y + game_height, game_width);
    cputcxy(offset_x - 1 , offset_y - 1, 0xb0);
    cputcxy(offset_x + game_width , offset_y - 1, 0xae);
    cputcxy(offset_x - 1, offset_y + game_height , 0xad);
    cputcxy(offset_x + game_width , offset_y + game_height, 0xbd);

    // Draw bricks
    for(x = 0; x < game_width; x++) {
        for(y = 0; y < game_height; y++) {
            textcolor(colors[rand() %3]);
            cputcxy(offset_x + x, offset_y + y, '*');
        }
    }
}

void main(void) {
    unsigned char key, x, y, new_x, new_y, snd_effect;

    // should be 40*25, but check to make sure
    screensize(&screen_width, &screen_height);

    // calculate offsets
    offset_x = (screen_width - game_width)/2;
    offset_y = (screen_height - game_height)/2;
    if(offset_x <= 2 || offset_y <=2) {
        // leave some space for the frame and text
        printf("The screen is too small!\n");
        return;
    }

	initSid();
    show_intro();
    draw_game();

    // init cursor
    x = 0;
    y = 0;

    // enable keyboard repeat
    POKE(650, 128);

    // wait for a key
    cursor(1);
    for(;;) {
        new_x = x;
        new_y = y;
        snd_effect = 0;
        gotoxy(offset_x + x, offset_y + y);
        key = cgetc();

        switch(key) {
            case 'q':
                clrscr();
                textcolor(COLOR_WHITE);
                printf("Thank you for playing this game!");
                return;
            case 'a':
                --new_x;
                snd_effect = 1;
                break;
            case 's':
                ++new_x;
                snd_effect = 2;
                break;
            case 'w':
                --new_y;
                snd_effect = 3;
                break;
            case 'z':
                ++new_y;
                snd_effect = 4;
                break;
            case ' ':
                // TODO
                // here the magic happens
                playThreeTones(0, 2, 5);
                playThreeTones(0, 2, 4);
                playThreeTones(0, 2, 5);
                break;
            default:
                break;
        }
        if(new_x < 0) new_x = 0;
        if(new_y < 0) new_y = 0;
        if(new_x >= game_width) new_x = game_width - 1;
        if(new_y >= game_height) new_y = game_height - 1;

        if(x != new_x || y != new_y) {
            if(callback_pressanykey() == 0) {
                playOneTone(snd_effect);
                playThreeTones(0, 2, 4);
            }
            x = new_x;
            y = new_y;
        }
    }
}
