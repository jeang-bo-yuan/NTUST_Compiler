/**
*  展示：
*  1. [] 運算子
*  2. 陣列透過 [] 取值後，可以變成 lvalue
*/
double A[100];

main(int argc, string argv[100]) {
    double _A[100];
    int i;
    
    foreach (i : 0 .. 100)
        A[i] = _A[i] = 10.0;

    if (A[0] == 9.0)
        return;

    int I[10][20];
    I[9][9] = I[0][0]-- + I[1][1]++ + --I[0][1] + ++I[1][0];

    return;
}