#include <sys/types.h>

#include <psxetc.h>
//#include <psxgte.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <psxapi.h>
#include <stdio.h>

#include "main.h"

void simulateOldInput(uint16_t player_id){
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
    uint16_t dodge_thresold = 5;
    if (this_player->character_id == CHARACTER_GULLEROD){
        dodge_thresold = 3;
    }
    
    if (this_player->raw_x<other_player->raw_x-7500){
        should_go_to_the_right = 1;
    }
    else if (this_player->raw_x>other_player->raw_x+7500){
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
        this_player->ai.dodge = ((randomNumber()&7) > dodge_thresold);
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

void getOldGameInput(uint16_t button_input, uint16_t player_id){
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
