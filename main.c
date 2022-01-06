/*
 * Cut for a Country: a fighting game for the PS1
 * using PSn00bSDK.
 * TODO: put each player into a structure
 * so that I can use the same code on both structures,
 * rather than having repeated code all over the place.
 * Also, I need to add more attacks, more characters,
 * and implement combos and stop the players
 * from walking through each other.
 */

#include <sys/types.h>
#include <psxetc.h>
//#include <psxgte.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <psxapi.h>
#include <stdio.h>


#include "colour-names.h"
#define PLAYER_VERTS_COUNT 255 // so that each vertex index fits in a byte
                               // (lowers eyebrows in doubt as I write
                               // that comment above.

#define PLAYER_MAX_HEALTH 20   // that's four punches
#define PLAYER_ONE_ID 0
#define PLAYER_TWO_ID 1

#ifdef MODE_IS_PAL
/*
 * Unfortunately, PSn00bSDK has some bugs that affect
 *  vertical resolution, sometimes even enabling interlacing
 *  depending on the horizontal resolution.
 * For now, I just pretend it works.
 */
#define SCREEN_WIDTH   512
#define SCREEN_HEIGHT  256 // PAL supports slightly higher vertical resoutions
#define FRAME_MULTIPLIER 6 // multiply speeds/accelerations by the other framerate's MCD
#define FIFTH_OF_A_SECOND 5 // frames
#define THREE_FRAMES 3
const int Video_Mode = MODE_PAL;
#else
#define SCREEN_WIDTH  512
#define SCREEN_HEIGHT 240
#define FRAME_MULTIPLIER 5
#define FIFTH_OF_A_SECOND 6
#define THREE_FRAMES 3
const int Video_Mode = MODE_NTSC;
#endif

#define OTLENGTH 32768 // Size of the ordering table

#define RANDOM_STATE_SIZE 7
int pseudoRandomState[RANDOM_STATE_SIZE]={3141592, 65358979, 32384626, 43383279, 50288419, 71693993, 75105820};
uint16_t pseudoRandomStep=0;

// functions
void initDisplay();
void swapBuffers();
void loadFont();
void getGameInput(uint16_t button_input, uint16_t player_id);
void resetGameState();
uint16_t debounceInputs(uint16_t button_input); // FIXME: move this into the players
void getResultScreenInput(uint16_t button_input);
void drawPlayer(int playerNum);
void drawHealth();
void handleHits();
void setPlayer1Frame(int f);
void drawText(unsigned char *text, int textY);
int main();

enum Game_Modes { // remember to adjust the count
    LANGUAGE_MENU,
    MAIN_MENU,
    CHARACTER_MENU,
    SHOW_RESULT,
    MENU_LEAVE_LANGUAGES,
    MENU_LEAVE_MAIN,
    FIGHT_MODE,
} game_mode=FIGHT_MODE;
# define AMOUNT_OF_GAME_MODES 9
int exit_from_mode[AMOUNT_OF_GAME_MODES];
int amount_of_players=2;

// graphics
DISPENV dispenvs[2];
DRAWENV drawenvs[2];
int currentBuffer = 0;

uint16_t button_filter;

u_long orderingTables[2][OTLENGTH];
char primBuff[2][OTLENGTH];
char *nextPrim;

// variables that each player needs
// FIXME: animations are player specific in a different way,
//        but that's something I gotta fix in another file first.

typedef struct ai{
    int16_t dodge;
    int16_t dodge_wait;
    int16_t attack_wait;
} AI;


typedef struct player_mesh{
    uint8_t *verts_x;
    uint8_t *verts_y;
    uint8_t *colrefs;
    uint8_t *trirefs;
    int vert_count;
    int tri_count;
} Player_Mesh;


typedef struct player{
    int raw_x;
    int raw_y;
    int pixel_x;
    int pixel_y;
    int speed_x;
    int speed_y;
    int16_t hit;
    int16_t health;
    int frame;
    int16_t dash_state;
    int16_t punch_state;
    AI ai;
    Player_Mesh mesh;
} Player;


//uint8_t *player_1_colrefs=NULL;

Player player_1, player_2;


int player_scale_x=7;
int player_scale_y=5;


int current_screen_time;

// int player_1_block.state=0; // Just checking animation frame for now

// for the controllers
u_char controller_buffers[2][34];

extern u_long font_texture_tim_data[];
TIM_IMAGE font_texture;

//                            0    5   10   15   20   25   30   35   40   45   50   55   60   65   70   75   80
//                            0                 19                37            51    57               74
unsigned char all_texts_da[]="$SPILLER 1 VANDT$  SPILLER 2 VANDT$  DET BLEV LIGE$DANSK$ENGLISH$ESPAN~OL$1 SPILLER$2 SPILLERE$ SPROG$  $DEMONSTRATION";
unsigned char all_texts_en[]="$PLAYER 1  WINS$   PLAYER 2 WINS$    PLAYERS TIED$ DANSK$ENGLISH$ESPAN~OL$1 PLAYER$ 2 PLAYERS$  LANGUAGE$DEMO";
unsigned char all_texts_es[]="$GANA EL JUGADOR 1$GANA EL JUGADOR 2$EMPATE$       DANSK$ENGLISH$ESPAN~OL$1 JUGADOR$2 JUGADORES$IDIOMA$ $DEMONSTRACIO[N";

