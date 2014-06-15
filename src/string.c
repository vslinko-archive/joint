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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string.h"

joint_string_t * joint_string_alloc(size_t block_size) {
    joint_string_t * string = malloc(sizeof(joint_string_t));
    assert(string);

    string->data = malloc(sizeof(char) * (block_size + 1));
    assert(string->data);

    string->block_size = block_size;
    string->length = 0;
    string->data[0] = '\0';

    return string;
}

void joint_string_set_content(joint_string_t * string, const char * content) {
    string->length = (int) strlen(content);
    realloc(string->data, sizeof(char) * (string->length / string->block_size * string->block_size + string->block_size + 1));
    assert(string->data);
    strcpy(string->data, content);
}

void joint_string_append_character(joint_string_t * string, char character) {
    if (string->length > 0 && string->length % string->block_size == 0) {
        string->data = realloc(string->data, sizeof(char) * (string->length + string->block_size + 1));
        assert(string->data);
    }

    string->data[string->length++] = character;
    string->data[string->length] = '\0';
}

void joint_string_print(const joint_string_t * string) {
    puts(string->data);
}

void joint_string_free(joint_string_t * string) {
    free(string->data);
    free(string);
}
