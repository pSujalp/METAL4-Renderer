CXX := clang++
CC := clang



EXTERNAL := external
CPPFLAGS := \
    -I./include \
    -I$(EXTERNAL)/metal-cpp \
    -I$(EXTERNAL)/metal-cpp-extensions \
	-I$(EXTERNAL)/GLFW \
	-I$(EXTERNAL)/stb \
	-I$(EXTERNAL)/magic_enum \
	-I$(EXTERNAL)/ \
	-I./shaders \


CXXFLAGS := -Wall -std=c++23 -O2 -fno-objc-arc
CFLAGS := -Wall -std=c11 -O2
CPPFLAGS += -I$(shell brew --prefix glfw)/include
CPPFLAGS += -I$(shell brew --prefix glm)/include
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
METAL_AIR := \
    $(patsubst include/%.metal,build/%.air,$(SRC_METAL1)) \
    $(patsubst shaders/%.metal,build/%.air,$(SRC_METAL))
OBJ := \
    $(patsubst src/%.c,build/%.c.o,$(SRC_C)) \
    $(patsubst src/%.cpp,build/%.cpp.o,$(SRC_CPP)) \
    $(patsubst src/%.mm,build/%.mm.o,$(SRC_MM))
	
ASSET_DIR := build/assets
ASSETS := $(ASSET_DIR)
BUILD_DIR := build
FILES_TO_COPY := build/default.metallib
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

$(ASSET_DIR): | $(BUILD_DIR)
	mkdir -p $@
	cp -R assets/. $@/

build/%.air: include/%.metal | $(BUILD_DIR)
	xcrun -sdk macosx metal -c $< -o $@

build/%.air: shaders/%.metal | $(BUILD_DIR)
	xcrun -sdk macosx metal -c $< -o $@

build/default.metallib: $(METAL_AIR)
	xcrun -sdk macosx metallib $(METAL_AIR) -o $@

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