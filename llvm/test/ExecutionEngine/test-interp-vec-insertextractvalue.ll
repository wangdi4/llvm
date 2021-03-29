 ; RUN: %lli -force-interpreter=true %s > /dev/null

define i32 @main() {

    %sa1 = insertvalue [ 2 x i32 ] [i32 1, i32 2], i32 9, 0
    %sa2 = insertvalue [ 2 x i32 ] zeroinitializer, i32 9, 0
    %sa3 = insertvalue [ 2 x i32 ] undef, i32 9, 0
    %s0 = insertvalue { i32, { float, double} } zeroinitializer, i32 9, 0
    %s1 = insertvalue { i32, { float, double} } undef, i32 9, 0
    %s2 = insertvalue { i32, { float, double} } %s1, float 3.0, 1, 0
    %s3 = insertvalue { i32, { float, double} } %s2, double 5.0, 1, 1

    %s4 = extractvalue { i32, { float, double} } %s3, 1

    %a1 = extractvalue { i32, { float, double} } %s3, 0

    %a2 = extractvalue { i32, { float, double} } %s3, 1, 0
    %a3 = extractvalue { i32, { float, double} } %s3, 1, 1
    %a4 = extractvalue { float, double} %s4, 0
    %a5 = extractvalue { float, double} %s4, 1

    %aa = fpext float %a4 to double

 ret i32 0
}
