
long func(long n, long *A, long *B) {

    long sum = 0;
    for(long i=0; i<n/10 ; i++) {
        sum += B[i];
        for(long j=0; j<n/10 ; j++) {
        sum += A[j];
        }
    }

    return sum;
}
