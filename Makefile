CC=g++
CFLAGS=-Isrc -std=c++11
LIBS=-lm -llo

DEPS = src/tides_sensor_osc.cpp

OBJ = obj/tides_sensor_osc.o

obj/%.o: $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bin/tides_sensor_osc: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f obj/*.o *~ src/*~