#define EMPTY_STRING 0
#define RESULT_ONE 1
#define RESULT_TWO 19
#define RESULT_TIE 37
#define LANGUAGE_DA 51
#define LANGUAGE_EN 57
#define LANGUAGE_ES 65
#define ONE_PLAYER 74
#define TWO_PLAYERS 84
#define LANGUAGE_OPTIONS 96
#define DEMO_OPTION 105

unsigned char *language_text=all_texts_en;

unsigned char *language_pointers[]={all_texts_da, all_texts_en, all_texts_es};
int current_language=1; // English

int show_text=EMPTY_STRING;
#define MENU_MAX_OPTIONS 4
int menu_full[MENU_MAX_OPTIONS];
int current_menu_options = 0;
int current_menu_selection=0;


// #include "player_1_xyc.h"
#include "player_1_animations.h"

char *juul_frames_x[]={player1verts_x_stille_0,
                       player1verts_x_slag_0, player1verts_x_slag_1,player1verts_x_slag_2, player1verts_x_slag_3, player1verts_x_slag_3,
                       player1verts_x_block_0,
                       player1verts_x_nede_0
                       };//////////// WIP


int8_t select_was_pressed = 0;
int8_t start_was_pressed = 0;
int8_t down_was_pressed = 0;
int8_t up_was_pressed = 0;
int8_t left_was_pressed = 0;
int8_t right_was_pressed = 0;
int8_t cross_was_pressed = 0;
int8_t circle_was_pressed = 0;
int8_t square_was_pressed = 0;
int8_t triangle_was_pressed = 0;

// Positions of different letters on the spritesheet

                    //    A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z
signed char textCols[]={  5,  3,  1,  6,  7,  8,  9,  2,  2,  0,  5,  6,  4, 10,  5,  4,  6,  7,  8,  9,  7,  8,  0,  9, 10, 13, -1};
signed char textRows[]={ 12, 15, 16, 12, 12, 12, 12, 16, 44, 18, 23, 23,  0, 12,  0, 24,  0, 23, 23, 23,  0,  0,  0,  0,  0, 4, -1};
signed char textSizs[]={ 11, 10, 10, 11, 11, 11, 11, 10,  4, 10, 10, 10, 13, 11,  12, 9, 12, 10, 10, 10, 12, 12, 18, 12, 12, 11, -1};

//                       0   1   2   3   4   5   6   7   8   9
signed char numSizs[]={ 11,  6, 10,  9,  9, 10, 10,  7, 10,  9};
signed char numRows[]={  0, 11, 17, 27, 36, 45, 55, 65, 72, 82};

void initModes(){
    exit_from_mode[LANGUAGE_MENU]=MENU_LEAVE_LANGUAGES;
    exit_from_mode[MAIN_MENU] = MENU_LEAVE_MAIN;
    exit_from_mode[CHARACTER_MENU]=FIGHT_MODE;
    exit_from_mode[FIGHT_MODE]=SHOW_RESULT;
    exit_from_mode[SHOW_RESULT]=FIGHT_MODE;
    exit_from_mode[MENU_LEAVE_LANGUAGES]=FIGHT_MODE;
    exit_from_mode[MENU_LEAVE_MAIN]=FIGHT_MODE;
}

void initMenu(int selection){
    current_menu_selection = selection;
    current_menu_options=0;
    for (int i=0;i<MENU_MAX_OPTIONS;i++){
        menu_full[i] = EMPTY_STRING;
    }
    current_screen_time++;
}

int addMenuOption(int string_id){
   if (current_menu_options>=MENU_MAX_OPTIONS){
       return 1;
   }
   menu_full[current_menu_options] = string_id;
   current_menu_options++;
   return 0;
}

void initLanguageMenu(){
    initMenu(1);
    addMenuOption (LANGUAGE_DA);
    addMenuOption (LANGUAGE_EN);
    addMenuOption (LANGUAGE_ES);
    game_mode = LANGUAGE_MENU;
}

void initMainMenu(){
    initMenu(amount_of_players-1);
    addMenuOption (ONE_PLAYER);
    addMenuOption (TWO_PLAYERS);
    addMenuOption (LANGUAGE_OPTIONS);
    addMenuOption (DEMO_OPTION);
    game_mode = MAIN_MENU;
    if (amount_of_players==0){
        current_menu_selection = 3;
    }
}

