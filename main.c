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
const int Video_Mode = MODE_PAL;
#else
#define SCREEN_WIDTH  512
#define SCREEN_HEIGHT 240
#define FRAME_MULTIPLIER 5
#define FIFTH_OF_A_SECOND 6
const int Video_Mode = MODE_NTSC;
#endif

#define OTLENGTH 32768 // Size of the ordering table

// functions
void initDisplay();
void showDisplay();
void loadFont();
void getInput(uint16_t button_input, uint16_t player_id);
void getCommonInput(uint16_t button_input);
void drawPlayer(int playerNum);
void drawHealth();
void handleHits();
void drawText(unsigned char *text);
int main();

// graphics
DISPENV dispenvs[2];
DRAWENV drawenvs[2];
int currentBuffer = 0;

u_long orderingTables[2][OTLENGTH];
char primBuff[2][OTLENGTH];
char *nextPrim;

// variables that each player needs
// FIXME: animations are player specific in a different way,
//        but that's something I gotta fix in another file first.
int player_1_x = 32;
int player_1_y=32;
int player_2_x=0;
int player_2_y=0;

int player_1_raw_x = 64*256;
int player_1_raw_y = 64*256;
int player_1_speed_x = 0;
int player_1_speed_y = 0;

int player_2_raw_x = 20*16*256;
int player_2_raw_y = 64*256;
int player_2_speed_x = 0;
int player_2_speed_y = 0;

int player_1_frame=0;
int player_2_frame=0;

int player_scale_x=7;
int player_scale_y=5;

signed int player_1_dash_state=0;
signed int player_2_dash_state=0;
signed int player_1_punch_state=0;
signed int player_2_punch_state=0;
// int player_1_block_state=0; // Just checking animation frame for now

signed int player_1_health=PLAYER_MAX_HEALTH;
signed int player_2_health=PLAYER_MAX_HEALTH;

int player_1_hit=0;
int player_2_hit=0;

// for the controllers
u_char controller_buffers[2][34];

extern u_long font_texture_tim_data[];
TIM_IMAGE font_texture;


//                           0                    20                   40           55
unsigned char wintexts_da[]="SPILLER ET VANDT\0   SPILLER TO VANDT\0   DET BLEV LIGE\0";
unsigned char wintexts_en[]="PLAYER ONE WINS\0    PLAYER TWO WINS\0    PLAYERS TIED\0 ";
unsigned char wintexts_es[]="GANA EL JUGADOR UNO\0GANA EL JUGADOR dos\0EMPATE\0       ";

#define RESULT_ONE 0
#define RESULT_TWO 20
#define RESULT_TIE 40
#define RESULT_EMPTY 55

unsigned char *language_text=wintexts_en;

unsigned char *language_pointers[]={wintexts_en, wintexts_da, wintexts_es};
int current_language=0; // English

unsigned char *show_text=wintexts_en+RESULT_EMPTY;

int select_was_pressed = 0;


// Positions of different letters on the spritesheet

                    //    A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z
signed char textCols[]={  5,  3,  1,  6,  7,  8,  9,  2,  2,  0,  5,  6,  4, 10,  5,  4,  6,  7,  8,  9,  7,  8,  0,  9, 10, -1};
signed char textRows[]={ 12, 15, 16, 12, 12, 12, 12, 16, 44, 18, 23, 23,  0, 12,  0, 24,  0, 23, 23, 23,  0,  0,  0,  0,  0, -1};
signed char textSizs[]={ 11, 10, 10, 11, 11, 11, 11, 10,  4, 10, 10, 10, 13, 11,  12, 9, 12, 10, 10, 10, 12, 12, 18, 12, 12, -1};

#include "player_1_xyc.h"
#include "player_1_animations.h"


