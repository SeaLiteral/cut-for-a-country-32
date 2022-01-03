# Cut for a Country

A homebrew fighting game for the PS1, made using PSn00bSDK.

Controls in game:
- Cross to punch.
- Circle to dash.
- L1 and R1 to zoom out and in.
- Dpad to move left and right. Up jumps, but there's no use for jumping.

Controls after a battle:
- Start to start a new battle.
- Select to change the text language.


Known issues (post-jam versions):
- You can crouch in mid-air: if you press up, then hold triangle before you land, you'll be crouching/blocking in mid-air. I guess some might consider that a feature rather than a bug, but I think what you can do in mid-air shouldn't be the same as what you can do on the ground, and the way this happens it also turns off gravity until you release the triangle button.
- There are problems, apparently in libpsn00b's GPU-related assembly code, that can cause the game to show wrong in no$psx and possibly other emulators. This is why I ony included an NTSC build, and these problems are still present in post-jam builds.
- Player two can't jump. I'll hopefully fix this soon. It seems to have been there for a while.


Known issues (jam version):
- The Spanish translation showed wrong if player two wins due to using out-of-font characters. And it partially goes out of the screen: it's too far to the right. 
- Text didn't show correctly in DuckStation (and probably looked wrong in some other emulators and on real hardware too). This was caused by the TIM data being misaligned in RAM.
- The psxgpu issues I mention for post-jam versions also existed in the jam version.
- Language switching didn't work. This was a hardcoded reference to the English text data getting to overwrite the pointers that were set when switching langauges. It should be fixed in postjam-builds.
- The letter Z wouldn't have drawn correctly, as its coordinates were wrong and its width was negative. That should be fixed in 2022 builds.
- You can crouch in mid-air, I haven't fixed that yet.
- Player two can't jump. This issue still exists in the January versions.