void setPlayerFrame(int f, int player_id){
    Player *player = NULL;
    if (player_id == PLAYER_ONE_ID){
        player = &player_1;
    }
    else{
        player = &player_2;
    }
    
    if (f<=7){
        //////////// WIP
        player->mesh.verts_x=juul_frames_x[f];
    }
    
    if (f==0){
        //player->mesh.verts_x = player1verts_x_stille_0;
        player->mesh.verts_y = player1verts_y_stille_0;
        player->mesh.trirefs = player_1_trirefs_stille_0;
        player->mesh.colrefs = player_1_colrefs_stille_0;
        
        player->mesh.vert_count = player_1_vert_count_stille_0;
        player->mesh.tri_count = player_1_tri_count_stille_0;
        return;
    }
    if (f==1){
        //player->mesh.verts_x = player1verts_x_slag_0;
        player->mesh.verts_y = player1verts_y_slag_0;
        player->mesh.trirefs = player_1_trirefs_slag_0;
        player->mesh.colrefs = player_1_colrefs_slag_0;
        
        player->mesh.vert_count = player_1_vert_count_slag_0;
        player->mesh.vert_count = player_1_tri_count_slag_0;
        return;
    }
    if (f==2){
        //player->mesh.verts_x = player1verts_x_slag_1;
        player->mesh.verts_y = player1verts_y_slag_1;
        player->mesh.trirefs = player_1_trirefs_slag_1;
        player->mesh.colrefs = player_1_colrefs_slag_1;
        
        player->mesh.vert_count = player_1_vert_count_slag_1;
        player->mesh.vert_count = player_1_tri_count_slag_1;
        return;
    }
    if (f==3){
        //player->mesh.verts_x = player1verts_x_slag_2;
        player->mesh.verts_y = player1verts_y_slag_2;
        player->mesh.trirefs = player_1_trirefs_slag_2;
        player->mesh.colrefs = player_1_colrefs_slag_2;
        
        player->mesh.vert_count = player_1_vert_count_slag_2;
        player->mesh.vert_count = player_1_tri_count_slag_2;
        return;
    }
    if (f==4 || f==5){
        //player->mesh.verts_x = player1verts_x_slag_3;
        player->mesh.verts_y = player1verts_y_slag_3;
        player->mesh.trirefs = player_1_trirefs_slag_3;
        player->mesh.colrefs = player_1_colrefs_slag_3;
        
        player->mesh.vert_count = player_1_vert_count_slag_3;
        player->mesh.vert_count = player_1_tri_count_slag_3;
        return;
    }
    if (f==6){
        //player->mesh.verts_x = player1verts_x_block_0;
        player->mesh.verts_y = player1verts_y_block_0;
        player->mesh.trirefs = player_1_trirefs_block_0;
        player->mesh.colrefs = player_1_colrefs_block_0;
        
        player->mesh.vert_count = player_1_vert_count_block_0;
        player->mesh.vert_count = player_1_tri_count_block_0;
        return;
    }
    if (f==7){
        //player->mesh.verts_x = player1verts_x_nede_0;
        player->mesh.verts_y = player1verts_y_nede_0;
        player->mesh.trirefs = player_1_trirefs_nede_0;
        player->mesh.colrefs = player_1_colrefs_nede_0;
        
        player->mesh.vert_count = player_1_vert_count_nede_0;
        player->mesh.vert_count = player_1_tri_count_nede_0;
        return;
    }
    if ((f>=20)&&(f<20+FIFTH_OF_A_SECOND)){
        player->mesh.verts_x = player1verts_x_stille_0;
        player->mesh.verts_y = player1verts_y_stille_0;
        player->mesh.trirefs = player_1_trirefs_stille_0;
        player->mesh.colrefs = player_1_colrefs_stille_0;
        
        player->mesh.vert_count = player_1_vert_count_stille_0;
        player->mesh.vert_count = player_1_tri_count_stille_0;
        return;
    }
}



void initDisplay(){   
    ResetGraph(0);

    SetDefDispEnv (&dispenvs[0], 0,   0, SCREEN_WIDTH, SCREEN_HEIGHT); //top
    SetDefDispEnv (&dispenvs[1], 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT); //bottom
    
    if (Video_Mode==MODE_PAL){
        dispenvs[0].screen.x = 0;
        dispenvs[0].screen.y = 24;
        dispenvs[1].screen.y = dispenvs[0].screen.y;
        dispenvs[1].screen.x = dispenvs[0].screen.x;
    }
    
    SetDefDrawEnv (&drawenvs[0], 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT); //bottom
    SetDefDrawEnv (&drawenvs[1], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); //top
    
    //SetVideoMode (Video_Mode);
    
    setRGB0(&drawenvs[0], BACK_RED, BACK_GREEN, BACK_BLUE);  // Set clear color (dark purple)
    setRGB0(&drawenvs[1], BACK_RED, BACK_GREEN, BACK_BLUE);
    drawenvs[0].isbg = 1; // auto-clearing, maybe not necessary if I use a background sprite,
                          //  but for now I'm not drawing a background, so I need to enable clearing.
    drawenvs[1].isbg = 1;

    nextPrim = primBuff[0];
    
    loadFont();
}

void swapBuffers() {
    DrawSync(0);
    VSync(0);

    PutDispEnv(&dispenvs[currentBuffer]);
    PutDrawEnv(&drawenvs[currentBuffer]);

    SetDispMask(1); // Enable the display, does this have to happen on every frame?

    DrawOTag(orderingTables[currentBuffer]+OTLENGTH-1);
    currentBuffer = !currentBuffer;
    nextPrim = primBuff[currentBuffer];
    
}


void loadFont() {    
    GetTimInfo ((u_long *)font_texture_tim_data, &font_texture);
    LoadImage (font_texture.prect, font_texture.paddr);
    printf("Font: %d, %d\n", font_texture.prect->x, font_texture.prect->y);
    if (font_texture.mode & 0x8){
        // the texture has has a CLUT
        LoadImage (font_texture.crect, font_texture.caddr);
        printf("CLUT: %d, %d\n", font_texture.crect->x, font_texture.crect->y);
    }
    drawenvs[0].tpage = getTPage( font_texture.mode&0x3, 0, font_texture.prect->x, font_texture.prect->y );
    drawenvs[1].tpage = getTPage( font_texture.mode&0x3, 0, font_texture.prect->x, font_texture.prect->y );
    PutDrawEnv (&drawenvs[!currentBuffer]);

}

void initControllers(){
    InitPAD (controller_buffers[0], 34, controller_buffers[1], 34);
    controller_buffers[0][0] = 0xff;
    controller_buffers[0][1] = 0xff;
    controller_buffers[1][0] = 0xff;
    controller_buffers[1][1] = 0xff;
    StartPAD();
    ChangeClearPAD(1); // should supress a BIOS warning,
                       //  but might not work with all BIOSes
}

