/* ============================================================================
   BASIC ARITHMETIC FUNCTIONS
   ============================================================================ */

// Function to add two integers together
/* Parameters: a (first number), b (second number)
   Returns: the sum of a and b */
int add(int a, int b) {
    return a + b;  // Return the sum
}

/*
 * Function to subtract the second integer from the first
 * Parameters: a (minuend), b (subtrahend)
 * Returns: the difference (a - b)
 * Note: Order matters! sub(10, 5) = 5, but sub(5, 10) = -5
 */
int sub(int a, int b) {
    return a - b;  /* Return first minus second */
}

// Function to multiply two integers together
// Parameters: a (first factor), b (second factor)
/* Returns: the product of a and b */
int mul(int a, int b) {
    return a * b;  // Return the product
}

/* Function to divide the first integer by the second using integer division
   Parameters: a (dividend), b (divisor)
   Returns: the quotient truncated to an integer (decimal part is discarded)
   WARNING: Does NOT check for division by zero!
   Example: div(7, 2) returns 3, not 3.5 */
int div(int a, int b) {
    return a / b;  /* Return integer quotient */
}

/* ============================================================================
   COMPOSITE ARITHMETIC FUNCTIONS
   ============================================================================ */

// Function to add three integers together
/* Parameters: a, b, c (the three numbers to add)
   Returns: the sum of all three parameters */
int add_three(int a, int b, int c) {
    return a + b + c;  // Return sum of three numbers
}

/*
 * Function to double a given integer
 * Parameters: a (the number to double)
 * Returns: a multiplied by 2
 * Example: double_number(5) returns 10
 */
int double_number(int a) {
    return a * 2;  /* Multiply by 2 */
}

// Function to add two numbers and then double the result
/* This calls add() twice and adds the results: (a+b) + (a+b) = 2*(a+b)
   Parameters: a (first number), b (second number)
   Returns: double the sum of a and b
   Example: add_and_double(3, 4) returns 14 because (3+4)*2 = 14 */
int add_and_double(int a, int b) {
    return add(a, b) + add(a, b);  // Add twice to get double the sum
}

/* ============================================================================
   MAIN FUNCTION - Program Entry Point
   ============================================================================ */

int main() {
    // Initialize three integer variables with sample values
    int a = 10;   /* First operand */
    int b = 20;   // Second operand
    int c = 30;   /* Third operand */
    
    /* Calculate sum of a and b: 10 + 20 = 30 */
    int sum = add(a, b);
    
    // Calculate difference of b and a: 20 - 10 = 10
    int diff = sub(b, a);
    
    /*
     * Calculate product of a and b: 10 * 20 = 200
     */
    int product = mul(a, b);
    
    // Calculate quotient of b divided by a: 20 / 10 = 2
    int quotient = div(b, a);
    
    /* Calculate sum of all three variables: 10 + 20 + 30 = 60 */
    int triple_sum = add_three(a, b, c);
    
    // Double the value of a: 10 * 2 = 20
    int doubled = double_number(a);
    
    /* Add a and b, then double: (10 + 20) * 2 = 60 */
    int result = add_and_double(a, b);
    
    // Complex nested calculation:
    /* Step 1: mul(a, b) = 10 * 20 = 200
       Step 2: div(b, a) = 20 / 10 = 2
       Step 3: sub(c, 2) = 30 - 2 = 28
       Step 4: add(200, 28) = 200 + 28 = 228
       Final result: 228 */
    int complex_result = add(mul(a, b), sub(c, div(b, a)));
    
    /* Return the complex_result as the program exit code */
    return complex_result;
}