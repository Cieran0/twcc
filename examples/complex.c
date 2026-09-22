int add(int a, int b) {
    return a + b;
}

int sub(int a, int b) {
    return a - b;
}

int mul(int a, int b) {
    return a * b;
}

int div(int a, int b) {
    return a / b;
}

int add_three(int a, int b, int c) {
    return a + b + c;
}

int double_number(int a) {
    return a * 2;
}

int add_and_double(int a, int b) {
    return add(a, b) + add(a, b);
}

int main() {
    int a = 10;
    int b = 20;
    int c = 30;
    
    int sum = add(a, b);
    
    int diff = sub(b, a);
    
    int product = mul(a, b);
    
    int quotient = div(b, a);
    
    int triple_sum = add_three(a, b, c);
    
    int doubled = double_number(a);
    
    int result = add_and_double(a, b);
    
    int complex_result = add(mul(a, b), sub(c, div(b, a)));
    
    return complex_result;
}