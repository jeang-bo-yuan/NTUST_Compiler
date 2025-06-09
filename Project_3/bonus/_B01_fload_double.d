/**
* Bonus 1 : float & double
*/
float fA() { return 1.0f; }
double dA() { return 1.0; }

void fAddAndPrint(const float a, const float b) { println a + b; }
void dAddAndPrint(const double a, const double b) { println a + b; }

main() {
    fAddAndPrint(fA(), fA() * 10.0f);   // 11
    dAddAndPrint(dA(), dA() * 10.0);    // 11
}