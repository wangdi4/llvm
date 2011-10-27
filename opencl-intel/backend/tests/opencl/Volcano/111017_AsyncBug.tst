RUN: python %S/../test_deploy.py %s.in .
RUN: python %S/../test_deploy.py %s.ll .
RUN: SATest -OCL -VAL -config=%s.cfg -basedir=.