void setPlayer1Frame(int f){
    
    if (f==0){
        player1verts_x = player1verts_x_stille_0;
        player1verts_y = player1verts_y_stille_0;
        player_1_trirefs = player_1_trirefs_stille_0;
        player_1_colrefs = player_1_colrefs_stille_0;
        
        player_1_vert_count = player_1_vert_count_stille_0;
        player_1_tri_count = player_1_tri_count_stille_0;
        return;
    }
    if (f==1){
        player1verts_x = player1verts_x_slag_0;
        player1verts_y = player1verts_y_slag_0;
        player_1_trirefs = player_1_trirefs_slag_0;
        player_1_colrefs = player_1_colrefs_slag_0;
        
        player_1_vert_count = player_1_vert_count_slag_0;
        player_1_tri_count = player_1_tri_count_slag_0;
        return;
    }
    if (f==2){
        player1verts_x = player1verts_x_slag_1;
        player1verts_y = player1verts_y_slag_1;
        player_1_trirefs = player_1_trirefs_slag_1;
        player_1_colrefs = player_1_colrefs_slag_1;
        
        player_1_vert_count = player_1_vert_count_slag_1;
        player_1_tri_count = player_1_tri_count_slag_1;
        return;
    }
    if (f==3){
        player1verts_x = player1verts_x_slag_2;
        player1verts_y = player1verts_y_slag_2;
        player_1_trirefs = player_1_trirefs_slag_2;
        player_1_colrefs = player_1_colrefs_slag_2;
        
        player_1_vert_count = player_1_vert_count_slag_2;
        player_1_tri_count = player_1_tri_count_slag_2;
        return;
    }
    if (f==4){
        player1verts_x = player1verts_x_slag_3;
        player1verts_y = player1verts_y_slag_3;
        player_1_trirefs = player_1_trirefs_slag_3;
        player_1_colrefs = player_1_colrefs_slag_3;
        
        player_1_vert_count = player_1_vert_count_slag_3;
        player_1_tri_count = player_1_tri_count_slag_3;
        return;
    }
    if (f==6){
        player1verts_x = player1verts_x_block_0;
        player1verts_y = player1verts_y_block_0;
        player_1_trirefs = player_1_trirefs_block_0;
        player_1_colrefs = player_1_colrefs_block_0;
        
        player_1_vert_count = player_1_vert_count_block_0;
        player_1_tri_count = player_1_tri_count_block_0;
        return;
    }
    if (f==7){
        player1verts_x = player1verts_x_nede_0;
        player1verts_y = player1verts_y_nede_0;
        player_1_trirefs = player_1_trirefs_nede_0;
        player_1_colrefs = player_1_colrefs_nede_0;
        
        player_1_vert_count = player_1_vert_count_nede_0;
        player_1_tri_count = player_1_tri_count_nede_0;
        return;
    }
    if ((f>=20)&&(f<20+FIFTH_OF_A_SECOND)){
        player1verts_x = player1verts_x_stille_0;
        player1verts_y = player1verts_y_stille_0;
        player_1_trirefs = player_1_trirefs_stille_0;
        player_1_colrefs = player_1_colrefs_stille_0;
        
        player_1_vert_count = player_1_vert_count_stille_0;
        player_1_tri_count = player_1_tri_count_stille_0;
        return;
    }
}

char addPlayer1Vert(int x, int y){
    if (player_1_vert_count>=PLAYER_VERTS_COUNT){
        return 0;
    }
    player1verts_x[player_1_vert_count]=x;
    player1verts_y[player_1_vert_count]=y;
    player_1_vert_count++;
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
    
    SetVideoMode (Video_Mode);
    
    setRGB0(&drawenvs[0], BACK_RED, BACK_GREEN, BACK_BLUE);  // Set clear color (dark purple)
    setRGB0(&drawenvs[1], BACK_RED, BACK_GREEN, BACK_BLUE);
    drawenvs[0].isbg = 1; // auto-clearing, maybe not necessary if I use a background sprite,
                          //  but for now I'm not drawing a background, so I need to enable clearing.
    drawenvs[1].isbg = 1;

    nextPrim = primBuff[0];
    
    loadFont();
}

