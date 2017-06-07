bin_PROGRAMS = bubid
bubid_SOURCES = main.cpp
bubid_CXXFLAGS = -std=c++11 \
					  $(NULL)
bubid_LDADD =	$(top_builddir)/src/3rd/linux/lib/libssl.a \
				$(top_builddir)/src/3rd/linux/lib/libcrypto.a \
				-lpthread -ldl \
				$(NULL)
					
