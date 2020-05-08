LIBS=-lgcrypt -lbluetooth
TARGET=ctd
SRCS=crypto.cpp bt.cpp log.cpp
OBJS=$(SRCS:.cpp=.o)
CPP=g++
CFLAGS=-std=c++17 

release: $(TARGET)

debug: CFLAGS += -g -DDEBUG_ADDR
debug: $(TARGET)

ctd: $(OBJS) main.o
	$(CPP) $(CFLAGS) -o $@ $^ $(LIBS)

en-gen: $(OBJS) en-gen.o
	$(CPP) $(CFLAGS) -o $@ $^ $(LIBS)

.cpp.o:
	$(CPP) $(CFLAGS) $(INCLUDES) -c $<  -o $@


clean:
	$(RM) *.o $(TARGET)


