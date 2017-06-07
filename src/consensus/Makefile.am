
#$(PROTOBUF) :
#	@echo "*************building protobuf now***************" && cd $(PROTOBUF_PATH) && ./autogen.sh && ./configure --prefix=/usr && $(MAKE) && $(MAKE) install

noinst_LIBRARIES = libbubi_consensus.a

libbubi_consensus_a_SOURCES =	\
						  consensus_manager.cpp \
						  consensus_msg.cpp \
						  consensus.cpp \
						  pbft.cpp \
						  pbft_instance.cpp \
							$(NULL)

libbubi_consensus_a_CXXFLAGS = -std=c++11 -DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STL_ \
							-I$(top_srcdir)/src \
							-I$(top_builddir)/src/3rd/linux/include \
							 $(NULL)

