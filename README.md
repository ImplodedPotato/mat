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
#include "mat/mat.h"

int main() {
    char *equation = "f(x)=3(x+1)^4-1";
    Mat mat = { 0 };
    if (init_mat(&mat, equation, 0, true, true) < 0) { return 1; }

    while (step_mat(&mat)) {
        switch (mat.token) {
            case Number: {
                printf("Num: %f\n", mat.number);
            } break;
            case Variable: {
                printf("Var: %c\n", mat.variable);
            } break;
            case Exponent: {
                printf("Xpn:  ^\n");
            } break;
            case Function: {
                printf("Fn:  %c\n", mat.function);
            } break;
            case CASE_BRACES: {
                printf("Dlm: %c\n", mat.token);
            } break;
            case Fail: {
                printf(MAT_ERROR"Something Went Wrong\n");
                return 1;
            } break;
            default: {
                printf(MAT_WARN"Unhandled Token: %s\n", token_to_cstr(mat.token));
            } break;
        }
    }
}
```

## TODO

- Add an optional namespacing with an 'MAT_PREFIX' macro.
- Add a function to return all of the tokens.
