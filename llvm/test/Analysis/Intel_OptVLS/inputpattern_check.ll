; Test whether intelovls-test parses the input file successfully if
; the input file is written following the rules like below.
; intelovls-test fails to parse the input file if the input file does
; not follow the rules mentioned in the intelovls-test.cpp
; REQUIRES: asserts
; RUN: intelovls-test < %s -debug
;
# 16
1 A 0 i32 4 SLoad C 40
2 A 12 i32 4 SLoad C 40
3 A 4 i32 4 SLoad C 40
4 A 16 i32 4 SLoad C 40
5 A 20 i32 4 SLoad C 40
6 B 0 i32 4 SLoad C 40