void drawPlayer(int playerNum){
    // TODO: This would be so much easier if each player was a struct
    Player * player = NULL; // FIXME: Maybe move that next conditional into a function
    if (playerNum == PLAYER_ONE_ID){
        player = &player_1;
    }
    else{
        player = &player_2;
    }
    POLY_G3 *triangle=NULL;
    //int16_t player->pixel_x=player_1.pixel_x;
    // int16_t player->pixel_y=player_1.pixel_y;
    uint8_t *player_red=player_1_red;
    uint8_t *player_green=player_1_green;
    uint8_t *player_blue=player_1_blue;
    int scale_x = player_scale_x;
    int scale_y = player_scale_y;
    int side_scale_x=1;
    int side_offset=0;
    
    int player_frame=player_1.frame;
    
    if (playerNum==1){
        //player->pixel_x=player_2.pixel_x;
        // player->pixel_y=(176-64)*256*player_scale_y/256/16;
        player_red=player_2_red;
        player_green=player_2_green;
        player_blue=player_2_blue;
        side_scale_x=-1;
        side_offset=256;
        
        player_frame=player_2.frame;
    }
    if(player_frame>=20){
        player_red=player_red_red;
        player_green=player_red_green;
        player_blue=player_red_blue;
    }
    
    if (player_1.pixel_x>player_2.pixel_x){
        side_scale_x=0-side_scale_x;
        side_offset=256-side_offset;
    }
    
    for (int tri_place=0; tri_place<player->mesh.vert_count*3;tri_place+=3){
            int tri_v1 = player->mesh.trirefs[tri_place];
            int tri_v2 = player->mesh.trirefs[tri_place+1];
            int tri_v3 = player->mesh.trirefs[tri_place+2];
            int tri_index=tri_v1;
            
            triangle = (POLY_G3*)nextPrim;
            setPolyG3 (triangle);
            setXY3 (triangle, player->pixel_x+(((int16_t)player->mesh.verts_x[tri_v1]*side_scale_x+side_offset)*scale_x)/16,
                              player->pixel_y+(((int16_t)player->mesh.verts_y[tri_v1])*scale_y)/16,
                              
                              player->pixel_x+(((int16_t)player->mesh.verts_x[tri_v2]*side_scale_x+side_offset)*scale_x)/16,
                              player->pixel_y+(((int16_t)player->mesh.verts_y[tri_v2])*scale_y)/16,
                              
                              player->pixel_x+(((int16_t)player->mesh.verts_x[tri_v3]*side_scale_x+side_offset)*scale_x)/16,
                              player->pixel_y+(((int16_t)player->mesh.verts_y[tri_v3])*scale_y)/16);
            char basecol= player->mesh.colrefs[tri_index];
            char tri_c1 = player_red[basecol];
            char tri_c2 = player_green[basecol];
            char tri_c3 = player_blue[basecol];
            setRGB0(triangle, tri_c1, tri_c2, tri_c3);
            tri_index=tri_v2;
            basecol= player->mesh.colrefs[tri_index];
            tri_c1 = player_red[basecol];
            tri_c2 = player_green[basecol];
            tri_c3 = player_blue[basecol];
            setRGB1(triangle, tri_c1, tri_c2, tri_c3);
            tri_index=tri_v3;
            basecol= player->mesh.colrefs[tri_index];
            tri_c1 = player_red[basecol];
            tri_c2 = player_green[basecol];
            tri_c3 = player_blue[basecol];
            setRGB2(triangle, tri_c1, tri_c2, tri_c3);
            
             addPrim(orderingTables[currentBuffer], triangle);
            
            nextPrim += sizeof(POLY_G3);
        }
}

void resetGameState(){
    Player *player = &player_1;
    for (int i=0;i<2;i++){
    // stuff shared by both players
    player->health = PLAYER_MAX_HEALTH;
    
    player->raw_y = 64*256;
    player->pixel_y=player->raw_y*player_scale_y/256/16;
    player->speed_x = 0;
    player->speed_y = 0;
    
    player->frame=0;
    
    player->ai.dodge = 0;
    player->ai.dodge_wait = 0;
    player->ai.attack_wait = 0;
    
    player = &player_2;
    }
    // Things that differ:
    player_1.raw_x = 64*256;
    player_2.raw_x = 20*16*256;
    player_1.pixel_x=player_1.raw_x*player_scale_x/128/16;
    player_2.pixel_x=player_2.raw_x*player_scale_x/128/16;
    
    // reset text:
    show_text=EMPTY_STRING;
}

