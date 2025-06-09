int iA() { return 1; }
float fA() { return 1.0f; }
double dA() { return 1.0; }
bool bA() { return true; }

void iAddAndPrint(const int a, const int b) { println a + b; }
void fAddAndPrint(const float a, const float b) { println a + b; }
void dAddAndPrint(const double a, const double b) { println a + b; }
void bAddAndPrint(const bool a, const bool b) { println a && b; }

main() {
    iAddAndPrint(iA(), iA() * 10);      // 11
    fAddAndPrint(fA(), fA() * 10.0f);   // 11
    dAddAndPrint(dA(), dA() * 10.0);    // 11
    bAddAndPrint(bA(), !bA() && false); // 0
}
