main() {
    int i = 0;
    for ( ; ; ++i) { // i = 0, 1, ..., 9
        if (i == 10) 
            break;
        else if (i % 2 == 0) 
            continue;

        print i;
        println " is odd";
    }

    while (true) { // i = 10, 9, ..., 0
        if (i < 0)
            break;
        else if (i % 2 != 0) {
            --i;
            continue;
        }
        
        print i--;
        println " is even";
    }
}
