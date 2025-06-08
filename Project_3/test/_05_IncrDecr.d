int A = 0;
main () {
    print "A++ = ";
    println A++; // 0

    print "++A = ";
    println ++A; // 2

    print "A-- = ";
    println A--; // 2

    print "--A = ";
    println --A; // 0

    int B = 0;
    print "B++ + B * ++B = "; // 0 + 1 * 2
    println B++ + B * ++B;

    // B is 2 now

    print "(A = B) * A * --B = "; // 2 * 2 * 1
    println (A = B) * A * --B;
}