void showDisplay() {
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
    POLY_G3 *triangle=NULL;
    int16_t player_x=player_1_x;
    int16_t player_y=player_1_y;
    uint8_t *player_red=player_1_red;
    uint8_t *player_green=player_1_green;
    uint8_t *player_blue=player_1_blue;
    int scale_x = player_scale_x;
    int scale_y = player_scale_y;
    int side_scale_x=1;
    int side_offset=0;
    
    int player_frame=player_1_frame;
    
    if (playerNum==1){
        player_x=player_2_x;
        player_y=(176-64)*256*player_scale_y/256/16;
        player_red=player_2_red;
        player_green=player_2_green;
        player_blue=player_2_blue;
        side_scale_x=-1;
        side_offset=256;
        
        player_frame=player_2_frame;
    }
    if(player_frame>=20){
        player_red=player_red_red;
        player_green=player_red_green;
        player_blue=player_red_blue;
    }
    
    if (player_1_x>player_2_x){
        side_scale_x=0-side_scale_x;
        side_offset=256-side_offset;
    }
    
    for (int tri_place=0; tri_place<player_1_tri_count*3;tri_place+=3){
            int tri_v1 = player_1_trirefs[tri_place];
            int tri_v2 = player_1_trirefs[tri_place+1];
            int tri_v3 = player_1_trirefs[tri_place+2];
            int tri_index=tri_v1;
            
            triangle = (POLY_G3*)nextPrim;
            setPolyG3 (triangle);
            setXY3 (triangle, player_x+(((int16_t)player1verts_x[tri_v1]*side_scale_x+side_offset)*scale_x)/16,
                              player_y+(((int16_t)player1verts_y[tri_v1])*scale_y)/16,
                              
                              player_x+(((int16_t)player1verts_x[tri_v2]*side_scale_x+side_offset)*scale_x)/16,
                              player_y+(((int16_t)player1verts_y[tri_v2])*scale_y)/16,
                              
                              player_x+(((int16_t)player1verts_x[tri_v3]*side_scale_x+side_offset)*scale_x)/16,
                              player_y+(((int16_t)player1verts_y[tri_v3])*scale_y)/16);
            char basecol= player_1_colrefs[tri_index];
            char tri_c1 = player_red[basecol];
            char tri_c2 = player_green[basecol];
            char tri_c3 = player_blue[basecol];
            setRGB0(triangle, tri_c1, tri_c2, tri_c3);
            tri_index=tri_v2;
            basecol= player_1_colrefs[tri_index];
            tri_c1 = player_red[basecol];
            tri_c2 = player_green[basecol];
            tri_c3 = player_blue[basecol];
            setRGB1(triangle, tri_c1, tri_c2, tri_c3);
            tri_index=tri_v3;
            basecol= player_1_colrefs[tri_index];
            tri_c1 = player_red[basecol];
            tri_c2 = player_green[basecol];
            tri_c3 = player_blue[basecol];
            setRGB2(triangle, tri_c1, tri_c2, tri_c3);
            
             addPrim(orderingTables[currentBuffer], triangle);
            
            nextPrim += sizeof(POLY_G3);
        }
}

void getCommonInput(uint16_t button_input){
    if (button_input & PAD_START){
        // reset everything
        player_1_health = PLAYER_MAX_HEALTH;
        player_2_health = PLAYER_MAX_HEALTH;
        player_1_raw_x = 64*256;
        player_1_raw_y = 64*256;
        player_1_speed_x = 0;
        player_1_speed_y = 0;
        player_2_raw_x = 20*16*256;
        player_2_raw_y = 64*256;
        player_2_speed_x = 0;
        player_2_speed_y = 0;
        
        player_1_frame=0;
        player_2_frame=0;
        
        // reset text:
        show_text=language_text+RESULT_EMPTY;
    }
    if ((button_input & PAD_SELECT) && (select_was_pressed <= 0)){
        int message=show_text-language_text;
        current_language++;
        if (current_language>2){
            current_language=0;
        }
        language_text=language_pointers[current_language];
        show_text=language_text;
        select_was_pressed = FIFTH_OF_A_SECOND;
        //printf("Select now: %d\n\n", select_was_pressed);
    }
    else if (((button_input & PAD_SELECT)==0) && (select_was_pressed>0)){
        //printf("Select decreasing: %d > %d\n", select_was_pressed, select_was_pressed-1);
        select_was_pressed--;
    }
}

