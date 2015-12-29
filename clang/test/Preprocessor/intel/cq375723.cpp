// RUN: %clang_cc1 -fintel-compatibility -E -verify %s
// expected-no-errors

#line 12 incorrect filename // expected-warning{{expected a file name}}
#line 13 incorrect_filename.cpp // expected-warning{{expected a file name}}
#line 228 "correct_filename.cpp" // no warning

#20 incorrect // expected-warning{{expected a file name}}

# 19 "/export/users/aeloviko/ws/boost/boost_1_50_0/libs/chrono/test/../../../boost/mpl/lambda_fwd.hpp" 2
