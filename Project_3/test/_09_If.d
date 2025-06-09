int ABS(const int a) {
    if (a < 0)
        return -a;
    if (a == 0)
        println "is ABS(0)";
    return a;
}

int fib(const int i) {
    if (i <= 0) return 0;
    if (i <= 2) return 1;
    return fib(i - 2) + fib(i - 1);
}

main() {
    println ABS(-100); // 100
    println ABS(0);   // 0
    println ABS(100); // 100

    println fib(16); // 987
}
