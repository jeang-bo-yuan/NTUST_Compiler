/**
*  展示 while , if / else, for, foreach
*/
const int MAX = 10926 / 10926 * 10;

int a(int b){}

main () {
    int n = MAX;
    
    while (n-- > 0) {
        const string Hello = "Hello" + ", World!";
        println Hello;
    }

    if (n == 0)
        if (false)
            ;
        else
            println "============";
    else
        return;

    for (n = 0; n <= MAX; ++n)
        println n;

    for (;;);

    foreach(n:MAX..1) {
        int x = MAX - n;
        println x;
    }

    return;
}