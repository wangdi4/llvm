RUN: python %S/../test_deploy.py %s.in .
RUN: python %S/../test_deploy.py %s.cl .
RUN: SATest -OCL -BUILD -config=%s.cfg -basedir=. -cpuarch="Unsupported_CPU_ARCH"
XFAIL: