CC=g++
EXECUTABLE=render
SRC_DIR=src
INTERM_DIR=obj

ifdef WIDTH
	RENDERPARAM=-DWIDTH=$(WIDTH) -DHEIGHT=$(HEIGHT)
else
	RENDERPARAM=
endif

INCLUDES=-I $(SRC_DIR)
LIBS=-lpng -stdc++
CFLAGS_COMMON=$(INCLUDES)
CFLAGS=$(CFLAGS_COMMON) -O3 -DNDEBUG $(RENDERPARAM)
#CFLAGS=$(CFLAGS_COMMON) -g -O0 -D_DEBUG -Wall

SOURCE_FILES=$(shell find $(SRC_DIR) -iname '*.cpp')
DEP_FILES=$(SOURCE_FILES:$(SRC_DIR)/%.cpp=./$(INTERM_DIR)/%.dep)
OBJ_FILES=$(SOURCE_FILES:$(SRC_DIR)/%.cpp=./$(INTERM_DIR)/%.o)

all: $(EXECUTABLE)

clean:
	rm -rf obj $(EXECUTABLE)

.PHONY: clean all

.SUFFIXES:
.SUFFIXES:.o .dep .cpp .h

$(INTERM_DIR)/%.dep: $(SRC_DIR)/%.cpp
	mkdir -p `dirname $@`
	echo -n `dirname $@`/ > $@
	$(CC) $(CFLAGS_COMMON) $< -MM | sed -r -e 's,^(.*)\.o\s*\:,\1.o $@ :,g' >> $@

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP_FILES)
endif

$(INTERM_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(INTERM_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $^ $(LIBS) -o $@
