# Cuts for a Country

A homebrew fighting game for the PS1, made using PSn00bSDK.

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
Carrot, the King of the Cuts, has decided to make a fighting competition, and whoever wins will get to rule the country. To make sure as many people has possible have the ability to participate, he's declared national holidays for a week, which should be enough time for everyone who wants to fight in the competition.

## Characters
There's currently two characters in the game:
- Juul, a Christmas elf/gnome who got tired of living in a farm, so now he wants a castle. His favourite holiday is Christmas.
- Carrot, the King of the Cuts, who organizes the competition, but also participates in it somehow. Doesn't like Easter, because he's a carrot, and some Easter bunnies tend to eat those. He does need them to cut his hair though.

Other characters I've thought of adding:
- A vampire whose favourite holiday is Halloween. He might distinguish between signed and unsigned bites.
- An Easter egg that cares about bite order.
- The elf's pet.
- A bee.

## Known issues

Known issues (post-jam versions):
- You can crouch in mid-air: if you press up, then hold triangle before you land, you'll be crouching/blocking in mid-air. I guess some might consider that a feature rather than a bug, but I think what you can do in mid-air shouldn't be the same as what you can do on the ground, and the way this happens it also turns off gravity until you release the triangle button.
- There are problems, apparently in libpsn00b's GPU-related assembly code, that can cause the game to show wrong in no$psx and possibly other emulators. This is why I ony included an NTSC build, and these problems are still present in post-jam builds.


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

## Credits
I'm making this game myself, writing the text in the three languages I speak reasonably well.

I'm using PSn00bSDK, which is under the Mozilla Public Licence and can be found on <https://github.com/Lameguy64/PSn00bSDK>.
