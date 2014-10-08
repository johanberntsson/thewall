/* (c) 2014 Johan Berntsson
 *
 * written for the cc65 cross compiler for 6502 computers,
 * tested on commodore 64
 *
 * Released under the GNU GENERAL PUBLIC LICENSE version 2
 */

#include <conio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <peekpoke.h>

#define MAX_WIDTH 15
#define MAX_HEIGHT 15

static unsigned char colors[] = { COLOR_RED, COLOR_GREEN, COLOR_YELLOW };

/* Sound effects from sid.c */
extern void initSid();
extern void playOneTone(uint8_t freqIndex);
extern void playThreeTones(uint8_t freqIndex0, uint8_t freqIndex1, uint8_t freqIndex2);
extern int play_melody(unsigned char __fastcall__ (*callback)(void));


/* C64 screen size defaults */
unsigned char screen_width = 40, screen_height = 25;
unsigned char game_width = MAX_WIDTH, game_height = MAX_HEIGHT;
unsigned char offset_x = 15, offset_y = 5;

typedef struct {
    unsigned char color;
    unsigned char state;
} Tile;

typedef struct {
    unsigned char x;
    unsigned char y;
} Coordinate;

Tile map[MAX_WIDTH][MAX_HEIGHT];

int queue_len;
Coordinate queue[MAX_WIDTH*MAX_HEIGHT];

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

void init_map(void) {
    unsigned char x, y;
    for(x = 0; x < game_width; x++) {
        for(y = 0; y < game_height; y++) {
            map[x][y].state = '*';
            map[x][y].color = colors[rand() %3];
        }
    }
}

unsigned char draw_bricks()
{
    unsigned char x, y, n = 0;
    for(x = 0; x < game_width; x++) {
        for(y = 0; y < game_height; y++) {
            textcolor(map[x][y].color);
            if(map[x][y].state == '*') ++n;
            cputcxy(offset_x + x, offset_y + y, map[x][y].state);
        }
    }
    return n;
}

unsigned char only_single_bricks_left()
{
    unsigned char x, y, xx, yy, color, ret_val = 1;
    for(x = 0; ret_val == 1 && x < game_width; x++) {
        for(y = 0; ret_val == 1 && y < game_height; y++) {
            if(map[x][y].state != '*') continue;
            color =  map[x][y].color;
            xx = x - 1; if(xx == 255) xx = x;
            if(xx != x && map[xx][y].state == '*' && map[xx][y].color == color) ret_val = 0;
            xx = x + 1; if(xx >= game_width) xx = x;
            if(xx != x && map[xx][y].state == '*' && map[xx][y].color == color) ret_val = 0;
            yy = y - 1; if(yy == 255) yy = y;
            if(yy != y && map[x][yy].state == '*' && map[x][yy].color == color) ret_val = 0;
            yy = y + 1; if(yy >= game_height) yy = y;
            if(yy != y && map[x][yy].state == '*' && map[x][yy].color == color) ret_val = 0;
        }
    }
    return ret_val;
}

void draw_game(void) {

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

    draw_bricks();
}

void compact_vertically()
{
    unsigned char x, y, z;
    for(x = 0; x < game_width; x++) {
        for(y = game_height - 1; y < 255; y--) {
            if(map[x][y].state == ' ') {
                for(z = y - 1; z < 255 && map[x][z].state == ' '; z--);
                if(z != 255) {
                    map[x][y].state = map[x][z].state;
                    map[x][y].color = map[x][z].color;
                    map[x][z].state = ' ';
                }
            }
        }
    }
}

void compact_horizontally()
{
    unsigned char x, y, z;
    for(y = 0; y < game_height; y--) {
        for(x = 0; x < game_width; x++) {
            if(map[x][y].state == ' ') {
                for(z = x + 1; z < game_width && map[z][y].state == ' '; z++);
                if(z < game_width) {
                    map[x][y].state = map[z][y].state;
                    map[x][y].color = map[z][y].color;
                    map[z][y].state = ' ';
                }
            }
        }
    }
}

void remove_brick(unsigned char x, unsigned char y) {
    map[x][y].state = ' ';
    //cputcxy(offset_x + x, offset_y + y, map[x][y].state);
}

void add_brick_to_queue(unsigned char x, unsigned char y, unsigned char color)
{
    if(x == 255) x = 0;
    if(y == 255) y = 0;
    if(x >= game_width) x = game_width - 1;
    if(y >= game_height) y = game_height - 1;
    if(map[x][y].state == '*' && map[x][y].color == color) {
        queue[queue_len].x = x;
        queue[queue_len].y = y;
        ++queue_len;
    }
}

void break_bricks(unsigned char x, unsigned char y)
{
    unsigned char xx, yy, color, first;
    color = map[x][y].color;
    // first entry
    queue_len = 1;
    queue[0].x = x;
    queue[0].y = y;
    first = 1;
    while(queue_len > 0) {
        --queue_len;
        x = queue[queue_len].x;
        y = queue[queue_len].y;
        remove_brick(x, y);
        add_brick_to_queue(x - 1, y, color);
        add_brick_to_queue(x + 1, y, color);
        add_brick_to_queue(x, y - 1, color);
        add_brick_to_queue(x, y + 1, color);
        if(first == 1 && queue_len == 0) {
            // not allowed to remove only one brick
            // put it back
            map[x][y].state = '*';
        } else {
            first = 0;
        }
    }
}

int play_game()
{
    unsigned char key, x, y, new_x, new_y, snd_effect;

    init_map();
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
                return 0;
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
                break_bricks(x, y);
                compact_vertically();
                //compact_horizontally();
                if(draw_bricks() == 0) {
                    textcolor(COLOR_WHITE);
                    print_centered(screen_height/2, "YOU WIN!");
                    playThreeTones(0, 2, 2);
                    playThreeTones(0, 2, 2);
                    playThreeTones(0, 2, 2);
                    playThreeTones(0, 2, 6);
                    cgetc();
                    return 1;
                }
                if(only_single_bricks_left()) {
                    textcolor(COLOR_WHITE);
                    print_centered(screen_height/2, "YOU WIN!");
                    playThreeTones(0, 2, 3);
                    playThreeTones(0, 2, 2);
                    playThreeTones(0, 2, 1);
                    cgetc();
                    return 1;
                }
                playThreeTones(0, 2, 5);
                playThreeTones(0, 2, 4);
                playThreeTones(0, 2, 5);
                break;
            default:
                break;
        }
        if(new_x == 255) new_x = 0;
        if(new_y == 255) new_y = 0;
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

void main(void) {

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

    while(play_game());
}

