LIBS=-lgcrypt -lbluetooth
TARGET=ctd
SRCS=ct_crypto.cpp ct_beacon.cpp main.cpp
OBJS=$(SRCS:.cpp=.o)
CPP=g++
CFLAGS=-std=c++17 -g

$(TARGET): $(OBJS)
	$(CPP) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CPP) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o $(TARGET)


