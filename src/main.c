/* (c) 2014,2020 Johan Berntsson
 *
 * written for the cc65 cross compiler for 6502 computers,
 * tested on commodore 64
 *
 * 2020 version: added bonus for more smashed bricks, improved sound
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

#define SPRITE_PTR  0x07F8

#define JOY2  0xDC00
#define JOYUP  0x01
#define JOYDOWN  0x02
#define JOYLEFT  0x04
#define JOYRIGHT  0x08
#define JOYFIRE  0x10

#define WAIT_WHILE_RASTERLINE_LOW    while (!(VIC.ctrl1 & 0x80)) {};
#define WAIT_WHILE_RASTERLINE_HIGH   while (VIC.ctrl1 & 0x80) {};

/* Sprites defined in sprites.s, must be 64 byte aligned */
extern const unsigned char sprites[64][]; 

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

void print_centered_1arg(unsigned char y, const char *text, void *arg) {
    unsigned char n = strlen(text);
    unsigned char x = (screen_width - n)/ 2;
    gotoxy((screen_width - n)/ 2, y);
    cprintf(text, arg);
}

unsigned char callback_pressanykey(void) {
    // 0 if no key pressed, 
    // otherwise 1... (# of keys in the buffer)
    return PEEK(198);
}

void show_intro(void) {
    unsigned char n1 = screen_height / 8;
    unsigned char n2 = screen_height / 4;
    unsigned char n3 = screen_height / 2;

    // Reset colours
    clrscr();
    bordercolor(COLOR_BLACK);
    bgcolor(COLOR_BLACK);

    textcolor(COLOR_WHITE);
    print_centered(n1, "The Wall");
    textcolor(COLOR_YELLOW);
    print_centered(n2, "By Johan Berntsson, 2014-2020");
    print_centered(n2+2,  "Keys: move with a,s,w,z or cursor keys");
    print_centered(n2+3,  "space to select, q to quit");
    textcolor(COLOR_GREEN);
    print_centered(n3, "Push space to crush all");
    print_centered(n3+1, "connected bricks of the same colour");
    print_centered(n3+3, "The more bricks you crush each");
    print_centered(n3+4, "time, the higher the score");
    print_centered(n3+6, "You win if you clear all bricks");
    print_centered(n3+7, "You lose if only single bricks remain");
    textcolor(COLOR_RED);
    print_centered(n3+9, "Press any key");

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
    print_centered(2,  "Score: 0");
    //print_centered(2,  "Keys: a,s,w,z,space. q to quit");

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
    unsigned char x, xx, x1, x2, y;
    for(x1 = 0; x1 < game_width && map[x1][game_height - 1].state == ' '; x1++);
    if(x1 >= game_width) return;
    for(x2 = game_width - 1; x2 != 255 && map[x1][game_height - 1].state == ' '; x1--);
    if(x1 == 255) return;

    // x1 is the first non-empty column
    // x2 is the last non-empty column
    for(x = x1; x < x2; x++) {
        while(x < x2 && map[x][game_height - 1].state == ' ') {
            // found an empty column, eliminate it
            for(xx = x; xx < x2; xx++) {
                for(y = 0; y < game_height; y++) {
                    map[xx][y].state = map[xx + 1][y].state;
                    map[xx][y].color = map[xx + 1][y].color;
                }
            }
            // clear the rightmost colum (since we moved it leftwards)
            for(y = 0; y < game_height; y++) {
                map[x2][y].state = ' ';
            }
            --x2;
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

unsigned char break_bricks(unsigned char x, unsigned char y)
{
    unsigned char color, first;
    unsigned char smashed_bricks = 0;
    if(map[x][y].state != '*') return 0;

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
        ++smashed_bricks;
        add_brick_to_queue(x - 1, y, color);
        add_brick_to_queue(x + 1, y, color);
        add_brick_to_queue(x, y - 1, color);
        add_brick_to_queue(x, y + 1, color);
        if(first == 1 && queue_len == 0) {
            // not allowed to remove only one brick
            // put it back
            //map[x][y].state = '*';
            //smashed_bricks = 0;
        } else {
            first = 0;
        }
    }
    return smashed_bricks;
}

void set_sprite(int xx, int yy) {
    // offsets needed to get the cursor on top of
    // the character at xx,yy
    xx = 8*xx + 24;
    yy = 8*yy + 48 + 2;
    VIC.spr0_x = xx & 0xff;
    VIC.spr0_y = yy & 0xff;
}

int play_game()
{
    int score = 0;
    unsigned char smashed_bricks, bonus;
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
        set_sprite(offset_x + x , offset_y + y);
        key = cgetc();

        switch(key) {
            case 'q':
            case 3:
                return 0;
            case 'a':
            case 157:
                // left
                --new_x;
                snd_effect = 1;
                break;
            case 's':
            case 29:
                // right
                ++new_x;
                snd_effect = 2;
                break;
            case 'w':
            case 145:
                // up
                --new_y;
                snd_effect = 3;
                break;
            case 'z':
            case 17:
                // down
                ++new_y;
                snd_effect = 4;
                break;
            case ' ':
                smashed_bricks = break_bricks(x, y);
                compact_vertically();
                compact_horizontally();
                // give bonus for smashing more bricks
                score += smashed_bricks;
                bonus = smashed_bricks;
                while(bonus > 3) {
                    bonus -= 3;
                    score += bonus;
                }
                print_centered_1arg(2,  "Score: %d ", score);
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
                    print_centered(screen_height/2, "YOU LOST!");
                    playThreeTones(0, 2, 6);
                    playThreeTones(0, 2, 5);
                    playThreeTones(0, 2, 4);
                    cgetc();
                    return 1;
                }
                if(smashed_bricks > 0) {
                    //playThreeTones(0, 2, 5);
                    //playThreeTones(0, 2, 4);
                    //playThreeTones(0, 2, 5);
                }
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
                //playOneTone(snd_effect);
                //playThreeTones(0, 2, 4);
            }
            x = new_x;
            y = new_y;
        }
    }
}

void main(void) {
    int sprite_base = ((int) sprites/64);

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

    // test sprite
    POKE(SPRITE_PTR, sprite_base); // sprite 0 data
    VIC.spr_ena = 1;
    VIC.spr0_color = COLOR_WHITE;

    while(play_game());

    VIC.spr_ena = 0;

    clrscr();
    textcolor(COLOR_WHITE);
    printf("Thank you for playing this game!");
}

