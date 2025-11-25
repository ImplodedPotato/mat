# Mat (MAth Tokenizer)

A simple, singluar, no dependency (other than std), C99 ```.h``` file to parse and tokenize math equations.

## Details

- Currently only supports LaTeX.

## Gettting Started

Just copy ```mat.h```, include it, and use it in you project.

## Examples

### Simple mat_step() Example

``` C
#include <stdio.h>
#define MAT_IMPLEMENTATION
#include "mat/mat.h"

int main() {
    char *equation = "f(x)=3(x+1)^4-1";
    Mat mat = { 0 };
    if (mat_init(&mat, equation) < 0) { return 1; }

    while (mat_step(&mat)) {
        switch (mat.token) {
            case Mat_T_Number: {
                printf("Num: %f\n", mat.data.num);
            } break;
            case Mat_T_Variable: {
                printf("Var: %c\n", mat.data.var);
            } break;
            case Mat_T_Exponent: {
                printf("Xpn:  ^\n");
            } break;
            case Mat_T_Function: {
                printf("Fn:  %c\n", mat.data.fun);
            } break;
            case MAT_CASE_BRACES: {
                printf("Dlm: %c\n", mat.token);
            } break;
            case Mat_T_Fail: {
                printf(MAT_ERROR"Something Went Wrong\n");
                return 1;
            } break;
            default: {
                printf(MAT_WARN"Unhandled Token: %s\n", mat_token_to_cstr(mat.token));
            } break;
        }
    }
}
```

### Simple mat_get_all_tokens() Example

``` C
#include <stddef.h>
#include <stdio.h>
#define MAT_IMPLEMENTATION
#include "mat/mat.h"

int main() {
    char *equation = "f(x)=3(x+1)^4-1";
    Mat mat = { 0 };
    if (mat_init(&mat, equation) < 0) { return 1; }

    Mat_Tokens tokens = mat_get_all_tokens(&mat);

    for (size_t i = 0; i < tokens.count; ++i) {
        Mat_TAD tad = tokens.items[i];
        mat_print_tad(tad);
    }
}
```
