TARGET = libbubi_http.a

CXX ?= g++

CXXFLAGS += -std=c++11 -DASIO_STANDALONE -O2

OBJS = \
    connection.o \
    connection_manager.o \
    HttpClient.o \
    reply.o \
    request_parser.o \
    server.o \
    io_service_pool.o

INCS = -I ../../linux/include -I ../../../

LIBS =  -L ../../linux/lib/

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

$(TARGET): $(OBJS)
	ar rcs $@ $(OBJS)

install:
	cp $(TARGET) $(INSTALL_PATH)/lib/
	mkdir -p $(INSTALL_PATH)/include/http
	cp *.hpp $(INSTALL_PATH)/include/http
	

clean: 
	rm -rf *.o *.so $(OBJS) $(TARGET) 