void getInput(uint16_t button_input, uint16_t player_id){
        signed int player_move_x = 0;
        signed int player_move_y = 0;
        
        
        signed int player_dash_state=player_1_dash_state;
        signed int player_punch_state=player_1_punch_state;
        signed int player_speed_x=player_1_speed_x;
        signed int player_speed_y=player_1_speed_y;
        
        int player_raw_x=player_1_raw_x;
        int player_raw_y=player_1_raw_y;
        int player_x=player_1_x;
        int player_y=player_1_y;
        
        int player_frame=player_1_frame;
        
        if ((player_frame>20) && (player_frame<20+FIFTH_OF_A_SECOND)){
            player_frame++;
        }
        if (player_id==1){
            player_dash_state=player_2_dash_state;
            player_punch_state=player_2_punch_state;
            
            player_speed_x=player_2_speed_x;
            player_speed_y=player_2_speed_y;
            
            player_raw_x=player_2_raw_x;
            player_raw_y=player_2_raw_y;
            player_x=player_2_x;
            player_y=player_2_y;
            
            player_frame=player_2_frame;
        }
        
        if ((player_frame>20) && (player_frame<20+FIFTH_OF_A_SECOND)){
            player_frame++;
            if (player_id==0){
                player_1_frame=player_frame;
                return;
            }
            if (player_id==1){
                player_2_frame=player_frame;
                return;
            }
        }
        if (player_frame==6){
            player_frame=0;
        }
        
        if (button_input & PAD_LEFT){
            player_move_x--;
        }
        if (button_input & PAD_RIGHT){
            player_move_x++;
        }
        if (button_input & PAD_UP){
            player_move_y--;
        }
        if (button_input & PAD_DOWN){
            player_move_y++;
            //puts("Down?");
        }
        if ((button_input & PAD_TRIANGLE)&&(player_frame==0)){
            player_frame=6;
            if (player_id==0){
                player_1_frame=player_frame;
                return;
            }
            if (player_id==1){
                player_2_frame=player_frame;
                return;
            }
        }
        else if ((button_input & PAD_CIRCLE)&&(player_dash_state==0)){
            player_dash_state=FIFTH_OF_A_SECOND*5;//half a second
            if (player_move_x<0){
                player_dash_state=0-player_dash_state;
                player_speed_x=(player_dash_state*8+400)*FRAME_MULTIPLIER;
            }
            //printf("dash state: %d\n", player_dash_state);
        }
        else if ((button_input & PAD_CROSS)&&(player_punch_state==0)){
            player_punch_state=FIFTH_OF_A_SECOND*5;//half a second
            if (player_move_x<0){
                player_punch_state=0-player_punch_state;
            }
            //printf("punch state: %d\n", player_punch_state);
        }
        else if (player_dash_state>0){
            player_dash_state--;
            player_speed_x=player_dash_state*16*FRAME_MULTIPLIER;
        }
        else if (player_dash_state<0){
            player_dash_state++;
            player_speed_x=player_dash_state*16*FRAME_MULTIPLIER;
        }
        if ((button_input & PAD_L1)&&(player_scale_x>=2)){
            player_scale_x--;
            player_scale_y--;
        }
        if ((button_input & PAD_R1)&&(player_scale_x<=15)){
            player_scale_x++;
            player_scale_y++;
        }
        int player_old_speed_x = player_speed_x;
        if (player_punch_state){
            signed int punch=player_punch_state;
            if (punch<0){
                punch=0-punch;
            }
            punch=(FIFTH_OF_A_SECOND*5)-punch;
            int punch_frame=punch/FIFTH_OF_A_SECOND;
            player_frame = 1+punch_frame;
            //printf("punch: %d: %d, %d\n", player_punch_state, punch, punch_frame, player_frame);
        }
        else{
            player_frame=0;
        }
        if (player_punch_state>0){
            player_punch_state--;
        }
        else if (player_punch_state<0){
            player_punch_state++;
        }
        if (player_dash_state==0){
            if(player_move_x>0){
                if (player_speed_x>(64*FRAME_MULTIPLIER-5*FRAME_MULTIPLIER)){
                    player_speed_x=64*FRAME_MULTIPLIER;
                }
                else{
                    player_speed_x+=6*FRAME_MULTIPLIER;
                }
            }
            else if(player_move_x<0){
                if (player_speed_x<(-64*FRAME_MULTIPLIER+8*FRAME_MULTIPLIER)){
                    player_speed_x=0-64*FRAME_MULTIPLIER;
                }
                else{
                    player_speed_x-=6*FRAME_MULTIPLIER;
                }
            }
            else{
                if (player_speed_x>12*FRAME_MULTIPLIER){
                    player_speed_x-=12*FRAME_MULTIPLIER;
                }
                else if (player_speed_x<-12*FRAME_MULTIPLIER){
                    player_speed_x+=12*FRAME_MULTIPLIER;
                }
                else{
                    player_speed_x=0;
                }
            }
            // = ( (player_move_x*64*FRAME_MULTIPLIER) + 7*player_old_speed_x)/8;
        }
        
        player_speed_y+=FRAME_MULTIPLIER*3;
        player_raw_y+=player_speed_y;
        if (player_raw_y>=(176-64)*256){
            player_raw_y=(176-64)*256;
            player_speed_y=0;
            if (0){
                player_speed_x = ( (player_move_x*64*FRAME_MULTIPLIER) + 7*player_old_speed_x)/8;
            }
            if (player_move_y<0){
                player_speed_y=(0-150)*FRAME_MULTIPLIER;
            }
        }
        player_raw_x +=  player_speed_x;
        player_x=player_raw_x*player_scale_x/128/16;
        player_y=player_raw_y*player_scale_y/256/16;
        
        
        if (player_id==1){
            player_2_dash_state=player_dash_state;
            player_2_punch_state=player_punch_state;
            
            player_2_speed_x=player_speed_x;
            player_2_speed_y=player_speed_y;
            
            player_2_raw_x=player_raw_x;
            player_2_raw_y=player_raw_y;
            player_2_x=player_x;
            player_2_y=player_y;
            
            player_2_frame=player_frame;
        }
        else{
            player_1_dash_state=player_dash_state;
            player_1_punch_state=player_punch_state;
            
            player_1_speed_x=player_speed_x;
            player_1_speed_y=player_speed_y;
            
            player_1_raw_x=player_raw_x;
            player_1_raw_y=player_raw_y;
            player_1_x=player_x;
            player_1_y=player_y;
            
            player_1_frame=player_frame;
        }
}

