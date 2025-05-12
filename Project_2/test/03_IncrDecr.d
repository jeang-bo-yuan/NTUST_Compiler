main() {
    int a, b;
    int c = ++a % 100 / --b - a++ + b-- / 100;
    // next line is illegal.
    // const int d = ++a;
}
