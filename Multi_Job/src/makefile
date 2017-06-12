PUBLIC = ../include/
MYSQL = ../include/db/mysql/64/include/
TINYXML = ../include/tinyxml/
CURL = ../include/curl/inc64/
OPENSSL = ../include/openssl/inc/
INC = -I../include/ -I$(MYSQL) -I$(TINYXML) -I../include/db/ -I$(CURL) -I$(OPENSSL)
CC = g++ -g -Wall 
LIB = ../lib/libdbaccess.a ../lib/libmysqlclient.a ../lib/libtinyxml.a -L../include/curl/lib64/ -lcurl -L../include/openssl/lib64/ -lssl -lcrypto -ldl -lpthread -lrt -lz
BIN = Multi_Job
OBJ = mainsvrd.o \
master.o \
worker.o \
tlog.o \
httpserviceadapter.o

all: $(BIN)

$(BIN): ${OBJ}
	rm -f $@
	$(CC) -o $@ $(INC) $^ $(LIB)
	cp $(BIN) ../bin/
	chmod +x ../bin/*

%.o: %.cpp
	$(CC) $(INC) -c -o $@ $<

clean:
	rm -f *.o ${BIN}

cleanall:
	rm -f *.o ${BIN} ${OBJ}
	
