CXX := clang++
CC := clang



EXTERNAL := external
CPPFLAGS := \
    -I./include \
    -I$(EXTERNAL)/metal-cpp \
    -I$(EXTERNAL)/metal-cpp-extensions \
	-I$(EXTERNAL)/GLFW \
	-I$(EXTERNAL)/stb \

CXXFLAGS := -Wall -std=c++23 -O2 -fno-objc-arc
CFLAGS := -Wall -std=c11 -O2
CPPFLAGS += -I$(shell brew --prefix glfw)/include
CPPFLAGS += -I$(shell brew --prefix cglm)/include
LDFLAGS += \
    -L$(shell brew --prefix glfw)/lib/ \
	-L$(shell brew --prefix cglm)/lib/ \
    -framework Metal \
    -framework Foundation \
    -framework Cocoa \
    -framework CoreGraphics \
    -framework MetalKit \
    -framework ModelIO \
    -framework MetalPerformanceShaders \
	-framework QuartzCore

LDLIBS += -lglfw
TARGET := build/metal
SRC_C   := $(wildcard src/*.c)
SRC_CPP := $(wildcard src/*.cpp)
SRC_MM  := $(wildcard src/*.mm)
SRC_METAL  := $(wildcard shaders/*.metal)
SRC_METAL1 := $(wildcard include/*.metal)
OBJ := \
    $(patsubst src/%.c,build/%.c.o,$(SRC_C)) \
    $(patsubst src/%.cpp,build/%.cpp.o,$(SRC_CPP)) \
    $(patsubst src/%.mm,build/%.mm.o,$(SRC_MM))
	
ASSETS  := $(patsubst assets/%,build/assets/%,$(wildcard assets/*))
BUILD_DIR := build
FILES_TO_COPY := build/default.metallib build/default.air
LIB_D := -Llib/

$(BUILD_DIR):
	mkdir -p $@

.DEFAULT_GOAL := all
.PHONY: all clean run
.SECONDARY:

all: $(TARGET) $(ASSETS) $(FILES_TO_COPY)

$(BUILD_DIR)/%: % | $(BUILD_DIR)
	mkdir -p $(dir $@)
	cp $< $@

build/assets/%: assets/%
	mkdir -p $(dir $@)
	cp $< $@

build/default.air: $(SRC_METAL) $(SRC_METAL1) | $(BUILD_DIR)
	xcrun -sdk macosx metal -c $(SRC_METAL) $(SRC_METAL1) -o $@

build/default.metallib: build/default.air
	xcrun -sdk macosx metallib $< -o $@

$(TARGET): $(OBJ) $(ASSETS) build/default.metallib
	$(CXX) $(CXXFLAGS) $(OBJ) $(LDFLAGS) $(LIB_D) $(LDLIBS) -o $@

build/%.c.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/%.cpp.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build/%.mm.o: src/%.mm
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -rf build