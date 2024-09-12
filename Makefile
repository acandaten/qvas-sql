CC=cc
POSTGRES_HOME=$(ECPG_HOME)
CFLAGS=-m32 -DPOSTGRES -DUNIX -I. -I/usr/include/postgresql -I$(POSTGRES_HOME)/include 
DEPS=
COPTS=
#CFLAG=-DPOSTGRES -I/usr/include/postgresql -m32

OBJ=obj

OBJS=$(OBJ)/LIB_vepQList.o $(OBJ)/LIB_vepQUte.o $(OBJ)/LIB_vepQStr.o 
LIBPOSTGRES=-L $(POSTGRES_HOME)/lib -L /usr/local/pgsql/lib -lpq -lpgtypes

LIBS=

TARGET=test_qstr

target: $(TARGET)

$(OBJ)/libqstruct.a: $(OBJS)
	echo Building $@
	# cd $(OBJ)
	ar -r $@ $(OBJS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $(COPTS) -o $@ $<

test_qstr: test_qstr.o vepQStr.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

qsql: qsql.o vepQStr.o 
	gcc $(CFLAGS) $(LIBPOSTGRES) -o $@ $^


run:
	@list_test

clean:
	rm -f *.o
	rm -f *.a 

clean-all: clean
	rm $(TARGET)
