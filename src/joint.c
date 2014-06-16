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

#include <argp.h>
#include <stdbool.h>
#include <stdio.h>
#include "source_file.h"
#include "tokenizer.h"
#include "parser.h"

const char * argp_program_version = "joint 0.1";
const char * argp_program_bug_address = "Vyacheslav Slinko <vyacheslav.slinko@gmail.com>";

static struct argp_option options[] = {
    {"tokens", 't', 0, OPTION_ARG_OPTIONAL, "Print tokens and exit"},
    {"ast", 'a', 0, OPTION_ARG_OPTIONAL, "Print AST tree and exit"},
    {0}
};

struct arguments {
    char * source_file_path;
    bool tokens;
    bool ast;
};

static error_t parse_opt(int key, char * arg, struct argp_state * state) {
    struct arguments * arguments = state->input;

    switch (key) {
        case 't':
            arguments->tokens = true;
            break;

        case 'a':
            arguments->ast = true;
            break;

        case ARGP_KEY_NO_ARGS:
            argp_usage(state);
            break;

        case ARGP_KEY_ARG:
            arguments->source_file_path = arg;
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = {
    .options = options,
    .parser = parse_opt,
    .args_doc = "SOURCE_FILE",
    .doc = "The Joint language compiler."
};

static struct arguments arguments = {
    .source_file_path = "-",
    .tokens = false,
    .ast = true
};

int main(int argc, char ** argv) {
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    joint_source_file_t * source_file = joint_source_file_alloc(arguments.source_file_path);

    if (!joint_source_file_read(source_file)) {
        fprintf(stderr, "Unable to read file \"%s\"\n", arguments.source_file_path);
        joint_source_file_free(source_file);
        return 1;
    }

    joint_tokenizer_t * tokenizer = joint_tokenizer_alloc(source_file);
    joint_tokenizer_tokenize(tokenizer);

    if (arguments.tokens) {
        joint_tokeinzer_print(tokenizer);
        joint_tokenizer_free(tokenizer);
        joint_source_file_free(source_file);
        return 0;
    }

    joint_parser_t * parser = joint_parser_alloc(tokenizer);
    joint_parser_parse(parser);

    if (arguments.ast) {
        joint_parser_print(parser);
        joint_parser_free(parser);
        joint_tokenizer_free(tokenizer);
        joint_source_file_free(source_file);
        return 0;
    }

    joint_parser_free(parser);
    joint_tokenizer_free(tokenizer);
    joint_source_file_free(source_file);

    return 0;
}
