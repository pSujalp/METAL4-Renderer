COMPILER := xcrun
SDK := -sdk macosx

METAL := $(COMPILER) $(SDK) metal
METALLIB := $(COMPILER) $(SDK) metallib

# Metal source files
METAL_SRC := $(wildcard shaders/*.metal)
METAL_SRC += $(wildcard include/*.metal)

# Corresponding AIR files
AIR_FILES := $(METAL_SRC:.metal=.air)

.PHONY: all clean

all: shader.metallib

# Compile shaders/*.metal
shaders/%.air: shaders/%.metal
	@echo "Compiling $<"
	$(METAL) -c $< -o $@

# Compile include/*.metal
include/%.air: include/%.metal
	@echo "Compiling $<"
	$(METAL) -c $< -o $@

# Create metallib
shader.metallib: $(AIR_FILES)
	@echo "Creating $@"
	$(METALLIB) $^ -o $@

clean:
	rm -f shaders/*.air include/*.air shader.metallib