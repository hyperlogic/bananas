enum token_type { SYMBOL_TOKEN = 0, QUOTE_TOKEN, LPAREN_TOKEN, RPAREN_TOKEN, NUMBER_TOKEN, NUM_TOKENS };

typedef struct {
    enum token_type type;
    const char* start;
    const char* end;
} token_t;

typedef struct {
    token_t* data;
    int max_tokens;
    int num_tokens;
} token_vec_t;

// Don't forget to free the vec.
// NOTE: tokens in the vec point directly into the source string.
token_vec_t* lex(const char* string);
void token_vec_free(token_vec_t* vec);