uint16_t debounceInputs(uint16_t button_input){
    uint16_t new_filter=0;
    if (button_input & PAD_UP){
        up_was_pressed = THREE_FRAMES;
    }
    if (button_input & PAD_DOWN){
        down_was_pressed = THREE_FRAMES;
    }
    if (button_input & PAD_LEFT){
        left_was_pressed = THREE_FRAMES;
    }
    if (button_input & PAD_RIGHT){
        right_was_pressed = THREE_FRAMES;
    }
    
    if (button_input & PAD_TRIANGLE){
        triangle_was_pressed = THREE_FRAMES;
    }
    if (button_input & PAD_SQUARE){
        square_was_pressed = THREE_FRAMES;
    }
    if (button_input & PAD_CIRCLE){
        circle_was_pressed = THREE_FRAMES;
    }
    if (button_input & PAD_CROSS){
        cross_was_pressed = THREE_FRAMES;
    }
    if (button_input & PAD_START){
        start_was_pressed = THREE_FRAMES;
    }
    if (button_input & PAD_SELECT){
        select_was_pressed = THREE_FRAMES;
    }
    
    
    if (((button_input & PAD_UP)==0) && (up_was_pressed>0)){
        up_was_pressed--;
    }
    if (((button_input & PAD_DOWN)==0) && (down_was_pressed>0)){
        down_was_pressed--;
    }
    if (((button_input & PAD_LEFT)==0) && (left_was_pressed>0)){
        left_was_pressed--;
    }
    if (((button_input & PAD_RIGHT)==0) && (right_was_pressed>0)){
        right_was_pressed--;
    }
    
    if (((button_input & PAD_TRIANGLE)==0) && (triangle_was_pressed>0)){
        triangle_was_pressed--;
    }
    if (((button_input & PAD_SQUARE)==0) && (square_was_pressed>0)){
        square_was_pressed--;
    }
    if (((button_input & PAD_CIRCLE)==0) && (circle_was_pressed>0)){
        circle_was_pressed--;
    }
    if (((button_input & PAD_CROSS)==0) && (cross_was_pressed>0)){
        cross_was_pressed--;
    }
    
    if (((button_input & PAD_START)==0) && (start_was_pressed>0)){
        start_was_pressed--;
    }
    if (((button_input & PAD_SELECT)==0) && (select_was_pressed>0)){
        select_was_pressed--;
    }
    
    if (down_was_pressed){
       new_filter|=(PAD_DOWN);
    }
    if (left_was_pressed){
       new_filter|=(PAD_LEFT);
    }
    if (up_was_pressed){
       new_filter|=(PAD_UP);
    }
    if (right_was_pressed){
       new_filter|=(PAD_RIGHT);
    }
    if (cross_was_pressed){
       new_filter|=(PAD_CROSS);
    }
    if (circle_was_pressed){
       new_filter|=(PAD_CIRCLE);
    }
    if (triangle_was_pressed){
       new_filter|=(PAD_TRIANGLE);
    }
    if (square_was_pressed){
       new_filter|=(PAD_SQUARE);
    }
    if (start_was_pressed){
       new_filter|=(PAD_START);
    }
    if (select_was_pressed){
       new_filter|=(PAD_SELECT);
    }
    // printf("new_filter: %x, %d\n", new_filter, new_filter);
    new_filter ^= (PAD_DOWN|PAD_LEFT|PAD_UP|PAD_RIGHT|PAD_CROSS|PAD_CIRCLE|PAD_TRIANGLE|PAD_SQUARE|PAD_START|PAD_SELECT);
    // printf("new_filter: %x, %d\n", new_filter, new_filter);
    return new_filter;
}

void getResultScreenInput(uint16_t button_input){
    if (button_input & PAD_START){
        resetGameState();
        game_mode = FIGHT_MODE;
    }
    if ((button_input & PAD_CROSS) && (cross_was_pressed <= 0)){
        resetGameState();
        game_mode = FIGHT_MODE;
    }
    if ((button_input & PAD_TRIANGLE) && (triangle_was_pressed <= 0)){
        resetGameState();
        initMainMenu();
    }
}

void getMenuInput(uint16_t button_input){
    if ((button_input & PAD_CROSS) && (cross_was_pressed <= 0)){
        game_mode = exit_from_mode[game_mode];
    }
    if ((button_input & PAD_DOWN) && (down_was_pressed <= 0)){
        current_menu_selection++;
        if (current_menu_selection>=current_menu_options){
            current_menu_selection=0;
        }
        //printf("menu selection: %d\n", current_menu_selection);
    }
    if ((button_input & PAD_UP) && (up_was_pressed <= 0)){
        current_menu_selection--;
        if (current_menu_selection<0){
            current_menu_selection=current_menu_options-1;
        }
        //printf("menu selection: %d\n", current_menu_selection);
    }
}

void getAttractInput(uint16_t button_input){
    if (button_input){
        initMainMenu();
    }
}

uint16_t randomNumber(){ // Not perfectly random, but might be good enough for single player
    int step1=pseudoRandomState[(pseudoRandomStep+1)%RANDOM_STATE_SIZE];
    int step2=pseudoRandomState[(pseudoRandomStep+2)%RANDOM_STATE_SIZE];
    int r=step1+step2;
    pseudoRandomState[pseudoRandomStep]=r;
    pseudoRandomStep++;
    pseudoRandomStep %=RANDOM_STATE_SIZE;
    return (r);
}

void simulateInput(uint16_t player_id){
    uint16_t button_input=0;
    int should_go_to_the_right = 0;
    Player *this_player=NULL;
    Player *other_player=NULL;
    if (player_id == PLAYER_ONE_ID){
        this_player = &player_1;
        other_player= &player_2;
    }
    else{
        this_player = &player_2;
        other_player= &player_1;
    }
    
    if (this_player->raw_x<other_player->raw_x-6000){
        should_go_to_the_right = 1;
    }
    else if (this_player->raw_x>other_player->raw_x+6000){
        should_go_to_the_right = -1;
    }
    
    if (should_go_to_the_right>0){
        button_input |= PAD_RIGHT;
    }
    else if (should_go_to_the_right<0){
        button_input |= PAD_LEFT;
    }
    int might_dodge = 0;
    if (other_player->frame==0){
        this_player->ai.dodge = 0;
    }
    
    if ( (this_player->ai.dodge_wait==0) &&
         (other_player->frame==2) &&
         (this_player->ai.dodge==0) ){
        this_player->ai.dodge = randomNumber()&1;
        //printf("Could try to dodge\n");
        if (this_player->ai.dodge==0){
            this_player->ai.dodge_wait=FIFTH_OF_A_SECOND;
        }
    }
    else if (this_player->ai.attack_wait){
        this_player->ai.attack_wait--;
    }
    
    if ((should_go_to_the_right==0) && ((randomNumber()&1)==1) && (this_player->ai.attack_wait==0)){
        button_input |= PAD_CROSS;
    }
    else if (this_player->ai.attack_wait){
        this_player->ai.attack_wait--;
    }
    else{
        this_player->ai.attack_wait = FIFTH_OF_A_SECOND;
    }
    
    if(this_player->ai.dodge){
        button_input |= PAD_TRIANGLE;
    }
    else if (this_player->ai.dodge_wait){
        this_player->ai.dodge_wait--;
    }
    
    getGameInput (button_input, player_id);
}

