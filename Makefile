run: install
	cd bin&&./'Sonic, jump!'
install: clean
	mkdir bin
	gcc src/engine/main.c src/game.c -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm -o 'bin/Sonic, jump!'
clean:
	rm bin -rf
