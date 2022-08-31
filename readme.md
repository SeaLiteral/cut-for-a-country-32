# Klipperne
A homebrew game engine for the PS1, made using PSn00bSDK and a fighting game made using the engine. The name "Klipperne" can mean "the people who cut threads/hair/wool/paper" or "the rocks" (as in mountains).

License: Apache

## Text rendering
Text should be stored in the `interface.ini` file and will get converted into a binary file when building the game. Here's some code from the example file:
```
# Language names: keep each language named in itself
.lang_da "DANSK"
.lang_en "ENGLISH"
.lang_es "ESPAÑOL"
```
It lists the languages that the game is localized in. Each language is introduced with `.lang_xx` with xx replaced by a language code. Then there's a quoted string containing the name of the language in the language itself as it will appear on the language selection screen. Lines starting with a hash character are comments.

The list of languages doesn't (yet) automatically get fetched by the language selection screen, so you'll need to edit it by hand. If it contains Danish, Spanish and English, it'll take this form:
```c
void initLanguageMenu(){
    initMenu(current_language);
    addMenuOptionWithFun (UI_LANG_DA, setLanguage);
    addMenuOptionWithFun (UI_LANG_EN, setLanguage);
    addMenuOptionWithFun (UI_LANG_ES, setLanguage);
    game_mode = LANGUAGE_MENU;
}
```
The `addMenuOptionWithFun` function will be explained later, but to add and remove languages you'd add and remove calls to that function from `initLanguageMenu`. As you can see, `.lang_da` becomes `UI_LANG_DA` and so on. That's because the C code accesses string pointers either through defines or constants. The current version of the engine uses defines, but I might change it if I find that constants have some advantage.

To display a string after a match in the fighting game, I use code like this:
```
show_text=UI_RE_WINNER_1; // Select the string to show
game_mode = SHOW_RESULT; // Enter a mode where the text will stay on screen until the player presses a button.
```

