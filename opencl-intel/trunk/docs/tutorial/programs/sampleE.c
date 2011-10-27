
long func(long n, long *A, long *B) {

    for (long i=0; i<n/100; i++) {
        if (i %2) {
            for (long j=0; j<600; j++) {
                A[i^90] = A[i^5 + 10];
            }
        } else {
            for (long j=10; j<600; j+=2) {
                B[i^40] = A[i^2];
            }
        }
    }
    return A[n/2];
}
