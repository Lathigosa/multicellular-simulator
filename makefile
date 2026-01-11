#------------------------------------------------------------------------------#
# Universal Linux Makefile with automatic GPU/OpenCL detection                 #
# Fedora + Debian/Ubuntu compatible                                            #
#------------------------------------------------------------------------------#

# Detect number of cores
NPROC := $(shell nproc)

# Force make to use multiple jobs (GNU Make 4.3+ supports MAKEFLAGS)
MAKEFLAGS += -j$(NPROC)

# Tell Make to search all directories for .cpp files
vpath %.cpp $(shell find . -type d)

# Compiler & tools
CXX      := g++
LD       := g++
RM       := rm -rf

# Build directories
BUILD_DIR      := build
OBJ_DIR_DEBUG  := $(BUILD_DIR)/obj/debug
OBJ_DIR_RELEASE:= $(BUILD_DIR)/obj/release
BIN_DIR_DEBUG  := $(BUILD_DIR)/bin/debug
BIN_DIR_RELEASE:= $(BUILD_DIR)/bin/release

SHARE_DIR := share
SHARE_DEST_DEBUG   := $(BIN_DIR_DEBUG)/share
SHARE_DEST_RELEASE := $(BIN_DIR_RELEASE)/share

# Source files
SRCS := $(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard */*/*.cpp) $(wildcard */*/*/*.cpp)
OBJS_DEBUG  := $(patsubst %.cpp,$(OBJ_DIR_DEBUG)/%.o,$(SRCS))
OBJS_RELEASE:= $(patsubst %.cpp,$(OBJ_DIR_RELEASE)/%.o,$(SRCS))

#------------------------------------------------------------------------------#
# Required pkg-config packages                                                 #
#------------------------------------------------------------------------------#
PKG_REQ := gtkmm-3.0 epoxy luajit

#------------------------------------------------------------------------------#
# Compiler & linker flags                                                      #
#------------------------------------------------------------------------------#

# Get includes/libs via pkg-config
PKG_INC  := $(shell pkg-config --cflags $(PKG_REQ))
PKG_LIBS := $(shell pkg-config --libs $(PKG_REQ))

# Python / pybind11
PYBIND_INC := $(shell python3 -m pybind11 --includes 2>/dev/null || echo "")
PYTHON_INC := $(shell python3-config --includes)
PYTHON_LDFLAGS := $(shell python3-config --ldflags --embed)

# Automatic OpenCL detection
OPENCL_INC  := $(shell pkg-config --cflags OpenCL 2>/dev/null)
OPENCL_LIBS := $(shell pkg-config --libs OpenCL 2>/dev/null)

ifeq ($(OPENCL_INC),)
OPENCL_INC  := -I/usr/include/CL
endif

ifeq ($(OPENCL_LIBS),)
ifeq ($(wildcard /usr/lib/x86_64-linux-gnu/libOpenCL.so),/usr/lib/x86_64-linux-gnu/libOpenCL.so)
OPENCL_LIBS := -L/usr/lib/x86_64-linux-gnu -lOpenCL
else ifeq ($(wildcard /usr/lib64/libOpenCL.so),/usr/lib64/libOpenCL.so)
OPENCL_LIBS := -L/usr/lib64 -lOpenCL
else
$(error OpenCL library not found. Install ocl-icd-opencl-dev or ocl-icd-devel)
endif
endif

# Base compiler flags
#  -Wall -Wextra -pedantic
CXXFLAGS_BASE := -std=c++23 -Wno-c++26-extensions
CXXFLAGS_DEBUG   := $(CXXFLAGS_BASE) -g -D_DEBUG $(PKG_INC) $(PYBIND_INC) $(PYTHON_INC) $(OPENCL_INC)
CXXFLAGS_RELEASE := $(CXXFLAGS_BASE) -O2 $(PKG_INC) $(PYBIND_INC) $(PYTHON_INC) $(OPENCL_INC)

# Linker flags
LDFLAGS_DEBUG   := $(PKG_LIBS) -lGL $(PYTHON_LDFLAGS) $(OPENCL_LIBS) -Wl,--export-dynamic
LDFLAGS_RELEASE := $(PKG_LIBS) -lGL $(PYTHON_LDFLAGS) $(OPENCL_LIBS) -s

# Output binaries
OUT_DEBUG   := $(BIN_DIR_DEBUG)/gtk3_cell_simulator
OUT_RELEASE := $(BIN_DIR_RELEASE)/gtk3_cell_simulator

# Include directories
INCLUDE_DIRS := $(shell find src -type d)
CXXFLAGS_DEBUG   += $(addprefix -I,$(INCLUDE_DIRS))
CXXFLAGS_RELEASE += $(addprefix -I,$(INCLUDE_DIRS))

#------------------------------------------------------------------------------#
# Phony targets                                                                #
#------------------------------------------------------------------------------#
.PHONY: all debug release compile_debug compile_release link_debug link_release clean clean_debug clean_release check_deps copy_share_debug copy_share_release

# Default target
all: check_deps debug release

# Check for required packages
check_deps:
	@echo "Checking required pkg-config packages..."
	@for pkg in $(PKG_REQ); do \
	    pkg-config --exists $$pkg || { echo "Error: missing pkg-config package: $$pkg"; exit 1; }; \
	done
	@echo "All required packages found."

#------------------------------------------------------------------------------#
# Build directories                                                            #
#------------------------------------------------------------------------------#
$(OBJ_DIR_DEBUG) $(OBJ_DIR_RELEASE) $(BIN_DIR_DEBUG) $(BIN_DIR_RELEASE):
	mkdir -p $@

debug: compile_debug link_debug copy_share_debug
release: compile_release link_release copy_share_release

compile_debug: $(OBJS_DEBUG)
compile_release: $(OBJS_RELEASE)

link_debug: $(OUT_DEBUG)
link_release: $(OUT_RELEASE)

#------------------------------------------------------------------------------#
# Compile rules                                                                #
#------------------------------------------------------------------------------#
$(OBJ_DIR_DEBUG)/%.o: %.cpp | $(OBJ_DIR_DEBUG)
	@echo "[ G++ ] Compiling $(notdir $<)."
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS_DEBUG) -MMD -MP -c $< -o $@

$(OBJ_DIR_RELEASE)/%.o: %.cpp | $(OBJ_DIR_RELEASE)
	@echo "[ G++ ] Compiling $(notdir $<)."
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS_RELEASE) -MMD -MP -c $< -o $@

#------------------------------------------------------------------------------#
# Link rules                                                                   #
#------------------------------------------------------------------------------#
$(OUT_DEBUG): $(OBJS_DEBUG) | $(BIN_DIR_DEBUG)
	@echo "Linking $@."
	@$(LD) -o $@ $^ $(LDFLAGS_DEBUG)

$(OUT_RELEASE): $(OBJS_RELEASE) | $(BIN_DIR_RELEASE)
	@echo "Linking $@."
	@$(LD) -o $@ $^ $(LDFLAGS_RELEASE)

#------------------------------------------------------------------------------#
# Copy share folder after linking                                              #
#------------------------------------------------------------------------------#

copy_share_debug: $(OUT_DEBUG)
	@echo "Copying $(SHARE_DIR) to $(BIN_DIR_DEBUG)"
	@mkdir -p $(SHARE_DEST_DEBUG)
	@cp -r $(SHARE_DIR)/* $(SHARE_DEST_DEBUG)/

copy_share_release: $(OUT_RELEASE)
	@echo "Copying $(SHARE_DIR) to $(BIN_DIR_RELEASE)"
	@mkdir -p $(SHARE_DEST_RELEASE)
	@cp -r $(SHARE_DIR)/* $(SHARE_DEST_RELEASE)/


#------------------------------------------------------------------------------#
# Clean rules                                                                  #
#------------------------------------------------------------------------------#
clean: clean_debug clean_release

clean_debug:
	$(RM) $(OBJ_DIR_DEBUG) $(BIN_DIR_DEBUG)

clean_release:
	$(RM) $(OBJ_DIR_RELEASE) $(BIN_DIR_RELEASE)

#------------------------------------------------------------------------------#
# Include dependency files                                                     #
#------------------------------------------------------------------------------#
-include $(OBJS_DEBUG:.o=.d)
-include $(OBJS_RELEASE:.o=.d)