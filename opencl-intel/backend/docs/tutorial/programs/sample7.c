
long func(long n, long *A, long *B) {

    long sum = 0;
    for(long i=0; i<n ; i++) {
         sum += A[i];
    }

    for(long i=0; i<n ; i++) {
            sum += B[i];
    }
    return sum;

}
