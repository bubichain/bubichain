LIBZMQ = $(top_builddir)/src/3rd/linux/lib/libzmq.a
JSONCPP = $(top_builddir)/src/3rd/linux/lib/libjson.a
LIBSSL = $(top_builddir)/src/3rd/linux/lib/libssl.a
LIBCRYPTO = $(top_builddir)/src/3rd/linux/lib/libcrypto.a
CURL = $(top_builddir)/src/3rd/linux/lib/libcurl.a
LIBZ = $(top_builddir)/src/3rd/linux/lib/libz.a
LIBBZ2 = $(top_builddir)/src/3rd/linux/lib/libbz2.a

bin_PROGRAMS = bubi_monitor

bubi_monitor_SOURCES = \
				main.cpp \
				monitor.cpp \
				alert.cpp \
				notice.cpp \
				upgrade.cpp \
				monitor_manager.cpp \
				processmessage.cpp
				
				

bubi_monitor_CXXFLAGS = -std=c++11 -DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STL_ \
					-I$(top_srcdir)/src \
					-I$(top_builddir)/src/3rd/linux/include \
					 $(NULL)

bubi_monitor_LDFLAGS =-static -static-libstdc++ -static-libgcc 

bubi_monitor_LDADD =	\
				$(top_builddir)/src/common/libbubi_common.a \
				$(top_builddir)/src/utils/libbubi_utils.a \
				$(LIBZMQ) \
				$(LIBSSL) \
				$(CURL) \
				$(LIBCRYPTO) \
				$(JSONCPP) \
				$(LIBZ) \
				$(LIBBZ2) \
				-lpthread -lidn -lrt -ldl \
				$(NULL)
