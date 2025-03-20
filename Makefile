OUTPUT = emulator
NAME = emulator
SRCS = $(shell find . -type f \( -iname \*.cpp -o -iname \*.cc \) | grep  -v "game-music-emu\/demo\|game-music-emu\/player\|game-music-emu\/test\|Vgm_Emu\|Ym2612_\|Gym_Emu")

default:
	$(MAKE) create_target_dir
	$(MAKE) build
	$(MAKE) copy

build:
	echo $(SRCS) | sed 's/ /\n/g'
	echo
	emcc --bind $(SRCS) \
    -o $(OUTPUT)/$(NAME).js \
    -I ./game-music-emu/gme \
    -I ./game-music-emu/ \
    -I ./msfa/ \
    -s USE_SDL=2 \
    -s ENVIRONMENT=shell \
    -s SINGLE_FILE=1 \
    -s EXPORTED_FUNCTIONS=_main,_malloc,_free \
    -s EXPORTED_RUNTIME_METHODS=addFunction,removeFunction \
    -s EXPORT_ES6=1 \
    -s MODULARIZE=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s ALLOW_TABLE_GROWTH=1 \
    -std=c++23 \
    -O3

	$(MAKE) copy

copy:
	cp index.html *.js server emulator/

clean:
	rm emulator/*

create_target_dir:
	mkdir -p emulator/
