TEST_DIR="$( cd "$( dirname "$0"  )" && pwd  )"

#start
echo start 4peers-without-slave
$TEST_DIR/env/4peers-without-slave/start.sh
sleep 30

#run testcases
cd $TEST_DIR/testcase/testng/InterfaceTest-1.8 && ant;



mv -f $TEST_DIR/testcase/testng/InterfaceTest-1.8/test-output/TestNG/Report.zip $TEST_DIR/testcase/testng/InterfaceTest-1.8/test-output/reports/Report-4peers-without-slave.zip

cd $TEST_DIR/testcase/testng/InterfaceTest-1.8 && ant -buildfile buildzmq.xml;

cd $TEST_DIR;


#start 4th peer to test sync
$TEST_DIR/env/4peers-without-slave/peer3-without-slave/start.sh
sleep 1800

#stop
$TEST_DIR/env/4peers-without-slave/stop.sh
sleep 3