void getGameInput(uint16_t button_input, uint16_t player_id){
        Player *player=NULL;
        if (player_id==0){
            player = &player_1;
        }
        else{
            player = &player_2;
        }
        signed int move_x = 0;
        signed int move_y = 0;
        
        if ((player->frame>20) && (player->frame<20+FIFTH_OF_A_SECOND)){
            player->frame++;
        }
        
        if ((player->frame>20) && (player->frame<20+FIFTH_OF_A_SECOND)){
            player->frame++;
            return;
        }
        if (player->frame==6){ // FIXME: Don't hardcode character-specific numbers in input handling
            player->frame=0;
        }
        
        if (button_input & PAD_LEFT){
            move_x--;
        }
        if (button_input & PAD_RIGHT){
            move_x++;
        }
        if (button_input & PAD_UP){
            move_y--;
        }
        if (button_input & PAD_DOWN){
            move_y++;
        }
        if ((button_input & PAD_TRIANGLE)&&(player->frame==0)){
            player->frame=6;
            return;
        }
        else if ((button_input & PAD_CIRCLE)&&(player->dash_state==0)){
            player->dash_state=FIFTH_OF_A_SECOND*5;// a whole second
            if (move_x<0){
                player->dash_state=0-player->dash_state;
                player->speed_x=(player->dash_state*8+400)*FRAME_MULTIPLIER;
            }
            //printf("dash state: %d\n", player->dash_state);
        }
        else if ((button_input & PAD_CROSS)&&(player->punch_state==0)){
            player->punch_state=FIFTH_OF_A_SECOND*6;//half a second
            if (move_x<0){
                player->punch_state=0-player->punch_state;
            }
            //printf("punch state: %d\n", player->punch_state);
        }
        else if (player->dash_state>0){
            player->dash_state--;
            player->speed_x=player->dash_state*16*FRAME_MULTIPLIER;
        }
        else if (player->dash_state<0){
            player->dash_state++;
            player->speed_x=player->dash_state*16*FRAME_MULTIPLIER;
        }
        /*
        if ((button_input & PAD_L1)&&(player_scale_x>=2)){
            player->scale_x--;
            player->scale_y--;
        }
        if ((button_input & PAD_R1)&&(player_scale_x<=15)){
            player_scale_x++;
            player_scale_y++;
        }
        */
        int old_speed_x = player->speed_x;
        if (player->punch_state){
            signed int punch=player->punch_state;
            if (punch<0){
                punch=0-punch;
            }
            punch=(FIFTH_OF_A_SECOND*5)-punch;
            int punch_frame=punch/FIFTH_OF_A_SECOND;
            if (punch_frame>5){
                punch_frame=5;
            }
            player->frame = 1+punch_frame;
            //printf("punch: %d: %d, %d\n", player->punch_state, punch, punch_frame, player->frame);
        }
        else{
            player->frame=0;
        }
        if (player->punch_state>0){
            player->punch_state--;
        }
        else if (player->punch_state<0){ // TODO: Don't use negative states for this
            player->punch_state++;
        }
        if (player->dash_state==0){
            if(move_x>0){
                if (player->speed_x>(64*FRAME_MULTIPLIER-5*FRAME_MULTIPLIER)){
                    player->speed_x=64*FRAME_MULTIPLIER;
                }
                else{
                    player->speed_x+=6*FRAME_MULTIPLIER;
                }
            }
            else if(move_x<0){
                if (player->speed_x<(-64*FRAME_MULTIPLIER+8*FRAME_MULTIPLIER)){
                    player->speed_x=0-64*FRAME_MULTIPLIER;
                }
                else{
                    player->speed_x-=6*FRAME_MULTIPLIER;
                }
            }
            else{
                if (player->speed_x>12*FRAME_MULTIPLIER){
                    player->speed_x-=12*FRAME_MULTIPLIER;
                }
                else if (player->speed_x<-12*FRAME_MULTIPLIER){
                    player->speed_x+=12*FRAME_MULTIPLIER;
                }
                else{
                    player->speed_x=0;
                }
            }
            // = ( (move_x*64*FRAME_MULTIPLIER) + 7*old_speed_x)/8;
        }
        
        player->speed_y+=FRAME_MULTIPLIER*3;
        player->raw_y+=player->speed_y;
        if (player->raw_y>=(176-64)*256){
            player->raw_y=(176-64)*256;
            player->speed_y=0;
            if (move_y<0){
                player->speed_y=(0-150)*FRAME_MULTIPLIER;
            }
        }
        player->raw_x +=  player->speed_x;
        player->pixel_x=player->raw_x*player_scale_x/128/16;
        player->pixel_y=player->raw_y*player_scale_y/256/16;
}

