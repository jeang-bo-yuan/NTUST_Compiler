main() {
    {
        int a = 1, b = 10;
        println +(a * b) + -(b / a % 3); // 9
    }

    {
        float a = 1.0f, b = 10.0f;
        println +(a * b) + -(b / a); // 0
    }

    {
        double a = 1.0, b = 10.0;
        b = +(a * b) + -(b / a); // 0
        println b;
    }
}