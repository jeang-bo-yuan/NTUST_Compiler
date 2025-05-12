// bool
const bool a = true || false && true || !false || !true;
const bool b = (a == true) && (a != true) || (!a);

main() {
    bool c, d = a || b || c;
}
