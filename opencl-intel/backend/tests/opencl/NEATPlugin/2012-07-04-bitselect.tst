; RUN: SATest -OCL -VAL -tsize=0 -config=%s.cfg --neat=1 --single_wg=1 --force_ref=0 -build-iterations=1 -execute-iterations=1
; XFAIL: