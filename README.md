## Building

The build project is generated with CMake but currently this program only builds with GCC (including MinGW and Cygwin) since it uses POSIX extensions to read the folder contents.



## Usage

1. Create a folder and drop 'anim_parser.exe' into it.
2. Unpack all game animations from '<GTA:SA root>/anim/anim.img' to the same folder.
3. Run 'anim_parser.exe'.
4. If everything is done properly, the program would create a text file called 'anim_parser.txt' that contains something like this:<br/>
```
Library: AIRPORT, 1 animations
	Animation: thrw_barl_thrw, 120 frames

Library: Attractors, 3 animations
	Animation: Stepsit_in, 100 frames
	Animation: Stepsit_loop, 200 frames
	Animation: Stepsit_out, 100 frames

Library: BAR, 12 animations
	Animation: Barcustom_get, 180 frames
	Animation: Barcustom_loop, 120 frames
	Animation: Barcustom_order, 220 frames
	Animation: BARman_idle, 500 frames
	Animation: Barserve_bottle, 180 frames
...
```
