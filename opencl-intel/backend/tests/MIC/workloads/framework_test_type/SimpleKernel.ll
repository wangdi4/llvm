; XFAIL: *
; Notice: exepected to fail due to data races (should be ignored currently)
; RUN: python ../../bin/SATest.py -tsize=0 -neat -config=/Volcano/Tests/Workloads/framework_test_type/RH64/SimpleKernel.cfg
