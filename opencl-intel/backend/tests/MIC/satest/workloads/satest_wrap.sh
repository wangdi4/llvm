echo Command: SATest -BUILD -cpuarch knc -config $1
SATest -BUILD -cpuarch knc -config $1
mv SATest.s $2/`basename $1`.s
mv SATest.ll $2/`basename $1`.ll
