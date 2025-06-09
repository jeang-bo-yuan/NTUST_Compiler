/**
* return a + (a+1) + ... + b (a <= b)
*/
int sigma (int a, int b) {
    if (b < a) {  // swap a, b
        int temp = a;
        a = b;
        b = temp;
    }

    int sum = 0;
    for (; a <= b; ++a) {
        sum = sum + a;
    }

    return sum;
}

main() {
    println sigma(1, 10); // 55
    println sigma(10, 1); // 55
}
