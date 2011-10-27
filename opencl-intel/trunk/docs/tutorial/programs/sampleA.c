
long func(long n, long *A, long *B) {


    if (n%2) {
        for(int i=0; i<n ; i++) {
            A[i] = B[i];
        }
    } else {
        for(int i=0; i<n+4 ; i++) {
            B[i] = A[i];
        }
    }
    return n;
}
