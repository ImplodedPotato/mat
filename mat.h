// Mat (MAth Tokenizer)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <stdbool.h>

#define MAT_WARN   "\e[33mWarning:\e[0m "
#define MAT_ERROR  "\e[1;31mError:\e[0m "

// From nob.h: https://github.com/tsoding/nob.h
#ifdef __cplusplus
#define MAT_DECLTYPE_CAST(T) (decltype(T))
#else
#define MAT_DECLTYPE_CAST(T)
#endif // __cplusplus

// From nob.h: https://github.com/tsoding/nob.h
#define mat_da_reserve(da, expected_capacity)                                              \
    do {                                                                                   \
        if ((expected_capacity) > (da)->capacity) {                                        \
            if ((da)->capacity == 0) {                                                     \
                (da)->capacity = 24;                                                       \
            }                                                                              \
            while ((expected_capacity) > (da)->capacity) {                                 \
                (da)->capacity *= 2;                                                       \
            }                                                                              \
            (da)->items = MAT_DECLTYPE_CAST((da)->items)realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                         \
        }                                                                                  \
    } while (0)

// From nob.h: https://github.com/tsoding/nob.h
#define mat_da_append(da, item)                \
    do {                                       \
        mat_da_reserve((da), (da)->count + 1); \
        (da)->items[(da)->count++] = (item);   \
    } while (0)

typedef enum MatFormat {
    Mat_F_LaTeX = 0,
    Mat_F_Mat,
} MatFormat;

// The compiler doesn't always set all of these to unique values
//   when only some are set, so each has to be set explicitly.
//   Don't depend on the char values.
typedef enum Mat_Token {
    Mat_T_Fail = 0,
    Mat_T_Done = 1,

            //    MatFormat::FLaTeX            MatFormat::FMat
            // expr examples: 1, x, (1+2), \sqrt[3]{8}, -\inf

    Mat_T_Variable  = 'x', // x, y, z, ...           x, y, z, ...
    Mat_T_Number    = 'n', // 1, 3.5, .2             1, 3.5, .2
    Mat_T_Function  = 'f', // f(x) g(a)              f(x) g(a)

    Mat_T_Factorial = '!', // expr!                  expr!
    Mat_T_E         = 'e', // e                      e
    Mat_T_Imaginary = 'i', // e                      e
    Mat_T_Pi        = 'p', // \pi                    \pi
    Mat_T_Inf       = ';', // \infty                 \inf
    Mat_T_Cos       = 'C', // \cos{expr} or          \cos(expr) or
                     //  \cos expr               \cos expr
    Mat_T_Sin       = 'S', // \sin{expr} or          \sin(expr) or
                     //  \sin expr               \sin expr
    Mat_T_Log       = 'O', // \log{expr} or           \log(expr) or
                     //  \log expr or             \log expr or
                     //   \log_expr{expr}          \log_expr(expr) // using SubScript for bases
    Mat_T_Ln        = 'N', // \ln{expr} or           \ln(expr) or
                     //  \ln expr or             \ln expr or

    Mat_T_Equal     = '=', // expr=expr              expr=expr
    Mat_T_Add       = '+', // expr+expr or +expr     expr+expr or +expr
    Mat_T_Subtract  = '-', // expr-expr or -expr     expr-expr or -expr
 // Multiplication is represented as two exprs beside eachother,
 //                                       ie. \frac{expr}{expr}expr
 //   '*' may also be used in MatFormat::FMat,
 //   but will implicitly be converted to '(' and ')' in MatFormat::FLaTeX
 // Division       is represented as fraction, see Token::Fraction
 //                                       ie. \frac{expr}{expr}

    Mat_T_Exponent     = '^', // expr^{expr}             expr^expr
    Mat_T_Fraction     = '/', //  \frac{expr}{expr}      expr/expr
    Mat_T_SubScript    = '_', //  _expr                  _expr
    Mat_T_SQRT         = 'q', //  \sqrt{expr} or         \sqrt(expr) or
                        //  \sqrt[expr]{expr}      \rt[expr](expr)
    Mat_T_Less         = 'l', //  expr \lt expr          expr < expr
    Mat_T_Greater      = 'g', //  expr \gt expr          expr > expr
    Mat_T_LessEqual    = 'L', //  expr \le expr          expr <= expr
    Mat_T_GreaterEqual = 'G', //  expr \ge expr          expr >= expr
    Mat_T_NotEqual     = '~', //  expr \neq expr         expr != expr
    Mat_T_Prime        = 'P', //  \prime
    Mat_T_Lim          = 'I', //  \lim_{ expr \to expr }     \lim(expr \to expr)
    Mat_T_To           = 'T', //  \to                    \to

    Mat_T_OParen   = '(' ,  // '('                    '('
    Mat_T_CParen   = ')' ,  // ')'                    ')'
    Mat_T_OCurly   = '{' ,  // '{'                    '{'
    Mat_T_CCurly   = '}' ,  // '}'                    '}'
    Mat_T_OBracket = '[' ,  // '['                    '['
    Mat_T_CBracket = ']' ,  // ']'                    ']'
} Mat_Token;

