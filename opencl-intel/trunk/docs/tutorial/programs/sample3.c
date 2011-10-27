
long func(long n, long *A, long *B) {
    long sum = 0;
    for(long i=0; i<n; i++) {
        if (B[i]) {
            sum += A[i];
        }
        B[i] = 0;
    }

    return sum;

}
