int iA() { return 1; }
float fA() { return 1.0f; }
double dA() { return 1.0; }
bool bA() { return true; }
void Hello() { println "Hello World!"; return; }

main() {
    println iA() + 10;
    println fA() + 10.0f;
    println dA() + 10.0;
    println bA() && false;
    Hello();
}