#define MAT_CASE_BRACES '(': case ')': case '{': case '}': case '[': case ']'
#define MAT_CASE_SINGLES '=': case '+': case '-': case '*': case '^': case '!': case 'e': case 'i'

#define MAT_WORD_TOKENS_MAP_LEN 16

// A map of escape sequence words, they need a '\' in in front of them
static const  struct  {
    const char *cstrs[MAT_WORD_TOKENS_MAP_LEN];
    Mat_Token tokens[MAT_WORD_TOKENS_MAP_LEN];
} MAT_WORD_TOKENS_MAP = {
    .cstrs = {
        "pi",
        "infty",
        "cos",
        "sin",
        "log",
        "ln",
        "frac",
        "sqrt",
        "lt",
        "gt",
        "le",
        "ge",
        "neq",
        "prime",
        "lim_",
        "to",
    },
    .tokens = {
        Mat_T_Pi,
        Mat_T_Inf,
        Mat_T_Cos,
        Mat_T_Sin,
        Mat_T_Log,
        Mat_T_Ln,
        Mat_T_Fraction,
        Mat_T_SQRT,
        Mat_T_Less,
        Mat_T_Greater,
        Mat_T_LessEqual,
        Mat_T_GreaterEqual,
        Mat_T_NotEqual,
        Mat_T_Prime,
        Mat_T_Lim,
        Mat_T_To,
    }
};

// Used the same field names as DAs in nob.h therefore,
// you can use those functions on these types.
// Although, nob.h isn't required.
typedef struct Mat_String {
    char *items;
    uint count;
    uint capacity;
} String;

typedef struct Mat_StringSlice {
    char *items;
    uint count;
} Mat_StringSlice;

typedef union Mat_TokenData {
    char   var;
    char   fun; // 'fun', as it's three chars long, like 'var' and 'num'.
    double num;
} Mat_TokenData;

typedef struct Mat_TokenAndData {
    Mat_Token token;
    Mat_TokenData data;
} Mat_TokenAndData;
#define Mat_TAD Mat_TokenAndData

typedef struct Mat_Tokens {
    Mat_TAD *items;
    size_t count;
    size_t capacity;
} Mat_Tokens;

#define MAT_SSF "%.*s" // StringSliceFormat

typedef struct Mat {
    MatFormat ftype;
    bool show_warnings;
    char *in_stream;
    int offset;
    bool strict_reporting; // returns false if Token == Fail
    char *fail;

    Mat_Token  token;
    Mat_TokenData data;
} Mat;

typedef struct Mat_InitOps {
    MatFormat ftype;
    bool show_warnings;
    bool strict_reporting;
} Mat_InitOps;

int mat__init(Mat *mat, char *in_stream, Mat_InitOps ops);
#define mat_init(mat, in_stream, ...) \
        mat__init((mat), (in_stream), (Mat_InitOps){  \
                .ftype = Mat_F_LaTeX,                  \
                .show_warnings = true,                 \
                .strict_reporting = true,              \
                __VA_ARGS__                            \
            })                                         \

bool mat_step(Mat *mat);
Mat_Tokens mat_get_all_tokens(Mat *mat);
Mat_TAD mat_get_tad(const Mat *mat);

const char *mat_token_to_cstr(const Mat_Token token);
Mat_Token mat_get_word_token(Mat_StringSlice ss);
Mat_StringSlice mat_get_word(char *str);

bool mat_ss_strcmp(const Mat_StringSlice ss, const char *cs);

void mat_print_tad(const Mat_TAD tad);

#define MAT_IMPLEMENTATION
#ifdef MAT_IMPLEMENTATION

int mat__init(Mat *mat, char *in_stream, Mat_InitOps ops) {
    if (!in_stream) { return -1; }

    *mat = (Mat){
        .ftype = ops.ftype,
        .in_stream = in_stream,
        .show_warnings = ops.show_warnings,
        .token = Mat_T_Done,
        .data = {0},
        .strict_reporting = ops.strict_reporting,
        .fail = NULL,
    };

    return 0;
}

bool mat_step(Mat *mat) {
    char *str = mat->in_stream+mat->offset;
    char curr_char = *str;
    char next_char = *(str+1);
    if (curr_char == '\0') {
        mat->token = Mat_T_Done;
        return false;
    }
    if (mat->strict_reporting && mat->token == Mat_T_Fail) {
        printf(MAT_ERROR"Fatal Error: number: %f, variable: '%c', offset: %d",
            mat->data.num, mat->data.var, mat->offset);
        return false;
    }

    switch (curr_char) {
        // WARNING: This only works with GCC and Clang
        case '0' ... '9': case '.': {
            char *end = NULL;
            mat->data.num = strtod(str, &end);
            mat->offset = end-mat->in_stream;
            mat->token = Mat_T_Number;
            return true;
        } break;
        case 'a' ... 'd': case 'f' ... 'h': case 'j' ... 'z': case 'A' ... 'Z': {
            // Excludes 'i', the imaginary unit, and the natual number 'e'
            if ('(' == next_char) {
                mat->token = Mat_T_Function;
                mat->data.fun = curr_char;
                mat->offset++;
                return true;
            }
            mat->token = Mat_T_Variable;
            mat->offset++;
            mat->data.var = curr_char;
            return true;
        } break;
        case '\\': {
            char *cstr = str+1;
            Mat_StringSlice word = mat_get_word(cstr);
            Mat_Token t = mat_get_word_token(word);
            if (t == Mat_T_Fail) {
                printf(MAT_WARN"Unhandled: \\'" MAT_SSF"'\n", word.count, word.items);
                return false;
            }
            mat->token = t;
            mat->offset += word.count+1;
            return true;
        } break;
        case MAT_CASE_BRACES: case MAT_CASE_SINGLES: {;
            mat->token = (Mat_Token)curr_char;
            mat->offset++;
            return true;
        } break;
    }
    printf(MAT_WARN"Unexpected: '%c'\n", curr_char);
    mat->token = Mat_T_Fail;
    mat->fail = (char*)MAT_ERROR"Fail";
    mat->offset++;
    return true;
}

