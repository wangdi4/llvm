# setenv LD_LIBRARY_PATH .../trunk/install/RH64/Release/bin/
# setenv PATH ${LD_LIBRARY_PATH}:${PATH}
# setenv DUMPIR 1
# setenv DUMPASM 1
setenv OUTDIR ./trunk
mkdir $OUTDIR
cd $OUTDIR
find .. -name '*.xml' -exec ../satest_wrap.sh {} \; >& run.log
cd -
