LIBS=-lgcrypt -lbluetooth
TARGET=ctd
SRCS=ct_crypto.cpp ct_beacon.cpp main.cpp
OBJS=$(SRCS:.cpp=.o)
CPP=g++
CFLAGS=-std=c++17 

release: $(TARGET)

debug: CFLAGS += -g -DDEBUG_ADDR
debug: $(TARGET)

$(TARGET): $(OBJS)
	$(CPP) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CPP) $(CFLAGS) $(INCLUDES) -c $<  -o $@


clean:
	$(RM) *.o $(TARGET)


