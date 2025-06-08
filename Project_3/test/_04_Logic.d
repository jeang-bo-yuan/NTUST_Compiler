main() {
    bool a = true, b = false;
    b = !a || (!b && a);
    print b; // 1
}