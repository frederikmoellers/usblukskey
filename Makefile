#!/usr/bin/make -f

# this Makefile is a proxy for CMake. It enables the user to just call "make"
# in the root directory of the project and make will then call CMake or make
# in the correct subdirectories so the source is built out-of-tree.

.SECONDEXPANSION:

# Available targets
TARGETS := Debug Release
# Target to be used as default for building and installing
RELEASE_TARGET := Release

CMAKE_INPUT_FILES := CMakeLists.txt $(shell find src -type f -name CMakeLists.txt) $(shell find src -type f -name '*.in')

.PHONY: all install clean clean-% maintainer-clean

# RULES AND RECIPES

all: $(RELEASE_TARGET)

$(TARGETS): build/$$@
	$(MAKE) -C build/$@

build/%: $(CMAKE_INPUT_FILES)
	mkdir -p build/$*
	cd build/$* && cmake -DCMAKE_BUILD_TYPE=$* ../..

install: $(RELEASE_TARGET)
	$(MAKE) -C build/$(RELEASE_TARGET) install

install-%: build/$$*
	$(MAKE) -C build/$* install

clean: $(foreach target,$(TARGETS),clean-$(target))

clean-%: build/$$*
	$(MAKE) -C build/$* clean

maintainer-clean:
	rm -rf build