void drawHealth(){
    TILE *tile;
    // player 1
    if(player_1_health>0){
        tile = (TILE*)nextPrim;
        setTile(tile);
        setXY0(tile, 20, 200);
        setWH(tile, player_1_health*(240/PLAYER_MAX_HEALTH), 20);
        setRGB0(tile, player_1_red[10], player_1_green[10], player_1_blue[10]);
        addPrim(orderingTables[currentBuffer], tile);
        nextPrim += sizeof(TILE);
    }
        
    // player 2
    if(player_2_health>0){
        tile = (TILE*)nextPrim;
        setTile(tile);
        setXY0(tile, 10+256, 200);
        setWH(tile, player_2_health*(240/PLAYER_MAX_HEALTH), 20);
        setRGB0(tile, player_2_red[10], player_2_green[10], player_2_blue[10]);
        addPrim(orderingTables[currentBuffer], tile);
        nextPrim += sizeof(TILE);
    }
}

void handleHits(){
    // let players get hit
    // FIXME: this code really shows why each player should have a struct
    player_1_hit--;
    player_2_hit--;
    
    if ((player_1_frame==4) &&
         (
              (
                  (player_2_raw_x-player_1_raw_x < 81920-73684) &&
                  (player_2_raw_x>player_1_raw_x)
              ) ||
              (
                  (player_1_raw_x-player_2_raw_x < 81920-73684) &&
                  (player_2_raw_x<player_1_raw_x)
              )
         ) &&
         ( player_2_frame<20 )
         &&
         ( player_2_frame!=6 )
         &&
         (player_1_hit<=0)
       ){
    
        player_2_frame=20;
        player_1_hit=FIFTH_OF_A_SECOND;
        if (player_2_health>5){
            player_2_health-=5;
        }
        else{
            player_2_health=0;
            player_2_frame=7;
        }
        //printf("Health: %d, %d\n", player_1_health, player_2_health);
        
    }
    
    // start of repeated code
    if ((player_2_frame==4) &&
         (
              (
                  (player_1_raw_x-player_2_raw_x < 81920-73684) &&
                  (player_1_raw_x>player_2_raw_x)
              ) ||
              (
                  (player_2_raw_x-player_1_raw_x < 81920-73684) &&
                  (player_1_raw_x<player_2_raw_x)
              )
         ) &&
         ( player_1_frame<20 )
         &&
         ( player_1_frame!=6 )
         &&
         (player_2_hit<=0)
       ){
    
        player_1_frame=20;
        player_2_hit=FIFTH_OF_A_SECOND;
        if (player_1_health>5){
            player_1_health-=5;
        }
        else{
            player_1_health=0;
            player_1_frame=7;
        }
        
    }
    
    // FIXME: I'm setting a variable and then checking it? Maybe that's correct
    if (player_1_health<=0 && player_2_health<=0){
        show_text=language_text+RESULT_TIE;
    }
    else if (player_1_health<=0){
        show_text=language_text+RESULT_TWO;
    }
    else if (player_2_health<=0){
        show_text=language_text+RESULT_ONE;
    }
    // end of repeated code
}

