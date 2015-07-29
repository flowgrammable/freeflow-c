
# GCC:
CC_GCC=gcc
GCC_FLAGS=-O2 -Wall
GCC_SO_FLAGS=-shared
#If need to expose all symbols, use: -rdynamic

# LLVM CLANG:
#CC_CLANG=./llvm-riscv-toolchain/bin/clang
CC_CLANG=clang
CLANG_FLAGS=-O2 -Wall -S -emit-llvm

# LLVM LLC:
#CC_LLC=./llvm-riscv-toolchain/bin/llc
CC_LLC=llc
LLC_FLAGS=-O2 -relocation-model=pic -filetype=obj
# Uncomment to override arch during JIT compile step
#LLC_TARGET=-march=ppc32

DL_LIB=-ldl

# Point to Data Plane General Types:
DP_SRC=../../

STAGES=x-connect
STAGE_DEPS=key.h util.h

# Quick test performs each on this machine:
all: stage-ir stage-lib

# Steps 1-3 are inteded to be run seperately on different machines.
# - Be sure to update paths to clang, llc, and gcc
# - Be sure to adjust LLC_TARGET on target machine to override triple in
#   the LLVM IR (*.ll) file.

# Step 1: Compile main normally with support for Dynamic Loading of Libs
#main: main-dl

# Step 2: Generate LLVM IR and transfer to target machine
stage-ir: $(STAGES:=.ll)

# Step 3: JIT compile LLVM IR to shared object library
stage-lib: $(STAGES:=.so)


main-dl: $(DP_DEPS)
	$(CC_GCC) $(GCC_FLAGS) main-dl.c -o main-dl $(DL_LIB)

# It would be nice to iterate through all stages in $(STAGES) and name
# appropriately.  Not sure how to do this atm..
%.so: %.o
	$(CC_GCC) $(GCC_SO_FLAGS) $< -o $@

%.o: %.ll
	$(CC_LLC) $(LLC_FLAGS) $(LLC_TARGET) $< -o $@

%.ll: %.c $(STAGE_DEPS)
	$(CC_CLANG) $(CLANG_FLAGS) -I$(DP_SRC) $< -o $@

clean:
	rm -f $(STAGES:=.o) $(STAGES:=.so) $(STAGES:=.ll)

