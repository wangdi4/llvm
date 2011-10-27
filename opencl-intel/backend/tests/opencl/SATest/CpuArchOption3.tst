RUN: python %S/../test_deploy.py %s.in .
RUN: python %S/../test_deploy.py %s.ll .
RUN: SATest -OCL -BUILD -config=%s.cfg -basedir=. -cpuarch="corei7"
XFAIL: