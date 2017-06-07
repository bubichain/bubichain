
noinst_LIBRARIES = libbubi_glue.a

libbubi_glue_a_SOURCES =	\
							glue_manager.cpp \
							txset_fetcher.cpp \
							consensus_receiver.cpp \
							transaction_set_frm.cpp \
							$(NULL)

libbubi_glue_a_CXXFLAGS = -std=c++11 -DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STL_ \
							-I$(top_srcdir)/src \
							-I$(top_builddir)/src/3rd/linux/include \
							 $(NULL)

