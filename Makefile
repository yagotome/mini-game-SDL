run: install
	cd bin&&./'Sonic, jump!'
install: clean
	mkdir bin
	gcc src/main.c -lSDL2 -lSDL2_image -lSDL2_mixer -o 'bin/Sonic, jump!'
clean:
	rm bin -rf