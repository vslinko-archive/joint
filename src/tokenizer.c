/* Copyright (c) 2014 Vyacheslav Slinko
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

static const char * joint_tokenizer_token_type_names[] = {
    "COMMENT_TOKEN",
    "KEYWORD_TOKEN",
    "IDENTIFIER_TOKEN",
    "PUNCTUATOR_TOKEN",
    "NUMERIC_LITERAL_TOKEN",
    "BOOLEAN_LITERAL_TOKEN",
    "CHARACTER_LITERAL_TOKEN",
    "STRING_LITERAL_TOKEN",
    "NULL_LITERAL_TOKEN",
    "EOF_TOKEN"
};

static inline bool joint_tokenizer_is_whitespace(char character) {
    return character == 0x20;
}

static inline bool joint_tokenizer_is_lite_terminator(char character) {
    return character == 0x0A;
}

static inline bool joint_tokenizer_is_numeric(char character) {
    return character >= 0x30 && character <= 0x39;
}

static inline bool joint_tokenizer_is_alpha(char character) {
    return (character >= 0x41 && character <= 0x5A) || (character >= 0x61 && character <= 0x7A);
}

static inline bool joint_tokenizer_is_comment_start(char character) {
    return character == 0x23;
}

static inline bool joint_tokenizer_is_identifier_start(char character) {
    return joint_tokenizer_is_alpha(character);
}

static inline bool joint_tokenizer_is_identifier_part(char character) {
    return joint_tokenizer_is_alpha(character) || joint_tokenizer_is_numeric(character);
}

static inline bool joint_tokenizer_is_character_quote(char character) {
    return character == 0x27;
}

static inline bool joint_tokenizer_is_string_quote(char character) {
    return character == 0x22;
}

static joint_source_file_position_t * joint_tokenizer_fixate_position(const joint_tokenizer_t * tokenizer) {
    joint_source_file_position_t * position = malloc(sizeof(joint_source_file_position_t));
    assert(position);

    position->path = malloc(sizeof(char) * (strlen(tokenizer->source_file->path) + 1));
    assert(position->path);
    strcpy(position->path, tokenizer->source_file->path);

    position->position = tokenizer->current_position;
    position->line = tokenizer->current_line_number;
    position->column = tokenizer->current_position - tokenizer->current_line_start_position;

    return position;
}

static void joint_tokenizer_skip_whitespaces(joint_tokenizer_t * tokenizer) {
    char character;

    while (tokenizer->current_position < tokenizer->source_file->content->length) {
        character = tokenizer->source_file->content->data[tokenizer->current_position];

        if (joint_tokenizer_is_whitespace(character)) {
            tokenizer->current_position++;
        } else if (joint_tokenizer_is_lite_terminator(character)) {
            tokenizer->current_position++;
            tokenizer->current_line_number++;
            tokenizer->current_line_start_position = tokenizer->current_position;
        } else {
            break;
        }
    }
}

static void joint_tokenizer_lex_comment(joint_tokenizer_t * tokenizer, joint_token_t * token) {
    char character = tokenizer->source_file->content->data[tokenizer->current_position];
    assert(joint_tokenizer_is_comment_start(character));
    tokenizer->current_position++;

    while (tokenizer->current_position < tokenizer->source_file->content->length) {
        character = tokenizer->source_file->content->data[tokenizer->current_position];

        if (joint_tokenizer_is_lite_terminator(character)) {
            break;
        } else {
            tokenizer->current_position++;
            joint_string_append_character(token->value, character);
        }
    }

    token->type = COMMENT_TOKEN;
}

static void joint_tokenizer_lex_identifier(joint_tokenizer_t * tokenizer, joint_token_t * token) {
    char character = tokenizer->source_file->content->data[tokenizer->current_position];
    assert(joint_tokenizer_is_identifier_start(character));
    tokenizer->current_position++;

    joint_string_append_character(token->value, character);

    while (tokenizer->current_position < tokenizer->source_file->content->length) {
        character = tokenizer->source_file->content->data[tokenizer->current_position];

        if (joint_tokenizer_is_identifier_part(character)) {
            tokenizer->current_position++;
            joint_string_append_character(token->value, character);
        } else {
            break;
        }
    }

    if (strcmp(token->value->data, "let") == 0 || strcmp(token->value->data, "import") == 0 || strcmp(token->value->data, "from") == 0) {
        token->type = KEYWORD_TOKEN;
    } else if (strcmp(token->value->data, "true") == 0 || strcmp(token->value->data, "false") == 0) {
        token->type = BOOLEAN_LITERAL_TOKEN;
    } else if (strcmp(token->value->data, "null") == 0) {
        token->type = NULL_LITERAL_TOKEN;
    } else {
        token->type = IDENTIFIER_TOKEN;
    }
}

static void joint_tokenizer_lex_numeric_literal(joint_tokenizer_t * tokenizer, joint_token_t * token) {
    char character = tokenizer->source_file->content->data[tokenizer->current_position];
    assert(joint_tokenizer_is_numeric(character));
    tokenizer->current_position++;

    joint_string_append_character(token->value, character);

    while (tokenizer->current_position < tokenizer->source_file->content->length) {
        character = tokenizer->source_file->content->data[tokenizer->current_position];

        if (joint_tokenizer_is_numeric(character)) {
            tokenizer->current_position++;
            joint_string_append_character(token->value, character);
        } else {
            break;
        }
    }

    token->type = NUMERIC_LITERAL_TOKEN;
}

static void joint_tokenizer_lex_character_literal(joint_tokenizer_t * tokenizer, joint_token_t * token) {
    char character = tokenizer->source_file->content->data[tokenizer->current_position];
    assert(joint_tokenizer_is_character_quote(character));
    tokenizer->current_position++;

    character = tokenizer->source_file->content->data[tokenizer->current_position];
    assert(!joint_tokenizer_is_character_quote(character));
    tokenizer->current_position++;
    joint_string_append_character(token->value, character);

    character = tokenizer->source_file->content->data[tokenizer->current_position];
    assert(joint_tokenizer_is_character_quote(character));
    tokenizer->current_position++;

    token->type = CHARACTER_LITERAL_TOKEN;
}

static void joint_tokenizer_lex_string_literal(joint_tokenizer_t * tokenizer, joint_token_t * token) {
    char character = tokenizer->source_file->content->data[tokenizer->current_position];
    assert(joint_tokenizer_is_string_quote(character));
    tokenizer->current_position++;

    while (tokenizer->current_position < tokenizer->source_file->content->length) {
        character = tokenizer->source_file->content->data[tokenizer->current_position];

        if (joint_tokenizer_is_string_quote(character)) {
            tokenizer->current_position++;
            token->type = STRING_LITERAL_TOKEN;
            return;
        } else if (joint_tokenizer_is_lite_terminator(character)) {
            tokenizer->current_position++;
            tokenizer->current_line_number++;
            tokenizer->current_line_start_position = tokenizer->current_position;
        } else {
            tokenizer->current_position++;
        }

        joint_string_append_character(token->value, character);
    }

    assert(NULL);
}

static void joint_tokenizer_lex_punctuator(joint_tokenizer_t * tokenizer, joint_token_t * token) {
    char character = tokenizer->source_file->content->data[tokenizer->current_position];

    switch (character) {
        case 0x25: // %
        case 0x28: // (
        case 0x29: // )
        case 0x2A: // *
        case 0x2B: // +
        case 0x2C: // ,
        case 0x2D: // -
        case 0x2E: // .
        case 0x2F: // /
        case 0x3A: // :
        case 0x3B: // ;
        case 0x3F: // ?
        case 0x5B: // [
        case 0x5D: // ]
        case 0x5E: // ^
        case 0x7B: // {
        case 0x7D: // }
        case 0x7E: // ~
            joint_string_append_character(token->value, character);
            tokenizer->current_position++;
            token->type = PUNCTUATOR_TOKEN;
            return;
    }

    if ((tokenizer->source_file->content->length - tokenizer->current_position) >= 3) {
        char * character3 = malloc(sizeof(char) * 4);
        assert(character3);
        memcpy(character3, tokenizer->source_file->content->data + tokenizer->current_position, 3);
        character3[3] = '\0';
        if (strcmp(character3, ">>>") == 0) {
            tokenizer->current_position += 3;
            joint_string_set_content(token->value, character3);
            token->type = PUNCTUATOR_TOKEN;
            free(character3);
            return;
        }
        free(character3);
    }

    if ((tokenizer->source_file->content->length - tokenizer->current_position) >= 2) {
        char * character2 = malloc(sizeof(char) * 3);
        assert(character2);
        memcpy(character2, tokenizer->source_file->content->data + tokenizer->current_position, 2);
        character2[2] = '\0';
        if (strcmp(character2, "!=") == 0 || (strchr("<>&|=", character2[0]) != NULL && character2[0] == character2[1])) {
            tokenizer->current_position += 2;
            joint_string_set_content(token->value, character2);
            token->type = PUNCTUATOR_TOKEN;
            free(character2);
            return;
        }
        free(character2);
    }

    if (strchr("<>=!&|", character) != NULL) {
        joint_string_append_character(token->value, character);
        tokenizer->current_position++;
        token->type = PUNCTUATOR_TOKEN;
        return;
    }

    assert(NULL);
}

static joint_token_t * joint_tokenizer_lex(joint_tokenizer_t * tokenizer) {
    joint_token_t * token = malloc(sizeof(joint_token_t));
    assert(token);

    token->value = joint_string_alloc(8);
    token->start_position = joint_tokenizer_fixate_position(tokenizer);

    joint_tokenizer_skip_whitespaces(tokenizer);

    if (tokenizer->current_position >= tokenizer->source_file->content->length) {
        token->type = EOF_TOKEN;
        token->end_position = joint_tokenizer_fixate_position(tokenizer);
        return token;
    }

    char character = tokenizer->source_file->content->data[tokenizer->current_position];

    if (joint_tokenizer_is_comment_start(character)) {
        joint_tokenizer_lex_comment(tokenizer, token);
    } else if (joint_tokenizer_is_identifier_start(character)) {
        joint_tokenizer_lex_identifier(tokenizer, token);
    } else if (joint_tokenizer_is_numeric(character)) {
        joint_tokenizer_lex_numeric_literal(tokenizer, token);
    } else if (joint_tokenizer_is_character_quote(character)) {
        joint_tokenizer_lex_character_literal(tokenizer, token);
    } else if (joint_tokenizer_is_string_quote(character)) {
        joint_tokenizer_lex_string_literal(tokenizer, token);
    } else {
        joint_tokenizer_lex_punctuator(tokenizer, token);
    }

    token->end_position = joint_tokenizer_fixate_position(tokenizer);

    return token;
}

joint_tokenizer_t * joint_tokenizer_alloc(joint_source_file_t * source_file) {
    joint_tokenizer_t * tokenizer = malloc(sizeof(joint_tokenizer_t));
    assert(tokenizer);

    tokenizer->source_file = source_file;

    tokenizer->current_position = 0;
    tokenizer->current_line_number = 0;
    tokenizer->current_line_start_position = 0;

    tokenizer->tokens_length = 0;
    tokenizer->tokens = NULL;

    return tokenizer;
}

void joint_tokenizer_tokenize(joint_tokenizer_t * tokenizer) {
    while (true) {
        joint_token_t * token = joint_tokenizer_lex(tokenizer);

        if (tokenizer->tokens_length == 0) {
            tokenizer->tokens = malloc(sizeof(joint_token_t *));
        } else {
            tokenizer->tokens = realloc(tokenizer->tokens, sizeof(joint_token_t *) * (tokenizer->tokens_length + 1));
        }
        assert(tokenizer->tokens);

        tokenizer->tokens[tokenizer->tokens_length++] = token;

        if (token->type == EOF_TOKEN) {
            break;
        }
    }
}

void joint_tokeinzer_print(const joint_tokenizer_t * tokenizer) {
    for (int i = 0; i < tokenizer->tokens_length; i++) {
        joint_token_t * token = tokenizer->tokens[i];

        printf("TYPE: %s\n", joint_tokenizer_token_type_names[token->type]);
        printf("VALUE: \"%s\"\n", token->value->data);
        printf("START: p %d l %d c %d\n", token->start_position->position, token->start_position->line, token->start_position->column);
        printf("END: p %d l %d c %d\n", token->end_position->position, token->end_position->line, token->end_position->column);
        puts("---");
    }
}

void joint_tokenizer_free(joint_tokenizer_t * tokenizer) {
    for (int i = 0; i < tokenizer->tokens_length; i++) {
        joint_string_free(tokenizer->tokens[i]->value);
        free(tokenizer->tokens[i]->start_position->path);
        free(tokenizer->tokens[i]->end_position->path);
        free(tokenizer->tokens[i]->start_position);
        free(tokenizer->tokens[i]->end_position);
        free(tokenizer->tokens[i]);
    }

    if (tokenizer->tokens != NULL) {
        free(tokenizer->tokens);
    }

    free(tokenizer);
}
