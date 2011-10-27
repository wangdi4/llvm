
long func(long n, long *A, long *B) {

    for(long i=0; i<n ; i++) {
        A[i] += B[i] * n;
    }

    return 0;
}
