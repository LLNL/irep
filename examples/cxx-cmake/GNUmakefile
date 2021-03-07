input = input.lua

test: build/cxx_prog
	build/cxx_prog $(input)

build/cxx_prog:
	mkdir -p build
	cd build && cmake .. && make

clean:
	rm -rf build
