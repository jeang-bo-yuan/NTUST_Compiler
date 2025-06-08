const int AAA = 1000 / 10;
int BBB = 1000 / 25;

main() {
    int a = AAA;
    float b = 2.0f;
    double c = 3.0;
    bool d = true;
    const int CCC = 1000 + 1;

    if (true) {
        int a = BBB;

        {
            int c = CCC;
            int d = a;
        }
    }
}
