
long func(long n, long *A, long *B) {

    long sum = 0;
    for(long i=0; i<n ; i++) {
        if (n%5) {
         sum += A[i];
        }
    }

    for(long i=0; i<n ; i++) {
        if (A[i] && B[i] && i != 9) {
            sum += B[i];
        } else {
            sum += 1;
            A[i] = B[i];
        }
    }
    return sum;

}
