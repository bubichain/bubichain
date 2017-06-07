make clean;
find -name "*.gcda" -exec rm \{\} \; 
find -name "*.gcno" -exec rm \{\} \;
find -name "app.info" -exec rm \{\} \;

make CXXFLAGS="-ftest-coverage -fprofile-arcs"  LDFLAGS=-fprofile-arcs;