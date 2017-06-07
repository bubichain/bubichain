
noinst_LIBRARIES = libbubi_ledger.a

libbubi_ledger_a_SOURCES =	account.cpp		\
							ledger_fetch.cpp \
							ledger_frm.cpp \
							ledger_manager.cpp \
							issue_asset_ope_frm.cpp \
							issue_unique_asset_ope_frm.cpp \
							operation_frm.cpp \
							payment_ope_frm.cpp \
							payment_unique_asset_ope_frm.cpp \
							record_ope_frm.cpp \
							setoptions_ope_frm.cpp \
							transaction_frm.cpp \
							init_payment_ope_frm.cpp \
							production_ope_frm.cpp \
							createaccount_ope_frm.cpp \
							account_tree.cpp \
							$(NULL)

libbubi_ledger_a_CXXFLAGS = -std=c++11 -DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STL_ \
							-I$(top_srcdir)/src \
							-I$(top_srcdir)/src/3rd/linux/include \
							 $(NULL)

