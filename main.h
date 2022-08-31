#include <psxgte.h>
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
extern const int Video_Mode;
#else
#define SCREEN_WIDTH  512
#define SCREEN_HEIGHT 240
#define FRAME_MULTIPLIER 5
#define FIFTH_OF_A_SECOND 6
#define THREE_FRAMES 3
extern const int Video_Mode;
#endif

#define OTLENGTH 32768 // Size of the ordering table

#define RANDOM_STATE_SIZE 7
extern int pseudoRandomState[RANDOM_STATE_SIZE];
extern uint16_t pseudoRandomStep;


// 3D stuff
extern VECTOR  model_position_in_world;
extern SVECTOR model_rotation;

extern MATRIX test_matrix;

enum Game_Modes {
    LANGUAGE_MENU,
    MAIN_MENU,
    CHARACTERS_MENU,
    OPTIONS_MENU,
    TEST_MENU,
    SHOW_RESULT,
    MENU_LEAVE_LANGUAGES,
    MENU_LEAVE_MAIN,
    MENU_LEAVE_CHARACTERS,
    MENU_LEAVE_OPTIONS,
    MENU_LEAVE_TEST,
    FIGHT_MODE,
    AMOUNT_OF_GAME_MODES,
} extern game_mode;
extern int exit_from_mode[AMOUNT_OF_GAME_MODES];
extern int amount_of_players;


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
    int height;
} Player_Mesh;

// FIXME: This could use an enum
#define CHARACTER_NISSE 0
#define CHARACTER_GULLEROD 1

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
    int16_t character_id;
    AI ai;
    Player_Mesh mesh;
} Player;



extern Player player_1, player_2;


extern int player_scale_x;
extern int player_scale_y;

extern int current_screen_time;

extern int unlocked_characters;
extern char last_winner;
extern char won_last_fight;

extern void initModes();
extern void initMenu(int selection);
extern int addMenuOption(int string_id);
extern int addMenuOptionWithFun(int string_id, void (*option_action)(int, int) );
extern void setLanguage(int lang, int action);
extern void initLanguageMenu();
extern void initOptionsMenu();
extern void initTestMenu();
extern void initMainMenu();
extern void maybeInitMainMenu(int nothing, int action);
extern void maybeSetPlayerAmount(int menu_sel, int action);
extern void maybeResetGame(int nothing, int action);
extern void maybeInitOptionsMenu(int nothing, int action);
extern void maybeInitTestMenu(int nothing, int action);
extern void maybeInitLanguageMenu(int nothing, int action);
extern void initCharactersMenu();
extern void setCharacter(int character, int action);
extern void setPlayerFrame(int f, int player_id);
extern void initDisplay();
extern void swapBuffers();
extern void loadFont();
extern void initControllers();
extern void drawPlayer(int playerNum);
extern void draw3DTest();
extern void resetGameState();
extern uint16_t debounceInputs(uint16_t button_input);
extern void getResultScreenInput(uint16_t button_input);
extern void getMenuInput(uint16_t button_input);
extern void getAttractInput(uint16_t button_input);
extern uint16_t randomNumber();
extern void simulateInput(uint16_t player_id);
extern void getGameInput(uint16_t button_input, uint16_t player_id);
extern void resetGameState();
extern uint16_t debounceInputs(uint16_t button_input); // FIXME: move this into the players
extern void getResultScreenInput(uint16_t button_input);
extern void drawPlayer(int playerNum);
extern void drawHealth();
extern void drawGround();
extern void handleHits();
extern void setPlayer1Frame(int f);
extern void drawText(unsigned char *text, int textY);
extern int main();
