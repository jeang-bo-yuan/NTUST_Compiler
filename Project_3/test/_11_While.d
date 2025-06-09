int fib (int n) {
    if (n <= 0)
        return 0;

    int Fn, FNminus1, temp;

    Fn = 1;
    FNminus1 = 1;
    while (n > 2) {
        temp = Fn;
        Fn = Fn + FNminus1;
        FNminus1 = temp;
        n = n - 1;
    }
    return Fn;
}


main() {
    println fib(16); // 987
    println fib(26); // 121393

    int A = 10;
    while (A-- != 0) { // 0, 1, 1, 2, 3, 5, 8, 13, 21, 34
        println fib(A);
    }
}
