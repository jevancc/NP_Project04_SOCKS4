CXX = clang++
CFLAGS = -std=c++11 -Wall -pedantic -pthread -lboost_system -lboost_filesystem
INCLUDES = \
	-Isrc/include \
	-Ilib/fmt/include

.PHONY: default all run
default: all

OUT := ./build
SRC := ./src
LIB := ./lib
OBJ := $(OUT)/obj
EXE := $(OUT)/socks_server
ALL := \
	$(wildcard $(LIB)/fmt/src/*.cc) \
	$(patsubst $(SRC)/%.cc,$(OBJ)/%.o,$(wildcard $(SRC)/*.cc))
ALL_EXE := $(ALL_EXE) $(EXE)

$(OBJ)/%.o: $(SRC)/%.cc $(shell find ./src/include/ -name "*.h")
	@mkdir -p $(OBJ)
	$(CXX) $(INCLUDES) -c $< -o $@ $(CFLAGS)

$(EXE): $(ALL)
	$(CXX) $(INCLUDES) $^ -o  $@ $(CFLAGS)
	@chmod 775 $@

all: $(ALL_EXE)

run: all
	echo $(ALL_EXE)
	cd build; ./socks_server 8080
