bin_PROGRAMS = bubi_slaved
bubi_slaved_SOURCES = main.cpp
bubi_slaved_CXXFLAGS = -std=c++11 \
					  $(NULL)
bubi_slaved_LDADD =	\
				$(top_builddir)/src/3rd/linux/lib/libssl.a \
				$(top_builddir)/src/3rd/linux/lib/libcrypto.a \
				-lpthread -ldl \
				$(NULL)
					
