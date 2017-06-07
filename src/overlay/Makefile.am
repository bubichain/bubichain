noinst_LIBRARIES = libbubi_overlay.a

libbubi_overlay_a_SOURCES =	broadcast.cpp	\
							peer.cpp \
							peer_manager.cpp \
							peer_message.cpp \
							$(NULL)

libbubi_overlay_a_CXXFLAGS = -std=c++11 -DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STL_ \
							-I$(top_srcdir)/src \
							-I$(top_srcdir)/src/3rd/linux/include \
							 $(NULL)
