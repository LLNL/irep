all: c cxx fortran

irep:
	mkdir -p build && \
		cd build && \
		cmake -DCMAKE_INSTALL_PREFIX=../irep ../.. && \
		make install

c: irep
	@echo -e $(cgreen)Testing IREP C Executable with GNU make$(cend)
	make -C c test

cxx: irep
	@echo -e $(cgreen)Testing IREP C++ Executable with CMake$(cend)
	make -C cxx-cmake test

fortran: irep
	@echo -e $(cgreen)Testing IREP Fortran Executable with GNU make$(cend)
	make -C fortran test

clean:
	make -C c clean
	make -C cxx-cmake clean
	make -C fortran clean
	rm -rf build irep
