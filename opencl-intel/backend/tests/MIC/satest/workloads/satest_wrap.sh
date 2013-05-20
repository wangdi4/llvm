SATest -BUILD -cpuarch knc -config $1
mv SATest.s `basename $1`.s
mv SATest.ll `basename $1`.ll
