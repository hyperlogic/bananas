enum token_type { SYMBOL_TOKEN = 0, QUOTE_TOKEN, LPAREN_TOKEN, RPAREN_TOKEN, NUMBER_TOKEN, NUM_TOKENS };

typedef struct {
    enum token_type type;
    char* string;
} token_t;

typedef struct {
    token_t* data;
    int max_tokens;
    int num_tokens;
} token_vec_t;

token_vec_t* lex(const char* string);
void token_vec_free(token_vec_t* vec);
