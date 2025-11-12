// Mat (MAth Tokenizer)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>

#define MAT_WARN   "\e[33mWarning:\e[0m "
#define MAT_ERROR  "\e[1;31mError:\e[0m "

typedef enum MatFormat {
    FLaTeX = 0,
    FMat,
} MatFormat;

// The compiler doesn't always set all of these to unique values
//   when only some are set, so each has to be set explicitly
typedef enum Token {
    Fail = 0,
    Done = 1,

            //    MatFormat::FLaTeX            MatFormat::FMat
            // expr examples: 1, x, (1+2), \sqrt[3]{8}, -\inf

    Variable  = 'x', // x, y, z, ...           x, y, z, ...
    Number    = 'n', // 1, 3.5, .2             1, 3.5, .2
    Factorial = '!', // expr!                  expr!
    E         = 'e', // e                      e
    Imaginary = 'i', // e                      e
    Pi        = 'p', // \pi                    \pi
    Inf       = ';', // \infty                 \inf
    Cos       = 'C', // \cos{expr} or          \cos(expr) or
                     //  \cos expr               \cos expr
    Sin       = 'S', // \sin{expr} or          \sin(expr) or
                     //  \sin expr               \sin expr
    Log       = 'O', // \log{expr} or           \log(expr) or
                     //  \log expr or             \log expr or
                     //   \log_expr{expr}          \log_expr(expr) // using SubScript for bases
    Ln        = 'N', // \ln{expr} or           \ln(expr) or
                     //  \ln expr or             \ln expr or

    Equal     = '=', // expr=expr              expr=expr
    Add       = '+', // expr+expr or +expr     expr+expr or +expr
    Subtract  = '-', // expr-expr or -expr     expr-expr or -expr
 // Multiplication is represented as two exprs beside eachother,
 //                                       ie. \frac{expr}{expr}expr
 //   '*' may also be used in MatFormat::FMat,
 //   but will implicitly be converted to '(' and ')' in MatFormat::FLaTeX
 // Division       is represented as fraction, see Token::Fraction
 //                                       ie. \frac{expr}{expr}

    Exponent     = '^', // expr^{expr}             expr^expr
    Fraction     = 'f', //  \frac{expr}{expr}      expr/expr
    SubScript    = '_', //  _expr                  _expr
    SQRT         = 'q', //  \sqrt{expr} or         \sqrt(expr) or
                        //  \sqrt[expr]{expr}      \rt[expr](expr)
    Less         = 'l', //  expr \lt expr          expr < expr
    Greater      = 'g', //  expr \gt expr          expr > expr
    LessEqual    = 'L', //  expr \le expr          expr <= expr
    GreaterEqual = 'G', //  expr \ge expr          expr >= expr
    NotEqual     = '~', //  expr \neq expr         expr != expr
    Prime        = 'P', //  \prime
    Lim          = 'I', //  \lim_{ expr \to expr }     \lim(expr \to expr)
    To           = 'T', //  \to                    \to

    OParen   = '(' ,  // '('                    '('
    CParen   = ')' ,  // ')'                    ')'
    OCurly   = '{' ,  // '{'                    '{'
    CCurly   = '}' ,  // '}'                    '}'
    OBracket = '[' ,  // '['                    '['
    CBracket = ']' ,  // ']'                    ']'
} Token;

#define CASE_BRACES '(': case ')': case '{': case '}': case '[': case ']'
#define CASE_SINGLS '=': case '+': case '-': case '*': case '^': case '!': case 'e': case 'i'

#define WORD_TOKENS_MAP_LEN 16

static const  struct  {
    const char *cstrs[WORD_TOKENS_MAP_LEN];
    Token tokens[WORD_TOKENS_MAP_LEN];
} WORD_TOKENS_MAP = {
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
        Pi,
        Inf,
        Cos,
        Sin,
        Log,
        Ln,
        Fraction,
        SQRT,
        Less,
        Greater,
        LessEqual,
        GreaterEqual,
        NotEqual,
        Prime,
        Lim,
        To,
    }
};

typedef struct Mat {
    MatFormat ftype;
    bool show_warnings;
    char *in_stream;
    int offset;
    bool strict_reporting; // returns false if Token == Fail
    char *fail;

    Token token;
    char  variable; // Should multi-char variable names be supported?
    double number;
} Mat;

typedef struct String {
    char *data;
    uint len;
    uint cap;
} String;

typedef struct StringSlice {
    char *data;
    uint len;
} StringSlice;

#define SSF "%.*s"

int init_mat(Mat *mat, char *in_stream, MatFormat ftype, bool show_warnings, bool strict_reporting);
bool step_mat(Mat *mat);

