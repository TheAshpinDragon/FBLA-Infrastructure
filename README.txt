==================================== README FILE ===================================================

2021-2022 FBLA COMPUTER GAME & SIMULATION PROGRAMMING PROJECT
ALEXANDER HIGH SCHOOL, DOUGLAS COUNTY SCHOOL SYSTEM, GEORGIA

==================================== LICENCE OLC-3 =================================================

License (OLC-3)
Copyright 2018, 2019, 2020, 2021 OneLoneCoder.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions or derivations of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

Redistributions or derivative works in binary form must reproduce the above copyright notice. This list of conditions and the following disclaimer must be reproduced in the documentation and/or other materials provided with the distribution.

Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

==================================== PROJECT INFO =================================================

Writen in C++, using the OLC Pixel Game Engine.
	GitHub: https://github.com/OneLoneCoder/olcPixelGameEngine
	Documentation:  https://github.com/OneLoneCoder/olcPixelGameEngine/wiki

Group members:
	David Gorzynski - Programmer - dagorzynski@gmail.com - 770 722 2508
	Emilio Lopez - Composer - emiliolopezwham@gmail.com - 770 885 0478
	Brayden Bayle - Artist - 3brayden.bayle3@gmail.com - 470 869 6328 

DESCRIPTION:
This is a top-down city building arcade game that requires the user to quickly expand before time runs out.

HOW TO PLAY:
Open the application, it will take a moment to load.
Controlls are listed on the bottom of the splash screen, or here:
	ENTER  = PLAY
	ESC    = CLOSE GAME
	WASD   = MOVE CURSOR
	ARROWS = MOVE CURSOR
	LMB    = BUILD/UPGRADE
	RMB    = CYCLE OPTIONS
Press 'ENTER' to create a new game.
Use the controlls listed above to navigate, build, and upgrade:
   Structures:
	Tower - Houses residents
	E-plant - Genreates electricity	
	W-plant - Disposes of waste
	C-plant - Creates construction materials
	Road - Manages traffic
   In order for structures to work, you must place roads around them.
   You can view the cost of construction and upgrades in the bottom bar.
   When the red cursor is over a blank tile, it will show build options
   When the red cursor is over a structure (accept for roads), it will show upgrades and info
   Gain enough population to move on to the next map
Traverse the three maps (grassland, forest, mountains) to beat the game, or, if you run out of time or resources, lose.
After reaching an end game state, you can enter your name into the score board:
	AD    = move carrot left and right
	WS    = cycle charater
	ENTER = submit
You will be returned to the main menu

EXTRA KEY BINDS:
	SHIFT + R = reset
	SHIFT + V = speed up game
	SHIFT + C = advance window
		Main Menu: changes from video to score board
		Game: wins the level