To create a menu you usually do three things:
- Call the `initMenu(selection)` function. The value of `selection` should be the amount of menu options shown before the option that would be highlighted by default.
- Add each menu item using the function `int addMenuOptionWithFun(int string_id, void (*option_action)(int, int) )`. The string_id should be one of the defines/constants generated from the `interface.ini` file (so it would start with `UI_`, then an uppercased version of the string ID used in that file. The action should be a function that takes an `int` parameter and returns an `int` result. See below for how to implement an action function.
- Set the `game_mode` to a menu mode, which might be specific for the menu you're creating (see "Menu modes" below).

You can look at the language selection screen for an example.

### Action functions
The action function takes two arguments:
- indicates which menu option is highlighted.

Example:
```c
void setLanguage(int lang, int action){
    current_language = lang;
    if (action==ACTION_CONFIRM){
        printf ("Language got set: %d!\n", lang);
        initMainMenu();
        return;
    }
    if (action==ACTION_CANCEL){
        current_language = previous_menu_selection;
    }
}
```
The `previous_menu_selection` is the option that was highlighted by default. Because of this, pressing triangle while on the language selection menu without previously having selected a language leads to English being selected, even if another language was highlighted.

### Menu modes
There's conditional code in the mainloop that checks for whether to show a menu. The way it then shows a menu is:

```
getMenuInput(button_input_1|button_input_2);
drawMenu();
```
`button_input_1` and `button_input_2` are the button inputs from player 1 and 2 respectively.

### More about the `interface.ini` format
The name of a string should be made of letters, numbers and underscores and it can't start with a number. Usually string IDs start with a code that indicate which "group" a string belongs to, for example main menu option labels start with `mm_`.

Each string has its own "section" in the file, for example:
```
.mm_1_player
da = "1 SPILLER"
en = "SINGLE PLAYER"
es = "1 JUGADOR"
```
The section starts with a line that starts with a dot and then the ID of a string. There might also be filters to conditionally include the string.

Each other line in the section would start with a language code, then have an equal sign, then the string in that language.

Here's an example of a filtered section:
```
.mm_exit ?pc|mobile
da = "LUK"
en = "EXIT"
es = "SALIR"
```
This would mean that the "exit" string is only needed in PC or mobile builds of the game. That way the option won't be shown if the game is being built for a platform that doesn't have any of the properties "pc" or "mobile". For example, the string would be left out of an HTML5 port of the game. Note that building the game for platforms other than PS1 is currently not implemented, so there's not much use for filters.

I'm aware that modern consoles are kinda PC-like in that games might need to have an exit option because now games are usually installed on the consoles and it has to be possible to exit a game without turning of or resetting the console. So maybe the filters for the exit option should work in a different way.

## Random number generator
The function `randomNumber` returns an unsigned 16-bit value. It uses a simple lagged Fibonacci generator, but that's still more random than most implementations of C's `rand` function. 

# Cuts for a Country
The fighting game, 2D, with 2D graphics made of triangles. Mostly tested in DuckStation.

License: Apache

## Controls
Controls in game:
- Cross to punch.
- Circle to dash.
- L1 and R1 to zoom out and in.
- Dpad to move left and right. Up jumps, but there's no use for jumping.
Right now, the only way to exit a fight is for the fight to finish (by one of the characters winning, or by a tie). It's also currently not possible to pause a fight. I want to fix both of these issues.

Controls after a battle:
- Start to start a new battle.
- Select to change the text language.

## Story
Carrot, the King of the Cuts, is a bit tired of ruling, and has decided to make a fighting competition, and whoever wins will get to rule the country. To make sure as many people has possible have the ability to participate, he's declared national holidays for a week, which should be enough time for everyone who wants to fight in the competition.

## Characters
There's currently two characters in the game:
- Nissen, a Christmas elf/gnome who got tired of living in a farm, so now he wants a castle. His favourite holiday is Christmas.
- Carrot, the King of the Cuts, who organizes the competition, but also participates in it somehow. Doesn't like Easter, because he's a carrot, and some Easter bunnies tend to eat those. He does need them to cut his hair though.

Other characters I've thought of adding:
- A vampire whose favourite holiday is Halloween. He might distinguish between signed and unsigned bites.
- An Easter egg that cares about bite order.
- The elf's pet: a cat that reads a lot.
- A witch that's tired of anti-witchcraft laws in the country and wants to change them.

## Known issues

Known issues (current version):
- You can crouch in mid-air: if you press up, then hold triangle before you land, you'll be crouching/blocking in mid-air. I guess some might consider that a feature rather than a bug, but I think what you can do in mid-air shouldn't be the same as what you can do on the ground, and the way this happens it also turns off gravity until you release the triangle button.
- There are problems, apparently in libpsn00b's GPU-related assembly code, that can cause the game to show wrong in no$psx and possibly other emulators. This is why I ony included an NTSC build, and these problems are still present in post-jam builds.
- The carrot animation looks weird when dodging.
- Some animations that exist in the game data don't actually get used in the game.


Known issues (jam version):
- The Spanish translation showed wrong if player two wins due to using out-of-font characters. And it partially goes out of the screen: it's too far to the right. 
- Text didn't show correctly in DuckStation (and probably looked wrong in some other emulators and on real hardware too). This was caused by the TIM data being misaligned in RAM.
- The psxgpu issues I mention for post-jam versions also existed in the jam version.
- Language switching didn't work. This was a hardcoded reference to the English text data getting to overwrite the pointers that were set when switching langauges. It should be fixed in postjam-builds.
- The letter Z wouldn't have drawn correctly, as its coordinates were wrong and its width was negative. That should be fixed in 2022 builds.
- You can crouch in mid-air, I haven't fixed that yet.
- Player two couldn't jump. This was actually a graphics issue, where the vertical position on the screen was being set to a constant.
- All versions before the the second week of 2022 probably have timing issues in EPSXE. I don't know if they're fixed there now, but I've made changes later to how I use VSync.

## TODO
A bunch of things:
- Add more animated actions.
- Add more characters.
- Properly separate code and data, and improve automation of conversion between file formats.
- Implement a proper "story mode".
- Add sound effects.
- Add music.
- Make jumping useful.
- Stop characters from running through each other, maybe except when dashing.
- Translate this file into Danish and Spanish.
- Add a "manual" and some "box art".
- Port the game to more platforms.
- Improve the large font and probably add a smaller font for longer pieces of text.
- Attempt to implement pixel-perfect hit detection and use that if it doesn't cause the framerate to drop or something.

## Credits
I'm making this game myself, writing the text in the three languages I speak reasonably well.

I'm using PSn00bSDK, which is under the Mozilla Public Licence and can be found on <https://github.com/Lameguy64/PSn00bSDK>.

I first made this game for the 32-bit Holiday Jam in 2021, then made some post-jam edits in January 2022. Then added a background "image" made of three rectangles during the 32-bit Spring Cleaning Jam in 2022, where I also made some animations that I didn't have time to actually make the game display.

Timeline:
- December 2021 for the 32bit Holiday Jam: simple combat, two players only
- January 2022: menus, single player mode, introduced the carrot character, added demo/attract mode.
- May 2022 during the 32bit Spring Cleaning Jam: simplified the animation tool, made some animations I didn't have time to actually make the game trigger, made the endscreen mention that it unlocks the carrot character, added a simple background "image" made of three rectangles, hiding the bright one when there's text in front of it because otherwise it could make the text hard to read.
- August 2022: Added license (Apache) and initial 3D code.
