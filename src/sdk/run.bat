javac -sourcepath ../../src/sdk/src/org/zeromq -d bin ../../src/sdk/src/org/zeromq/*.java -encoding UTF-8
javac -classpath protobuf-java-3.0.0.jar;bin -sourcepath ../../src/sdk/src/blockchain/adapter/BlockChainAdapter -d bin ../../src/sdk/src/cn/bubi/blockchain/adapter/*.java -encoding UTF-8
jar cvfe BlockChainAdapter-2.0.0.0.2221.jar BlockChainAdapter.BlockChainAdapter -C bin .