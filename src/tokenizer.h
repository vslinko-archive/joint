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

#ifndef joint_tokenizer_h
#define joint_tokenizer_h

#include "source_file.h"
#include "string.h"

typedef struct joint_source_file_position {
    char * path;
    int position;
    int line;
    int column;
} joint_source_file_position_t;

typedef struct joint_token {
    enum {
        COMMENT_TOKEN,
        KEYWORD_TOKEN,
        IDENTIFIER_TOKEN,
        PUNCTUATOR_TOKEN,
        NUMERIC_LITERAL_TOKEN,
        BOOLEAN_LITERAL_TOKEN,
        CHARACTER_LITERAL_TOKEN,
        STRING_LITERAL_TOKEN,
        NULL_LITERAL_TOKEN,
        EOF_TOKEN
    } type;

    joint_string_t * value;

    joint_source_file_position_t * start_position;
    joint_source_file_position_t * end_position;
} joint_token_t;

typedef struct joint_tokenizer {
    joint_source_file_t * source_file;

    int current_position;
    int current_line_number;
    int current_line_start_position;

    int tokens_length;
    joint_token_t ** tokens;
} joint_tokenizer_t;

joint_tokenizer_t * joint_tokenizer_alloc(joint_source_file_t * source_file);

void joint_tokenizer_tokenize(joint_tokenizer_t * tokenizer);

void joint_tokeinzer_print(const joint_tokenizer_t * tokenizer);

void joint_tokenizer_free(joint_tokenizer_t * tokenizer);

#endif
