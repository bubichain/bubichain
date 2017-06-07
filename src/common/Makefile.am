noinst_LIBRARIES = libbubi_common.a
libbubi_common_a_CXXFLAGS =  				-std=c++11 -DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STL_ \
							-I$(top_srcdir)/src/3rd/linux/include \
							-I$(top_srcdir)/src \
							$(NULL)

libbubi_common_a_SOURCES = configure.cpp \
						general.cpp \
						storage.cpp \
						private_key.cpp \
						daemon.cpp \
						ecc_sm2.cpp \
						pipeline_server.cpp \
						network.cpp
							
noinst_HEADERS = configure.h general.h storage.h private_key.h ecc_sm2.h pipeline_server.h

