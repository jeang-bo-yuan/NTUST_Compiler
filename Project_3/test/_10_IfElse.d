int ABS(const int a) {
    if (a < 0)
        return -a;
    else {
        if (a == 0)
            println "is ABS(0)";
        return a;
    }
}

int fib(const int i) {
    int res;

    if (i <= 0)      res = 0;
    else if (i <= 2) res = 1;
    else             res = fib(i - 2) + fib(i - 1);

    return res;
}

main() {
    println ABS(-100); // 100
    println ABS(0);   // 0
    println ABS(100); // 100

    println fib(16); // 987
}
