; RUN opt < %s -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that we are able to correctly link bblocks using reachability analysis.
; HIR verifier checks that test passes successfully.

; CHECK: DO i1 = 0, smax(1, %n) + -1, 1


target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

define void @DistortImage(i64 %n) {
t1800:                                             ; preds = %t1775
  br label %t1801

t1801:                                             ; preds = %t1975, %t1800
  %t1806 = phi i64 [ %t1978, %t1975 ], [ 0, %t1800 ]
  switch i64 %t1806, label %t1961 [
    i64 18, label %t1936
    i64 0, label %t1948
    i64 17, label %t1924
    i64 2, label %t1819
    i64 3, label %t1816
    i64 16, label %t1912
    i64 5, label %t1831
    i64 6, label %t1834
    i64 7, label %t1846
    i64 15, label %t1900
    i64 9, label %t1858
    i64 10, label %t1861
    i64 11, label %t1873
    i64 12, label %t1885
    i64 14, label %t1897
    i64 4, label %t1950
    i64 19, label %t1953
    i64 1, label %t1954
    i64 13, label %t1952
    i64 8, label %t1951
  ]

t1816:                                             ; preds = %t1801
  br label %t1965

t1819:                                             ; preds = %t1801
  br label %t1993

t1831:                                             ; preds = %t1801
  br label %t1965

t1834:                                             ; preds = %t1801
  br label %t2000

t1846:                                             ; preds = %t1801
  br label %t2006

t1858:                                             ; preds = %t1801
  br label %t1965

t1861:                                             ; preds = %t1801
  br label %t2013

t1873:                                             ; preds = %t1801
  br label %t2019

t1885:                                             ; preds = %t1801
  br label %t2025

t1897:                                             ; preds = %t1801
  br label %t1965

t1900:                                             ; preds = %t1801
  br label %t2032

t1912:                                             ; preds = %t1801
  br label %t2038

t1924:                                             ; preds = %t1801
  br label %t2044

t1936:                                             ; preds = %t1801
  br label %t2050

t1948:                                             ; preds = %t1801
  br label %t1965

t1950:                                             ; preds = %t1801
  br label %t1954

t1951:                                             ; preds = %t1801
  br label %t1954

t1952:                                             ; preds = %t1801
  br label %t1954

t1953:                                             ; preds = %t1801
  br label %t1954

t1954:                                             ; preds = %t1953, %t1952, %t1951, %t1950, %t1801
  br label %t1987

t1961:                                             ; preds = %t1801
  br label %t1987

t1965:                                             ; preds = %t1948, %t1897, %t1858, %t1831, %t1816
  br label %t1975

t1975:                                             ; preds = %t2056, %t2050, %t2044, %t2038, %t2032, %t2031, %t2025, %t2019, %t2013, %t2012, %t2006, %t2000, %t1999, %t1993, %t1992, %t1987, %t1965
  %t1978 = add nsw i64 %t1806, 1
  %t1986 = icmp slt i64 %t1978, %n
  br i1 %t1986, label %t1801, label %t2057

t1987:                                             ; preds = %t1961, %t1954
  switch i64 %t1806, label %t1975 [
    i64 11, label %t2019
    i64 10, label %t2013
    i64 1, label %t1992
    i64 2, label %t1993
    i64 16, label %t2038
    i64 4, label %t1999
    i64 19, label %t2056
    i64 12, label %t2025
    i64 18, label %t2050
    i64 13, label %t2031
    i64 15, label %t2032
    i64 17, label %t2044
    i64 6, label %t2000
    i64 7, label %t2006
    i64 8, label %t2012
  ]

t1992:                                             ; preds = %t1987
  br label %t1975

t1993:                                             ; preds = %t1987, %t1819
  br label %t1975

t1999:                                             ; preds = %t1987
  br label %t1975

t2000:                                             ; preds = %t1987, %t1834
  br label %t1975

t2006:                                             ; preds = %t1987, %t1846
  br label %t1975

t2012:                                             ; preds = %t1987
  br label %t1975

t2013:                                             ; preds = %t1987, %t1861
  br label %t1975

t2019:                                             ; preds = %t1987, %t1873
  br label %t1975

t2025:                                             ; preds = %t1987, %t1885
  br label %t1975

t2031:                                             ; preds = %t1987
  br label %t1975

t2032:                                             ; preds = %t1987, %t1900
  br label %t1975

t2038:                                             ; preds = %t1987, %t1912
  br label %t1975

t2044:                                             ; preds = %t1987, %t1924
  br label %t1975

t2050:                                             ; preds = %t1987, %t1936
  br label %t1975

t2056:                                             ; preds = %t1987
  br label %t1975

t2057:                                             ; preds = %t1975
  ret void
}
