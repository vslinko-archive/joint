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
#include "source_file.h"
#include "string.h"

joint_source_file_t * joint_source_file_alloc(const char * path) {
    joint_source_file_t * source_file = malloc(sizeof(joint_source_file_t));
    assert(source_file);

    source_file->path = malloc(sizeof(char) * strlen(path));
    assert(source_file->path);
    strcpy(source_file->path, path);

    source_file->content = joint_string_alloc(512);

    return source_file;
}

bool joint_source_file_read(joint_source_file_t * source_file) {
    FILE * handle = fopen(source_file->path, "r");

    if (handle == NULL) {
        return false;
    }

    char character;

    while (true) {
        character = fgetc(handle);

        if (character == EOF) {
            break;
        }

        joint_string_append_character(source_file->content, character);
    }

    fclose(handle);

    return true;
}

void joint_source_file_print(const joint_source_file_t * source_file) {
    printf("Source file path: %s\n", source_file->path);
    puts("Source file content:");
    joint_string_print(source_file->content);
}

void joint_source_file_free(joint_source_file_t * source_file) {
    joint_string_free(source_file->content);
    free(source_file->path);
    free(source_file);
}
