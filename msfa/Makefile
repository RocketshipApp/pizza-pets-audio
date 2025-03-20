OUTPUT = emulator
NAME = emulator
SRCS = $(shell find . -type f \( -iname \*.cpp -o -iname \*.cc \) )

default:
	$(MAKE) build
	$(MAKE) copy

build:
	echo $(SRCS) | sed 's/ /\n/g'
	echo
	emcc --bind $(SRCS) \
    -o $(OUTPUT)/$(NAME).js \
    -s USE_SDL=2 \
    -s ASSERTIONS=1 \
    -s ENVIRONMENT=shell \
    -s SINGLE_FILE=1 \
    -s EXPORTED_FUNCTIONS=_main,_malloc,_free \
    -s EXPORT_ES6=1 \
    -s MODULARIZE=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s ALLOW_TABLE_GROWTH=1 \
    -std=c++23 \
    -O3

	$(MAKE) copy

copy:
	cp *.html *.js server emulator/

clean:
	rm emulator/*
