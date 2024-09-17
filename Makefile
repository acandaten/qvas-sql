BIN=bin
CC=gcc
POSTGRES_HOME=$(ECPG_HOME)
CFLAGS=-m32 -DPOSTGRES -DUNIX -I. -I/usr/include/postgresql -I$(POSTGRES_HOME)/include 
DEPS=
COPTS=
#CFLAG=-DPOSTGRES -I/usr/include/postgresql -m32

OBJS=vepQStr.o vepQList.o qsql_funs.o
LIBS=
LIBPOSTGRES=-L $(POSTGRES_HOME)/lib -L /usr/local/pgsql/lib -lpq -lpgtypes


TARGET=$(BIN)/test_qstr $(BIN)/test_qsql_funs $(BIN)/qsql

target: $(TARGET)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $(COPTS) -o $@ $<

$(BIN)/test_qsql_funs: test_qsql_funs.o $(OBJS)
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $^ 

$(BIN)/test_qstr: test_qstr.o vepQStr.o
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $^ 

$(BIN)/test_opt: test_opt.o
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $^ 

$(BIN)/qsql: qsql.o $(OBJS) 
	mkdir -p $(BIN)
	gcc $(CFLAGS) $(LIBPOSTGRES) -o $@ $^

run:
	@list_test

clean:
	rm -f *.o
	rm -f *.a 

clean-all: clean
	rm $(TARGET)
