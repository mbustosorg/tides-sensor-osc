CC=g++
CFLAGS=-Isrc -std=c++11 -pthread
LIBFLAGS=-pthread
LIBS=-llo
SRC_DIR=./src/
OBJ_DIR=./obj/

DEPS_FILES=tides_sensor_osc.cpp tides_data.cpp tides_display_model.cpp
DEPS=$(addprefix $(SRC_DIR),$(DEPS_FILES))
OBJ=$(addprefix $(OBJ_DIR),$(DEPS_FILES:.cpp=.o))

$(OBJ_DIR)%.o : $(SRC_DIR)%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

bin/tides_sensor_osc: $(OBJ)
	$(CC) -o $@ $^ $(LIBFLAGS) $(LIBS)

.PHONY: clean

clean:
	mkdir -p obj
	mkdir -p bin
	rm -f obj/*.o *~ src/*~ bin/*
