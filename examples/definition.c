int add(int a, int b);

int add_and_double(int a, int b) {
    return add(a,b) * 2;
}

int add(int a, int b) {
    return a + b;
}