const char *token_to_cstr(const Token token);
Token get_word_token(StringSlice ss);

StringSlice get_word(char *str);
bool ss_strcmp(const StringSlice ss, const char *cs);

#define MAT_IMPLEMENTATION
#ifdef MAT_IMPLEMENTATION

int init_mat(Mat *mat, char *in_stream, MatFormat ftype, bool show_warnings, bool strict_reporting) {
    if (!in_stream) { return -1; }

    *mat = (Mat){
        .ftype = ftype,
        .in_stream = in_stream,
        .show_warnings = show_warnings,
        .token    = Done,
        .variable = 0,
        .number   = 0,
        .strict_reporting = strict_reporting,
        .fail = NULL,
    };

    return 0;
}

bool step_mat(Mat *mat) {
    char *str = mat->in_stream+mat->offset;
    char next_char = *str;
    if (next_char == '\0') {
        mat->token = Done;
        return false;
    }
    if (mat->strict_reporting && mat->token == Fail) {
        printf(MAT_ERROR"Fatal Error: number: %f, variable: '%c', offset: %d",
            mat->number, mat->variable, mat->offset);
        return false;
    }

    switch (next_char) {
        // WARNING: This only works with GCC and Clang
        case '0' ... '9': case '.': {
            char *end = NULL;
            mat->number = strtod(str, &end);
            mat->offset = end-mat->in_stream;
            mat->token = Number;
            return true;
        } break;
        case 'x': {
            mat->token = Variable;
            mat->offset++;
            mat->variable = next_char;
            return true;
        } break;
        case '\\': {
            char *cstr = str+1;
            StringSlice word = get_word(cstr);
            Token t = get_word_token(word);
            if (t == Fail) {
                printf(MAT_WARN"Unhandled: \\'" SSF"'\n", word.len, word.data);
                return false;
            }
            mat->token = t;
            mat->offset += word.len+1;
            return true;
        } break;
        case CASE_BRACES: case CASE_SINGLS: {;
            mat->token = (Token)next_char;
            mat->offset++;
            return true;
        } break;
        defualt: break;
    }
    printf(MAT_WARN"Unexpected: '%c'\n", next_char);
    mat->token = Fail;
    mat->fail =(char*)MAT_ERROR"Fail";
    mat->offset++;
    return true;
}

StringSlice get_word(char *str) {
    uint len = 0;

    for (char next = *str; next >= 'a' && next <= 'z'; len++) {
        next = *(str+len);
    }

    return (StringSlice){
        .data = str,
        .len = len-1,
    };
}

Token get_word_token(StringSlice ss) {
    for (int i = 0; i < WORD_TOKENS_MAP_LEN; ++i) {
        if (ss_strcmp(ss, WORD_TOKENS_MAP.cstrs[i])) {
            return WORD_TOKENS_MAP.tokens[i];
        }
    }
    return Fail;
}

inline
bool ss_strcmp(const StringSlice ss, const char *cs) {
    return memcmp(ss.data, cs, ss.len) == 0 ? true : false;
}

const char *token_to_cstr(const Token token) {
    switch (token) {
        case Fail:         { return "Fail";      } break;
        case Done:         { return "Done";      } break;

        case Variable:     { return "Variable";  } break;
        case Number:       { return "Number";    } break;
        case E:            { return "e";         } break;
        case Imaginary:    { return "i";         } break;
        case Pi:           { return "π (pi)";    } break;
        case Inf:          { return "∞ (infty)"; } break;
        case Factorial:    { return "!";         } break;
        case Cos:          { return "cos";       } break;
        case Sin:          { return "sin";       } break;
        case Log:          { return "log";       } break;
        case Ln:           { return "ln";        } break;

        case Equal:        { return "=";         } break;
        case Add:          { return "+";         } break;
        case Subtract:     { return "-";         } break;
        case Exponent:     { return "Exponent";  } break;
        case Fraction:     { return "Fraction";  } break;
        case SubScript:    { return "SubScript"; } break;
        case SQRT:         { return "sqrt";      } break;
        case Less:         { return "<";         } break;
        case Greater:      { return ">";         } break;
        case LessEqual:    { return "<=";        } break;
        case GreaterEqual: { return ">=";        } break;
        case NotEqual:     { return "!=";        } break;
        case Prime:        { return "Prime (`)"; } break;
        case Lim:          { return "Lim";       } break;
        case To:           { return "To (->)";   } break;

        case OParen:       { return "(";         } break;
        case CParen:       { return ")";         } break;
        case OCurly:       { return "{";         } break;
        case CCurly:       { return "}";         } break;
        case OBracket:     { return "[";         } break;
        case CBracket:     { return "]";         } break;
    }
}



#endif
