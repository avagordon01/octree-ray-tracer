all: bin/render bin/build bin/convert

bin/render: src/render.c src/file.h src/vector.h
	clang -o $@ $< -O3 -lm -lGL -lglfw
bin/build: src/build.c src/file.h
	clang -o $@ $< -O3 -lm
bin/convert: src/convert.c
	clang -o $@ $< -O3 -lm

clean:
	git clean -Xfd

.PHONY: all clean
