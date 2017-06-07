# A general java project makefile

# Set the file name of your jar package:
JAR_PKG = benchmark.jar
# Set your entry point of your java app:
ENTRY_POINT = benchmark.benchmark
RES_DIR = no
MAIN_DIR = src/benchmark
JAR_DIR = lib/BlockChainAdapter.jar:lib/bubi-common-0.0.1.RELEASE.jar:lib/commons-codec-1.10.jar:lib/eddsa-0.1.0.jar:lib/fastjson-1.2.7.jar:lib/Java-WebSocket-1.3.0.jar:lib/protobuf-java-3.3.0.jar:lib/slf4j-api-1.7.9.jar

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

all:build jar

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
else
	jar cvfe $(JAR_PKG) $(ENTRY_POINT) -C bin .
endif
