main() {
    // Array
    int MyArray1[10], MyArray2[10], MyArray3[10];
    int MyArrayCopy[10] = MyArray2 = MyArray1;
    bool MyArray_Comp = (MyArray1 = MyArray2) == MyArrayCopy || MyArray3 != MyArrayCopy;
}
