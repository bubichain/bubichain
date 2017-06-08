# A general java project makefile

# Set the file name of your jar package:
JAR_PKG = BlockChainAdapter.jar
ENTRY_POINT = cn.bubi.BlockChainAdapter
RES_DIR = no

MAIN_DIR = src/cn/bubi/blockchain/adapter
JAR_DIR = ../3rd/linux/lib/protobuf-java-3.3.0.jar:lib/Java-WebSocket-1.3.0.jar:lib/slf4j-api-1.7.9.jar

# Set your java compiler here:
JAVA_BIN = ../3rd/jdk/bin

JFLAGS = -encoding UTF-8 

vpath %.class bin
vpath %.java src

# show help message by default
Default:
	@echo "make new: new project, create src, bin, res dirs."
	@echo "make build: build project."
	@echo "make clean: clear classes generated."
	@echo "make rebuild: rebuild project."
	@echo "make run: run your app."
	@echo "make jar: package your project into a executable jar."

build: 
	javac -classpath $(JAR_DIR):bin	-sourcepath $(MAIN_DIR) -d bin $(MAIN_DIR)/*.java $(JFLAGS)

all:new build jar

rebuild: clean build

.PHONY: new clean run jar

new:
ifeq ($(RES_DIR),yes)
	mkdir -pv src bin res
else
	mkdir -pv src bin
endif

clean:
	rm -frv bin/*

run:
	java -cp bin $(ENTRY_POINT)

jar:
ifeq ($(RES_DIR),yes)
	jar cvfe $(JAR_PKG) $(ENTRY_POINT)  -C bin . res
	cp BlockChainAdapter.jar ../../test/benchmark/lib
else
	jar cvfe $(JAR_PKG) $(ENTRY_POINT) -C bin .
	cp BlockChainAdapter.jar ../../test/benchmark/lib
endif
