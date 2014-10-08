#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char screen_height, screen_width;
static unsigned char game_width = 15, game_height = 15;
static unsigned char offset_x, offset_y;
static unsigned char colors[] = { COLOR_RED, COLOR_GREEN, COLOR_YELLOW };

void print_centered(unsigned char y, const char *text) {
    unsigned char n = strlen(text);
    unsigned char x = (screen_width - n)/ 2;
    gotoxy((screen_width - n)/ 2, y);
    cprintf(text);
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
    unsigned char key, cx, cy;

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

    show_intro();
    draw_game();

    // init cursor
    cx = 0;
    cy = 0;

    // wait for a key
    cursor(1);
    for(;;) {
        gotoxy(offset_x + cx, offset_y + cy);
        key = cgetc();

        switch(key) {
            case 'q':
                clrscr();
                textcolor(COLOR_WHITE);
                printf("Thank you for playing this game!");
                return;
            case 'a':
                --cx;
                break;
            case 's':
                ++cx;
                break;
            case 'w':
                --cy;
                break;
            case 'z':
                ++cy;
                break;
            case ' ':
                // TODO
                // here the magic happens
                break;
        }
        if(cx < 0) cx = 0;
        if(cy < 0) cy = 0;
        if(cx >= game_width) cx = game_width - 1;
        if(cy >= game_height) cy = game_height - 1;
    }
}
