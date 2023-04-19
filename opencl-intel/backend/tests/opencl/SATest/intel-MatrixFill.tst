RUN: SATest -BUILD -config=%s.cfg -cpuarch=sapphirerapids 2>&1 | FileCheck %s

; CHECK: Test program was successfully built.