void drawText(unsigned char *text){
    int nextPlace=0;
    SPRT *sprite;
    for (int textPlace=0; text[textPlace]!=0;textPlace++){
        unsigned char oldc = text[textPlace];
        if (oldc!=' '){
            unsigned int newc=oldc-65;
            sprite = (SPRT*)nextPrim;
            setSprt(sprite);
            setXY0 (sprite, 50+nextPlace, 50);
            setWH  (sprite, textSizs[newc]*2, 18);
            setUV0 (sprite, (textRows[newc])*2, 18*textCols[newc]);
            setClut(sprite, font_texture.crect->x, font_texture.crect->y);
            setRGB0(sprite, 128, 128, 128); // what does this colour do and why do I need it?
            addPrim(orderingTables[currentBuffer], sprite);
          
            nextPlace+=2*(textSizs[newc]+2);
            nextPrim += sizeof(SPRT);
            //printf("character: %d\n", newc);
        }
        else{
            nextPlace+=18;
        }
    }
}
    

int main() {
    initDisplay();
    initControllers();

    int controller_type = 0;
    uint16_t button_input_1 = 0;
    uint16_t button_input_2 = 0;
    while(1) {
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
        
        
        if ((player_1_health>0) && (player_2_health>0)){
            getInput(button_input_1, 0);
            getInput(button_input_2, 1);
        }
        else{
            getCommonInput(button_input_1|button_input_2);
        }

        ClearOTagR(orderingTables[currentBuffer], OTLENGTH); 
        
        handleHits();
        drawText (show_text);
        drawHealth();
        
        setPlayer1Frame(player_1_frame);
        drawPlayer(0);
        setPlayer1Frame(player_2_frame);
        drawPlayer(1);
        // Update the display
        showDisplay();
        
    }
    
    return 0;
}
