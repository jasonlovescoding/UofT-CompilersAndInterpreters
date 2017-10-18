{
    int a;
    int b;
    int c;
    int d;

    /* Precedence 6 vs 7 */
    d = a || b && c;
    d = a && b || c;

    /* Precedence 5 vs 6 */
    d = a == b && c;
    d = a && b == c;

    /* Precedence 4 vs 5 */
    d = a + b == c;
    d = a == b + c;

    /* Precedence 3 vs 4 */
    d = a * b + c;
    d = a + b * c;

    /* Precedence 2 vs 3 */
    d = a ^ b * c;
    d = a * b ^ c;

    /* Precedence 1 vs 2 */
    d = a ^ -b;
    d = -a ^ b;

    /* Precedence 0 vs 1 */
    d = -a[1];
}

