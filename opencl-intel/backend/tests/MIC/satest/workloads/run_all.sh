# setenv LD_LIBRARY_PATH .../trunk/install/RH64/Release/bin/
# setenv PATH ${LD_LIBRARY_PATH}:${PATH}
setenv DUMPIR 1
setenv DUMPASM 1
setenv OUTDIR ./trunk-new-kernels
mkdir $OUTDIR
find . -maxdepth 1 -name '*.xml' -exec satest_wrap.sh {} $OUTDIR \; >& ${OUTDIR}/run.log
