const int A = 100;

main() {
    {
        const int A = 1;
    }
    {
        const int A = 2;
    }
    {
        const int A = 3;
        {
            const int A = 4;
            println A;
        }
        println A;
    }
    println A;

    int A = 200;
    A = 10;
    println A;

    return;
}