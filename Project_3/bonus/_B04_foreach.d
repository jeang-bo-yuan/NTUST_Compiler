int A = 0;

main() {
    int sum = 0;

    // 0 .. 10
    foreach (A : A .. A + 10) {
        if (A % 2 == 0)
            continue;
        if (A == 9)
            break;

        println A;
        sum = sum + A;
    }

    print "Total Sum = ";
    println sum;

    // 0 .. 10
    foreach (A : (A = 10) - 10 .. A) {
        if (A % 2 == 0)
            continue;
        if (A == 9)
            break;

        println A;
        sum = sum + A;
    }

    print "Total Sum = ";
    println sum;
}