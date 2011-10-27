
long func(long n, long *A, long *B) {
    if (A[0]) {
        for (long i=0; i<n/30; i++) {
            for (long j=0; j<700; j++) {
                A[j] = B[i];
            }
        }
    } else {
        for (long j=0; j<n/30; j++) {
            A[j] = B[j];
        }
    }
    return A[3];
}