void drawHealth(){
    TILE *tile;
    // player 1
    if(player_1.health>0){
        tile = (TILE*)nextPrim;
        setTile(tile);
        setXY0(tile, 10, 200);
        setWH(tile, player_1.health*(240/PLAYER_MAX_HEALTH), 20);
        setRGB0(tile, player_1_red[10], player_1_green[10], player_1_blue[10]);
        addPrim(orderingTables[currentBuffer], tile);
        nextPrim += sizeof(TILE);
    }
        
    // player 2
    if(player_2.health>0){
        tile = (TILE*)nextPrim;
        setTile(tile);
        setXY0(tile, 10+256, 200);
        setWH(tile, player_2.health*(240/PLAYER_MAX_HEALTH), 20);
        setRGB0(tile, player_2_red[10], player_2_green[10], player_2_blue[10]);
        addPrim(orderingTables[currentBuffer], tile);
        nextPrim += sizeof(TILE);
    }
}

void handleHits(){
    // let players get hit
    // FIXME: this code really shows why each player should have a struct
    player_1.hit--;
    player_2.hit--;
    
    if ((player_1.frame==4) &&
         (
              (
                  (player_2.raw_x-player_1.raw_x < 81920-73684) &&
                  (player_2.raw_x>player_1.raw_x)
              ) ||
              (
                  (player_1.raw_x-player_2.raw_x < 81920-73684) &&
                  (player_2.raw_x<player_1.raw_x)
              )
         ) &&
         ( player_2.frame<20 )
         &&
         ( player_2.frame!=6 )
         &&
         (player_1.hit<=0)
       ){
    
        player_2.frame=20;
        player_1.hit=FIFTH_OF_A_SECOND;
        if (player_2.health>5){
            player_2.health-=5;
        }
        else{
            player_2.health=0;
            player_2.frame=7;
        }
        //printf("Health: %d, %d\n", player_1.health, player_2.health);
        
    }
    
    // start of repeated code
    if ((player_2.frame==4) &&
         (
              (
                  (player_1.raw_x-player_2.raw_x < 81920-73684) &&
                  (player_1.raw_x>player_2.raw_x)
              ) ||
              (
                  (player_2.raw_x-player_1.raw_x < 81920-73684) &&
                  (player_1.raw_x<player_2.raw_x)
              )
         ) &&
         ( player_1.frame<20 )
         &&
         ( player_1.frame!=6 )
         &&
         (player_2.hit<=0)
       ){
    
        player_1.frame=20;
        player_2.hit=FIFTH_OF_A_SECOND;
        if (player_1.health>5){
            player_1.health-=5;
        }
        else{
            player_1.health=0;
            player_1.frame=7;
        }
        
    } // end of repeated code
    
    // FIXME: I'm setting a variable and then checking it? Maybe that's correct
    if (player_1.health <=0 && player_2.health<=0){
        show_text=RESULT_TIE;
        game_mode = SHOW_RESULT;
        current_screen_time=0;
    }
    else if (player_1.health<=0){
        show_text=RESULT_TWO;
        game_mode = SHOW_RESULT;
        current_screen_time=0;
    }
    else if (player_2.health<=0){
        show_text=RESULT_ONE;
        game_mode = SHOW_RESULT;
        current_screen_time=0;
    }
}

void drawText(unsigned char *text, int textY){
    int nextPlace=0;
    SPRT *sprite;
    char prev_width=0;
    for (int textPlace=0; (text[textPlace]!=0)&&(text[textPlace]!=36);textPlace++){
        unsigned char oldc = text[textPlace];
        if (oldc!=' '){
            if ((oldc>='A') && (oldc<='Z')){
                unsigned int newc=oldc-65;
                sprite = (SPRT*)nextPrim;
                setSprt(sprite);
                setXY0 (sprite, 50+nextPlace, textY);
                setWH  (sprite, textSizs[newc]*2, 18);
                setUV0 (sprite, (textRows[newc])*2, 18*textCols[newc]);
                setClut(sprite, font_texture.crect->x, font_texture.crect->y);
                setRGB0(sprite, 128, 128, 128); // what does this colour do and why do I need it?
                addPrim(orderingTables[currentBuffer], sprite);
              
                nextPlace+=2*(textSizs[newc]+2);
                nextPrim += sizeof(SPRT);
                prev_width = textSizs[newc]; // Intentionally just half the width!
                //printf("character: %d\n", newc);
            }
            if ((oldc>='0') && (oldc<='9')){
                unsigned int newc=oldc-'0';
                sprite = (SPRT*)nextPrim;
                setSprt(sprite);
                setXY0 (sprite, 50+nextPlace, textY);
                setWH  (sprite, numSizs[newc]*2, 18);
                setUV0 (sprite, (numRows[newc])*2, 18*12);
                setClut(sprite, font_texture.crect->x, font_texture.crect->y);
                setRGB0(sprite, 128, 128, 128);
                addPrim(orderingTables[currentBuffer], sprite);
                nextPlace+=2*(numSizs[newc]+2);
                nextPrim += sizeof(SPRT);
                prev_width = textSizs[newc]; // Intentionally just half the width!
                //printf("character: %d\n", newc);
            }
            else if (oldc=='~'){
                //unsigned int newc=oldc-65;
                //puts("tilde");
                sprite = (SPRT*)nextPrim;
                setSprt(sprite);
                //setXY0 (sprite, 50+nextPlace, textY-12); // textY-12: keep this glyph close to the one below
                setXY0 (sprite, 50+nextPlace-prev_width-2-11, textY-12); // subtract half the width of the previous character, and half the width of this one, and the gap between characters
                setWH  (sprite, 10*2, 18);
                setUV0 (sprite, 0*2, 18*11);
                setClut(sprite, font_texture.crect->x, font_texture.crect->y);
                setRGB0(sprite, 128, 128, 128); // what does this colour do and why do I need it?
                addPrim(orderingTables[currentBuffer], sprite);
              
                //nextPlace+=2*(11+2);
                nextPrim += sizeof(SPRT);
                //printf("character at %d,%d size %d,%d\n", 50+nextPlace, textY-12, 11*2, 18);
            }
            else if (oldc=='['){
                //unsigned int newc=oldc-65;
                //puts("tilde");
                sprite = (SPRT*)nextPrim;
                setSprt(sprite);
                setXY0 (sprite, 50+nextPlace-prev_width-2-4, textY-10); // subtract half the width of the previous character, and half the width of this one, and the gap between characters
                setWH  (sprite, 4*2, 18);
                setUV0 (sprite, 13*2, 18*11);
                setClut(sprite, font_texture.crect->x, font_texture.crect->y);
                setRGB0(sprite, 128, 128, 128); // what does this colour do and why do I need it?
                addPrim(orderingTables[currentBuffer], sprite);
              
                //nextPlace+=2*(11+2);
                nextPrim += sizeof(SPRT);
                //printf("character at %d,%d size %d,%d\n", 50+nextPlace, textY-12, 11*2, 18);
            }
        }
        else{
            nextPlace+=18;
        }
    }
}

