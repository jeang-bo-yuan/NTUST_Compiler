const int AAA = 1000 / 10;
int BBB = 1000 / 25;

main() {
    int a = AAA;
    print a;
    print "  ";
    
    float b = 1.0e3f;
    print b;
    print "  ";

    double c = 3.0;
    print c;
    print "  ";

    bool d = true;
    println d;

    {
        int TMP;
        TMP = a;
        print "a = ";
        println a = BBB;
        print "BBB = ";
        println BBB = TMP;
    }
}
