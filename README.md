# Mat (MAth Tokenizer)

A simple, singluar, no dependency (other than std), C99 ```.h``` file to parse and tokenize math equations.

## Details

- Currently only supports LaTeX.

## Gettting Started

Just copy ```mat.h```, include it, and use it in you project.


## Simple example

``` C
#include <stdio.h>
#define MAT_IMPLEMENTATION
#include "mat.h"

int main() {
    char *equation = "f(x)=2(x+2)^2-5";
    Mat mat = { 0 };
    if (init_mat(&mat, equation, 0, true) < 0) { return 1; }

    while (step_mat(&mat)) {
        switch (mat.token) {
            case Number: {
                printf("  %f\n", mat.number);
            } break;
            case Variable: {
                printf("  %c\n", mat.variable);
            } break;
            case Exponent: {
                printf("  ^");
            } break;
            case CASE_BRACES: {
                printf("  %c\n", mat.token);
            } break;
            case Fail: {
                printf(MAT_ERROR"Something Went Wrong\n");
            } break;
            default: {
                printf(MAT_WARN"Unhandled Token: %s\n", token_to_cstr(mat.token));
            } break;
        }
    }
}
```