void drawMenu(){
    TILE *tile=NULL;
    int vPos=25;
    for (int item=0;item<current_menu_options;item++){
        drawText (&(language_text[menu_full[item]]), vPos+25);
        vPos+=35;
    }
    if(current_menu_selection<current_menu_options){
        tile = (TILE*)nextPrim;
        setTile(tile);
        setXY0(tile, 15, 45 + current_menu_selection*35);
        setWH(tile, 20, 30);
        setRGB0(tile, 255, 255, 128);
        addPrim(orderingTables[currentBuffer], tile);
        nextPrim += sizeof(TILE);
    }
        
}

int main() {
    initDisplay();
    initControllers();
    initModes();

    int controller_type = 0;
    uint16_t button_input_1 = 0;
    uint16_t button_input_2 = 0;
    initLanguageMenu();
    resetGameState();
    while(1) {
        ClearOTagR(orderingTables[currentBuffer], OTLENGTH);
        language_text=language_pointers[current_language];
        button_input_1 = 0;
        button_input_2 = 0;
        if (controller_buffers[0][0] == 0){
            int controller_data_size = controller_buffers[0][1]&0x0f;
            button_input_1 = 0xffff^(*((uint16_t*)(controller_buffers[0]+2)));
        }
        
        if (controller_buffers[1][0] == 0){
            int controller_data_size = controller_buffers[1][1]&0x0f;
            button_input_2 = 0xffff^(*((uint16_t*)(controller_buffers[1]+2)));
        }
        
        
        if ((game_mode==LANGUAGE_MENU) || (game_mode==MAIN_MENU)){
            getMenuInput(button_input_1|button_input_2);
            drawMenu();
        }
        else if (game_mode==SHOW_RESULT){
            if(show_text != EMPTY_STRING){
                drawText (&(language_text[show_text]), 25);
            }
            if (amount_of_players){
                getResultScreenInput(button_input_1|button_input_2);
            }
            else{
                getAttractInput((button_input_1|button_input_2)&button_filter);
            }
            drawHealth();
        }
        else if (game_mode==FIGHT_MODE){
            if (amount_of_players==0){
                getAttractInput((button_input_1|button_input_2)&button_filter);
            }
            if (amount_of_players>=1){
                getGameInput(button_input_1, PLAYER_ONE_ID);
            }
            else{
                simulateInput(PLAYER_ONE_ID);
            }
            if (amount_of_players==2){
                getGameInput(button_input_2, PLAYER_TWO_ID);
            }
            else{
                simulateInput(PLAYER_TWO_ID);
            }
            handleHits();
            drawHealth();
        }
        if (game_mode==MENU_LEAVE_LANGUAGES){
            current_language=current_menu_selection;
            resetGameState();
            initMainMenu();
        }
        else if (game_mode==MENU_LEAVE_MAIN){
            if (current_menu_selection==2){
                initLanguageMenu();
            }
            else if (current_menu_selection == 3){
                resetGameState();
                amount_of_players = 0;
                printf("amount_of_players: %d\n", amount_of_players);
                game_mode=FIGHT_MODE;
            }
            else{
                resetGameState();
                amount_of_players = current_menu_selection+1;
                printf("amount_of_players: %d\n", amount_of_players);
                game_mode=FIGHT_MODE;
            }
        }
        button_filter=debounceInputs(button_input_1|button_input_2);
        
        setPlayerFrame(player_1.frame, PLAYER_ONE_ID);
        drawPlayer(0);
        setPlayerFrame(player_2.frame, PLAYER_TWO_ID);
        drawPlayer(1);
        
        swapBuffers();
        
        if ((game_mode==SHOW_RESULT) &&
            (amount_of_players==0) &&
            (current_screen_time>=FIFTH_OF_A_SECOND*5*3)
           ){
           resetGameState();
           game_mode=FIGHT_MODE;
        }
        current_screen_time++;
        
    }
    
    return 0;
}
