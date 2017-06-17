# This is makefile for kcpnet
CC  = /usr/bin/gcc
CXX = /usr/bin/g++
AR  = ar
RANLIB = ranlib

CCOBJS = ikcp.o
CCFLAGS=

CXXOBJS = 
CXXFLAGS=

TARGET = libkcpnet.a

all: $(TARGET) 

$(CCOBJS): %.o: %.c
	$(CC) -c $(CCFLAGS) $< -o $@

$(OBJS): %.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@
	
$(TARGET): $(CCOBJS) $(CXXOBJS)
	@rm -f $(TARGET)
	@$(AR) rc $(TARGET) $(CCOBJS) $(CXXOBJS) $(LDFLAGS)
	@$(RANLIB) $(TARGET)

clean:
	rm -f *.o
	rm -f ${TARGET}
