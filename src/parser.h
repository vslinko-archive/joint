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

#ifndef joint_parser_h
#define joint_parser_h

#include <stdbool.h>
#include "tokenizer.h"

enum joint_node_type {
    PROGRAM_NODE,
    COMMENT_NODE,
    IDENTIFIER_NODE,
    LITERAL_NODE,
    CALL_EXPRESSION_NODE,
    IMPORT_DECLARATION_NODE,
    VARIABLE_DECLARATION_NODE
};

typedef struct joint_node {
    enum joint_node_type type;
    int childrens_length;
    struct joint_node_children ** childrens;

    joint_source_file_position_t * start_position;
    joint_source_file_position_t * end_position;
} joint_node_t;

typedef struct joint_node_children {
    enum {
        NODE_CHILDREN,
        NUMBER_CHILDREN,
        BOOLEAN_CHILDREN,
        CHARACTER_CHILDREN,
        STRING_CHILDREN,
        NULL_CHILDREN
    } type;

    char * name;

    union {
        joint_node_t * node;
        long double number;
        bool boolean;
        char character;
        char * string;
    };
} joint_node_children_t;

typedef struct joint_parser {
    joint_tokenizer_t * tokenizer;
    int current_token;
    joint_token_t * next_token;
    joint_node_t * program;
} joint_parser_t;

joint_parser_t * joint_parser_alloc(joint_tokenizer_t * tokenizer);

void joint_parser_parse(joint_parser_t * parser);

void joint_parser_print(const joint_parser_t * parser);

void joint_parser_free(joint_parser_t * parser);

#endif