// TODO: Should this really be a function
inline
Mat_TAD mat_get_tad(const Mat *mat) {
    return (Mat_TAD){
        .token = mat->token,
        .data = mat->data,
    };
}

void mat_print_tad(const Mat_TAD tad) {
    printf("%s", mat_token_to_cstr(tad.token));
    if (tad.token == Mat_T_Variable) {
        printf("(%c)\n", tad.data.var);
    } else if (tad.token == Mat_T_Number) {
         printf("(%f)\n", tad.data.num);
    } else  if (tad.token == Mat_T_Function) {
         printf("(%c)\n", tad.data.fun);
    } else {
        printf("\n");
    }
}

Mat_Tokens mat_get_all_tokens(Mat *mat) {
    Mat_Tokens tks = { 0 };

    while (mat_step(mat)) {
        Mat_TAD tad = mat_get_tad(mat);
        mat_print_tad(tad);
        mat_da_append(&tks, tad);
    }

    return tks;
}

Mat_StringSlice mat_get_word(char *str) {
    uint len = 0;

    for (char next = *str; next >= 'a' && next <= 'z'; len++) {
        next = *(str+len);
    }

    return (Mat_StringSlice){
        .items = str,
        .count = len-1,
    };
}

Mat_Token mat_get_word_token(Mat_StringSlice ss) {
    for (int i = 0; i < MAT_WORD_TOKENS_MAP_LEN; ++i) {
        if (mat_ss_strcmp(ss, MAT_WORD_TOKENS_MAP.cstrs[i])) {
            return MAT_WORD_TOKENS_MAP.tokens[i];
        }
    }
    return Mat_T_Fail;
}

inline
bool mat_ss_strcmp(const Mat_StringSlice ss, const char *cs) {
    return memcmp(ss.items, cs, ss.count) == 0 ? true : false;
}

const char *mat_token_to_cstr(const Mat_Token token) {
    switch (token) {
        case Mat_T_Fail:         { return "Fail";      } break;
        case Mat_T_Done:         { return "Done";      } break;

        case Mat_T_Variable:     { return "Variable";  } break;
        case Mat_T_Number:       { return "Number";    } break;
        case Mat_T_Function:     { return "Function";  } break;

        case Mat_T_E:            { return "e";         } break;
        case Mat_T_Imaginary:    { return "i";         } break;
        case Mat_T_Pi:           { return "π (pi)";    } break;
        case Mat_T_Inf:          { return "∞ (infty)"; } break;
        case Mat_T_Factorial:    { return "!";         } break;
        case Mat_T_Cos:          { return "cos";       } break;
        case Mat_T_Sin:          { return "sin";       } break;
        case Mat_T_Log:          { return "log";       } break;
        case Mat_T_Ln:           { return "ln";        } break;

        case Mat_T_Equal:        { return "=";         } break;
        case Mat_T_Add:          { return "+";         } break;
        case Mat_T_Subtract:     { return "-";         } break;
        case Mat_T_Exponent:     { return "Exponent";  } break;
        case Mat_T_Fraction:     { return "Fraction";  } break;
        case Mat_T_SubScript:    { return "SubScript"; } break;
        case Mat_T_SQRT:         { return "sqrt";      } break;
        case Mat_T_Less:         { return "<";         } break;
        case Mat_T_Greater:      { return ">";         } break;
        case Mat_T_LessEqual:    { return "<=";        } break;
        case Mat_T_GreaterEqual: { return ">=";        } break;
        case Mat_T_NotEqual:     { return "!=";        } break;
        case Mat_T_Prime:        { return "Prime (`)"; } break;
        case Mat_T_Lim:          { return "Lim";       } break;
        case Mat_T_To:           { return "To (->)";   } break;

        case Mat_T_OParen:       { return "(";         } break;
        case Mat_T_CParen:       { return ")";         } break;
        case Mat_T_OCurly:       { return "{";         } break;
        case Mat_T_CCurly:       { return "}";         } break;
        case Mat_T_OBracket:     { return "[";         } break;
        case Mat_T_CBracket:     { return "]";         } break;
    }
}

#endif
