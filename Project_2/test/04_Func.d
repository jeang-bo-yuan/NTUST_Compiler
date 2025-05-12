const int A = (1+100)*100/2;
int global = 0;

void main(const int argc, const string argv[100]) {
    int b = A + argc % 10;
    global = b++;
}

void oops(int a, double b, float c, string d, bool e, int aA[10]){}