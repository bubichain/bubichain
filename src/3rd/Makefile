INSTALL_PATH = ${PWD}/linux

ROCKSDB = $(INSTALL_PATH)/lib/librocksdb.a
PROTOBUF = $(INSTALL_PATH)/lib/libprotobuf.a
LIBSSL = $(INSTALL_PATH)/lib/libssl.a
LIBBZ2 = $(INSTALL_PATH)/lib/libbz2.a
LIBZ = $(INSTALL_PATH)/lib/libz.a
JSONCPP = $(INSTALL_PATH)/lib/libjson.a
LIBZMQ = $(INSTALL_PATH)/lib/libzmq.a
POSTGRE = $(INSTALL_PATH)/lib/libpq.a
CURL = $(INSTALL_PATH)/lib/libcurl.a
ED25519_DONNA = $(INSTALL_PATH)/lib/libed25519_donna.a
LIBHTTP = $(INSTALL_PATH)/lib/libbubi_http.a
#default libraries target list
all: \
    $(ROCKSDB) \
    $(PROTOBUF) \
    $(LIBSSL) \
    $(LIBBZ2) \
    $(LIBZ) \
    $(JSONCPP) \
    $(LIBZMQ) \
    $(POSTGRE) \
    $(ED25519_DONNA) \
    $(LIBHTTP) \
	$(CURL) 

#sub target 
$(ROCKSDB):
	@echo "*************building rocksdb now*************" && \
	cd src/rocksdb && tar xvf rocksdb-*.tar.gz && cd rocksdb-* && \
	$(MAKE) static_lib && INSTALL_PATH=$(INSTALL_PATH) $(MAKE) install-static
	
$(PROTOBUF) :
	@echo "*************building protobuf now***************" && \
	cd src/protobuf && tar xvf protobuf-*.tar.gz && cd protobuf-* && \
	chmod +x autogen.sh && ./autogen.sh && chmod +x configure && \
	./configure --prefix=$(INSTALL_PATH) && $(MAKE) && $(MAKE) install
	
$(LIBSSL) :
	@echo "*************building openssl now ***************" && \
	cd src/openssl && tar xvf openssl-*.tar.gz && cd openssl-* && chmod +x config && \
	./config --prefix=$(INSTALL_PATH)  && $(MAKE) && $(MAKE) install_sw
	
$(LIBBZ2) :
	@echo "*************building BZIP2 now ***************" && \
	cd src/bzip2 && tar xvf bzip2-*.tar.gz && cd bzip2-* && \
	$(MAKE) && $(MAKE) install PREFIX=$(INSTALL_PATH)
	
$(LIBZ) :
	@echo "*************building LIBZ now ***************" && \
	cd src/zlib && tar xvf zlib-*.tar.gz && cd zlib-* && chmod +x configure && \
	./configure --prefix=$(INSTALL_PATH) && $(MAKE) && $(MAKE) install
	
$(JSONCPP) :
	@echo "*************building jsoncpp now***************" && \
	cd src/jsoncpp && chmod 777 automake.sh && ./automake.sh && \
	cp -rf include/* $(INSTALL_PATH)/include && cp -rf libs/*.a $(INSTALL_PATH)/lib
	
$(LIBZMQ) :
	@echo "*************building LIBZMQ now ***************" && \
	cd src/zeromq && tar xvf zeromq-*.tar.gz && cd zeromq-* && \
	chmod +x configure version.sh && \
	./configure --prefix=$(INSTALL_PATH) && $(MAKE) && $(MAKE) install
	
$(POSTGRE) :
	@echo "*************building POSTGRE now***************" && \
	cd src/postgresql && tar xvf postgresql-*.tar.gz && cd postgresql-* && chmod +x configure && \
	./configure --without-readline --prefix=$(INSTALL_PATH) && $(MAKE) && $(MAKE) install
	
$(CURL):
	@echo "**************building curl now***************" && \
	cd src/curl && tar xvf curl-*.tar.gz && cd curl-* && chmod +x configure && \
	./configure --disable-ldap --disable-ldaps --without-librtmp --with-ssl=$(INSTALL_PATH) --prefix=$(INSTALL_PATH) && $(MAKE) && $(MAKE) install
	
$(ED25519_DONNA):
	@echo "**************building ed25519 now***************" && \
	cd src/ed25519-donna && unzip -o ed25519-donna-*.zip && cd ed25519-donna-* && \
	gcc ed25519.c -c -o ed25519.o && ar rcs libed25519_donna.a ed25519.o && \
	cp -f libed25519_donna.a $(INSTALL_PATH)/lib && \
	mkdir -p $(INSTALL_PATH)/include/ed25519-donna && cp *.h $(INSTALL_PATH)/include/ed25519-donna
	
$(LIBHTTP):
	@echo "**************building http now***************" && \
	cd src/http && $(MAKE) && INSTALL_PATH=$(INSTALL_PATH) $(MAKE) install

clean:
	@echo "******************skip clean 3rd, you can use 'clean_3rd' if you need******************"
	
clean_3rd: \
    ROCKSDB_clean \
    PROTOBUF_clean \
    LIBSSL_clean \
    LIBCRYPTO_clean \
    LIBBZ2_clean \
    LIBZ_clean \
    LIBZMQ_clean \
    POSTGRE_clean \
    JSONCPP_clean \
	CURL_clean \
	ED25519_DONNA_clean \
	LIBHTTP_clean 
    
install:
	@echo "No library to install for 3rd"
    
ROCKSDB_clean:
	@echo "******************Cleaning rocksdb now*************" && cd $(ROCKSDB_PATH) && make clean
PROTOBUF_clean:
	@echo "******************Cleaning rocksdb now*************" && cd $(PROTOBUF_PATH) && make clean
LIBSSL_clean:
	@echo "******************Cleaning rocksdb now*************" && cd $(LIBSSL_PATH) && make clean
LIBCRYPTO_clean:
	@echo "******************Cleaning rocksdb now*************" && cd $(LIBSSL_PATH) && make clean
LIBBZ2_clean:
	@echo "******************Cleaning rocksdb now*************" && cd $(LIBBZ2_PATH) && make clean
LIBZ_clean:
	@echo "******************Cleaning rocksdb now*************" && cd $(LIBZ_PATH) && make clean
LIBZMQ_clean:
	@echo "******************Cleaning rocksdb now*************" && cd $(LIBZMQ_PATH) && make clean
POSTGRE_clean:
	@echo "******************Cleaning rocksdb now*************" && cd $(POSTGRE_PATH) && make clean
CURL_clean:
	@echo "******************Cleaning curl now*************" && cd $(CURL_PATH) && make clean
ED25519_DONNA_clean:
	@echo "******************Cleaning ed25519 now*************" && cd $(ED25519_DONNA_PATH) && make clean
LIBHTTP_clean:
	@echo "******************Cleaning http now*************" && cd $(LIBHTTP_PATH) && make clean

#scon -c may complains error
JSONCPP_clean: 
	@echo "******************Cleaning rocksdb now*************" && cd $(JSONCPP_PATH) && ./automake.sh -c || true   
    
