
long func(long n, long *A, long *B) {


    if (B[0]) {
        crazy:
        for(int i=0; i<n ; i++) {
            A[i] = B[i];
        }
    } else {
        for(int i=0; i<n+4 ; i++) {
            if (i == 80) goto crazy;
            A[i+4] = B[i*2];
        }
    }
    return 99;
}
