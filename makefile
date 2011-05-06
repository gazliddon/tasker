# Makefile to build the a simple tasker example
# And programs to test it

LIB_NAME := glib
ARCH :=	x86_64

EXECUTABLE = testexe

# Directories
OUT_DIR := out
INC_DIR := source

# Things to build
SRC := $(shell find . -type f -name '*.cpp' -o -name '*.s')
OBJS := $(foreach FILE, $(SRC), $(OUT_DIR)/$(basename $(FILE)).o)
DIRS := $(sort $(foreach FILE, $(OBJS), $(dir $(FILE))))


# Dependencies 
all: dirs exe
	@echo Made all

dirs: $(DIRS)
	@echo Built directories

clean:
	@rm -rf $(OUT_DIR) $(EXECUTABLE)
	@echo Cleaned all

exe: $(EXECUTABLE)
	@echo Built executable

# How to build things


#Make directories
$(DIRS):
	@mkdir -p $@

#Link
$(EXECUTABLE): $(OBJS)
	@echo Linking $@ 
	@g++ -o $@ -arch $(ARCH) $^

#Compile C++
$(OUT_DIR)/%.o: %.cpp
	@echo Compiling $< to $@
	@g++ -arch $(ARCH) -I $(INC_DIR)  -c $< -o $@

#Assemble
$(OUT_DIR)/%.o: %.s
	@echo Assembling $< to $@
	@cpp -undef -D__$(ARCH)__ -c++ $< | as -arch $(ARCH) -o $@

#end