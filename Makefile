CC=cc
CFLAGS=-DUNIX -I.
DEPS=
COPTS=

OBJ=obj

OBJS=$(OBJ)/LIB_vepQList.o $(OBJ)/LIB_vepQUte.o $(OBJ)/LIB_vepQStr.o 

LIBS=

TARGET=test_qstr

target: $(TARGET)

$(OBJ)/libqstruct.a: $(OBJS)
	echo Building $@
	# cd $(OBJ)
	ar -r $@ $(OBJS)

%.o: %.c $(DEPS)
	$(CC) -c $(COPTS) -o $@ $< $(CFLAGS)

test_qstr: test_qstr.o vepQStr.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

run:
	@list_test

clean:
	rm -f *.o
	rm -f *.a 

clean-all: clean
	rm $(TARGET)
