
long func(long n, long *A, long *B) {

        for (long i=0; i<n/4; i++) {
            for (long j=0; j<n/10; j++) {
                A[i^90] = A[i + 10];
            }
            B[i] ^= n;
        }
    return B[3] + A[3];
}
