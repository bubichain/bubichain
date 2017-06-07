TEST_DIR="$( cd "$( dirname "$0"  )" && pwd  )"

#start
echo start 4peers-with-slave
$TEST_DIR/env/4peers-with-slave/start.sh
sleep 30

#run testcases
cd $TEST_DIR/testcase/testng/InterfaceTest-1.8 && ant -buildfile build2peer.xml;
mv -f $TEST_DIR/testcase/testng/InterfaceTest-1.8/test-output/TestNG/Report.zip $TEST_DIR/testcase/testng/InterfaceTest-1.8/test-output/reports/Report-4peers-with-slave.zip
cd $TEST_DIR;

#stop
$TEST_DIR/env/4peers-with-slave/stop.sh
sleep 3