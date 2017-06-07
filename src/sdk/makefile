# A general java project makefile
# Author: Wu Jiqing (jiqingwu@gmail.com)
# create: 2012-06-12
# update: 2012-06-13
# version: 0.7

# 设置你要生成的jar包的文件名
# Set the file name of your jar package:
JAR_PKG = BlockChainAdapter.jar
# 设置你的项目的入口点
# Set your entry point of your java app:
ENTRY_POINT = cn.bubi.BlockChainAdapter
# 是否需要res目录，如果你的程序有图片、文档等，
# 最好放入res目录中。
# yes: 需要；no：不需要
RES_DIR = no
# 设置你项目包含的源文件
SUB_DIR = src/org/zeromq
MAIN_DIR = src/cn/bubi/blockchain/adapter
JAR_DIR = ../3rd/linux/lib/protobuf-java-3.3.0.jar:lib/Java-WebSocket-1.3.0.jar:lib/slf4j-api-1.7.9.jar

# 设置你的java编译器
# Set your java compiler here:
JAVA_BIN = ../3rd/jdk/bin
# 设置你的编译选项
JFLAGS = -encoding UTF-8 
#############下面的内容建议不要修改####################

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
	cp BlockChainAdapter.jar ../../test/benchmark/lib
else
	jar cvfe $(JAR_PKG) $(ENTRY_POINT) -C bin .
	cp BlockChainAdapter.jar ../../test/benchmark/lib
endif
