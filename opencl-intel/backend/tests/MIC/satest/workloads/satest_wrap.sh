echo Command: SATest -BUILD -cpuarch knc -config $1
export TMP=`mktemp -d`
cp *.h ${TMP}/
cd ${TMP} && SATest -BUILD -cpuarch knc -config $1
mv SATest.s $2/`basename $1`.s
mv SATest.ll $2/`basename $1`.ll
rm -f *
cd -
rmdir ${TMP}
