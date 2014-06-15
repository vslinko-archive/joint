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
#include "parser.h"

static const char * joint_node_type_names[] = {
    "PROGRAM_NODE",
    "COMMENT_NODE",
    "IDENTIFIER_NODE",
    "LITERAL_NODE",
    "CALL_EXPRESSION_NODE",
    "IMPORT_DECLARATION_NODE",
    "VARIABLE_DECLARATION_NODE"
};

static joint_source_file_position_t * joint_node_clone_position(const joint_source_file_position_t * position) {
    joint_source_file_position_t * cloned_position = malloc(sizeof(joint_source_file_position_t));
    assert(cloned_position);
    cloned_position->path = malloc(sizeof(char) * (strlen(position->path) + 1));
    assert(cloned_position->path);
    strcpy(cloned_position->path, position->path);
    cloned_position->position = position->position;
    cloned_position->line = position->line;
    cloned_position->column = position->column;
    return cloned_position;
}

static joint_node_t * joint_node_alloc(enum joint_node_type type, const joint_source_file_position_t * start_position) {
    joint_node_t * node = malloc(sizeof(joint_node_t));
    node->type = type;
    node->childrens_length = 0;
    node->start_position = joint_node_clone_position(start_position);
    return node;
}

static void joint_node_attach_children(joint_node_t * node, joint_node_children_t * children) {
    if (node->childrens_length == 0) {
        node->childrens = malloc(sizeof(joint_node_children_t *));
    } else {
        node->childrens = realloc(node->childrens, sizeof(joint_node_children_t *) * (node->childrens_length + 1));
    }
    assert(node->childrens);
    node->childrens[node->childrens_length++] = children;
}

static void joint_node_attach_node(joint_node_t * node, char * name, joint_node_t * related) {
    joint_node_children_t * relation = malloc(sizeof(joint_node_children_t));
    assert(relation);
    relation->type = NODE_CHILDREN;
    relation->name = name;
    relation->node = related;
    joint_node_attach_children(node, relation);
}

static void joint_node_attach_number(joint_node_t * node, char * name, long double number) {
    joint_node_children_t * relation = malloc(sizeof(joint_node_children_t));
    assert(relation);
    relation->type = NUMBER_CHILDREN;
    relation->name = name;
    relation->number = number;
    joint_node_attach_children(node, relation);
}

static void joint_node_attach_boolean(joint_node_t * node, char * name, bool boolean) {
    joint_node_children_t * relation = malloc(sizeof(joint_node_children_t));
    assert(relation);
    relation->type = BOOLEAN_CHILDREN;
    relation->name = name;
    relation->boolean = boolean;
    joint_node_attach_children(node, relation);
}

static void joint_node_attach_character(joint_node_t * node, char * name, char character) {
    joint_node_children_t * relation = malloc(sizeof(joint_node_children_t));
    assert(relation);
    relation->type = CHARACTER_CHILDREN;
    relation->name = name;
    relation->character = character;
    joint_node_attach_children(node, relation);
}

static void joint_node_attach_string(joint_node_t * node, char * name, const char * string) {
    joint_node_children_t * relation = malloc(sizeof(joint_node_children_t));
    assert(relation);

    relation->type = STRING_CHILDREN;
    relation->name = name;

    relation->string = malloc(sizeof(char) * (strlen(string) + 1));
    assert(relation->string);
    strcpy(relation->string, string);

    joint_node_attach_children(node, relation);
}

static void joint_node_attach_null(joint_node_t * node, char * name) {
    joint_node_children_t * relation = malloc(sizeof(joint_node_children_t));
    assert(relation);
    relation->type = NULL_CHILDREN;
    relation->name = name;
    joint_node_attach_children(node, relation);
}

static void joint_node_print(const joint_node_t * root, int level) {
    char * indent = malloc(sizeof(char) * level * 4 + 1);
    memset(indent, ' ', level * 4);
    indent[level * 4] = '\0';

    printf("%sTYPE: %s\n", indent, joint_node_type_names[root->type]);
    printf("%sSTART: p %d l %d c %d\n", indent, root->start_position->position, root->start_position->line, root->start_position->column);
    printf("%sEND: p %d l %d c %d\n", indent, root->end_position->position, root->end_position->line, root->end_position->column);

    if (root->childrens_length > 0) {
        printf("%sCHILDRENS:\n", indent);

        for (int i = 0; i < root->childrens_length; i++) {
            printf("%s    %s:", indent, root->childrens[i]->name);

            switch (root->childrens[i]->type) {
                case NODE_CHILDREN:
                    printf("\n");
                    joint_node_print(root->childrens[i]->node, level + 2);
                    break;

                case NUMBER_CHILDREN:
                    printf(" %Lf\n", root->childrens[i]->number);
                    break;

                case BOOLEAN_CHILDREN:
                    if (root->childrens[i]->boolean) {
                        printf(" true\n");
                    } else {
                        printf(" false\n");
                    }
                    break;

                case CHARACTER_CHILDREN:
                    printf(" '%c'\n", root->childrens[i]->character);
                    break;

                case STRING_CHILDREN:
                    printf(" \"%s\"\n", root->childrens[i]->string);
                    break;

                case NULL_CHILDREN:
                    printf(" null\n");
                    break;
            }
        }
    } else {
        printf("%sCHILDRENS: ~\n", indent);
    }

    free(indent);
}

static void joint_node_free(joint_node_t * node) {
    for (int i = 0; i < node->childrens_length; i++) {
        switch (node->childrens[i]->type) {
            case NODE_CHILDREN:
                joint_node_free(node->childrens[i]->node);
                break;

            case STRING_CHILDREN:
                free(node->childrens[i]->string);
                break;

            default:
                break;
        }

        free(node->childrens[i]);
    }

    if (node->childrens != NULL) {
        free(node->childrens);
    }

    free(node->start_position->path);
    free(node->start_position);
    free(node->end_position->path);
    free(node->end_position);
    free(node);
}

static inline bool joint_parser_match_punctuator(const joint_parser_t * parser, const char * punctuator) {
    return parser->next_token->type == PUNCTUATOR_TOKEN && strcmp(parser->next_token->value->data, punctuator) == 0;
}

static void joint_parser_get_next_token(joint_parser_t * parser) {
    if (parser->current_token + 1 >= parser->tokenizer->tokens_length) {
        return;
    }

    parser->current_token++;
    parser->next_token = parser->tokenizer->tokens[parser->current_token];
}

static void joint_parser_expect_keyword(joint_parser_t * parser, const char * keyword) {
    assert(parser->next_token->type == KEYWORD_TOKEN);
    assert(strcmp(parser->next_token->value->data, keyword) == 0);
    joint_parser_get_next_token(parser);
}

static void joint_parser_expect_punctuator(joint_parser_t * parser, const char * punctuator) {
    assert(parser->next_token->type == PUNCTUATOR_TOKEN);
    assert(strcmp(parser->next_token->value->data, punctuator) == 0);
    joint_parser_get_next_token(parser);
}

static joint_node_t * joint_parser_parse_comment(joint_parser_t * parser) {
    assert(parser->next_token->type == COMMENT_TOKEN);
    joint_node_t * comment = joint_node_alloc(COMMENT_NODE, parser->next_token->start_position);
    comment->end_position = joint_node_clone_position(parser->next_token->end_position);
    joint_node_attach_string(comment, "value", parser->next_token->value->data);
    joint_parser_get_next_token(parser);
    return comment;
}

static joint_node_t * joint_parser_parse_identifier(joint_parser_t * parser) {
    assert(parser->next_token->type == IDENTIFIER_TOKEN);
    joint_node_t * indentifier = joint_node_alloc(IDENTIFIER_NODE, parser->next_token->start_position);
    indentifier->end_position = joint_node_clone_position(parser->next_token->end_position);
    joint_node_attach_string(indentifier, "value", parser->next_token->value->data);
    joint_parser_get_next_token(parser);
    return indentifier;
}

static joint_node_t * joint_parser_parse_literal(joint_parser_t * parser) {
    joint_node_t * literal = joint_node_alloc(LITERAL_NODE, parser->next_token->start_position);
    literal->end_position = joint_node_clone_position(parser->next_token->end_position);
    long double value;

    switch (parser->next_token->type) {
        case NUMERIC_LITERAL_TOKEN:
            sscanf(parser->next_token->value->data, "%Lf", &value);
            joint_node_attach_number(literal, "value", value);
            break;

        case BOOLEAN_LITERAL_TOKEN:
            joint_node_attach_boolean(literal, "value", strcmp(parser->next_token->value->data, "true") == 0);
            break;

        case CHARACTER_LITERAL_TOKEN:
            joint_node_attach_character(literal, "value", parser->next_token->value->data[0]);
            break;

        case STRING_LITERAL_TOKEN:
            joint_node_attach_string(literal, "value", parser->next_token->value->data);
            break;

        case NULL_LITERAL_TOKEN:
            joint_node_attach_null(literal, "value");
            break;

        default:
            assert(NULL);
    }

    joint_parser_get_next_token(parser);

    return literal;
}

static joint_node_t * joint_parser_parse_primary_expression(joint_parser_t * parser) {
    switch (parser->next_token->type) {
        case IDENTIFIER_TOKEN:
            return joint_parser_parse_identifier(parser);

        case NUMERIC_LITERAL_TOKEN:
        case BOOLEAN_LITERAL_TOKEN:
        case CHARACTER_LITERAL_TOKEN:
        case STRING_LITERAL_TOKEN:
        case NULL_LITERAL_TOKEN:
            return joint_parser_parse_literal(parser);

        default:
            assert(NULL);
    }
}

static joint_node_t * joint_parser_parse_call_expression(joint_parser_t * parser, joint_node_t * callee) {
    joint_node_t * call_expression = joint_node_alloc(CALL_EXPRESSION_NODE, callee->start_position);
    joint_node_attach_node(call_expression, "callee", callee);

    joint_parser_expect_punctuator(parser, "(");

    while (true) {
        if (joint_parser_match_punctuator(parser, ")")) {
            break;
        }

        joint_node_t * argument = joint_parser_parse_primary_expression(parser);
        joint_node_attach_node(call_expression, "argument", argument);

        if (joint_parser_match_punctuator(parser, ",")) {
            joint_parser_get_next_token(parser);
        } else {
            break;
        }
    }

    call_expression->end_position = joint_node_clone_position(parser->next_token->end_position);

    joint_parser_expect_punctuator(parser, ")");

    return call_expression;
}

static joint_node_t * joint_parser_parse_left_hand_side_expression_allow_call(joint_parser_t * parser) {
    joint_node_t * expression = joint_parser_parse_primary_expression(parser);

    while (joint_parser_match_punctuator(parser, "(")) {
        expression = joint_parser_parse_call_expression(parser, expression);
    }

    free(expression->end_position->path);
    free(expression->end_position);
    expression->end_position = joint_node_clone_position(parser->next_token->end_position);

    joint_parser_expect_punctuator(parser, ";");

    return expression;
}

static joint_node_t * joint_parser_parse_import_declaration(joint_parser_t * parser) {
    joint_node_t * import_declaration = joint_node_alloc(IMPORT_DECLARATION_NODE, parser->next_token->start_position);

    joint_parser_expect_keyword(parser, "import");
    joint_parser_expect_punctuator(parser, "{");

    while (true) {
        joint_node_t * specifier = joint_parser_parse_identifier(parser);
        joint_node_attach_node(import_declaration, "specifier", specifier);

        if (joint_parser_match_punctuator(parser, ",")) {
            joint_parser_get_next_token(parser);
        } else {
            break;
        }
    }

    joint_parser_expect_punctuator(parser, "}");
    joint_parser_expect_keyword(parser, "from");

    assert(parser->next_token->type == STRING_LITERAL_TOKEN);
    joint_node_t * source = joint_parser_parse_literal(parser);
    joint_node_attach_node(import_declaration, "source", source);

    import_declaration->end_position = joint_node_clone_position(parser->next_token->end_position);

    joint_parser_expect_punctuator(parser, ";");

    return import_declaration;
}

static joint_node_t * joint_parser_parse_variable_declaration(joint_parser_t * parser) {
    joint_node_t * variable_declaration = joint_node_alloc(VARIABLE_DECLARATION_NODE, parser->next_token->start_position);

    joint_parser_expect_keyword(parser, "let");

    joint_node_t * id = joint_parser_parse_identifier(parser);
    joint_node_attach_node(variable_declaration, "id", id);

    joint_parser_expect_punctuator(parser, ":");

    joint_node_t * type = joint_parser_parse_identifier(parser);
    joint_node_attach_node(variable_declaration, "type", type);

    if (joint_parser_match_punctuator(parser, "?")) {
        joint_parser_get_next_token(parser);
        joint_node_attach_boolean(variable_declaration, "optional", true);
    } else {
        joint_node_attach_boolean(variable_declaration, "optional", false);
    }

    joint_parser_expect_punctuator(parser, "=");

    joint_node_t * init = joint_parser_parse_primary_expression(parser);
    joint_node_attach_node(variable_declaration, "init", init);

    variable_declaration->end_position = joint_node_clone_position(parser->next_token->end_position);

    joint_parser_expect_punctuator(parser, ";");

    return variable_declaration;
}

static joint_node_t * joint_parser_parse_program_element(joint_parser_t * parser) {
    if (parser->next_token->type == COMMENT_TOKEN) {
        return joint_parser_parse_comment(parser);
    } else if (parser->next_token->type == KEYWORD_TOKEN) {
        if (strcmp(parser->next_token->value->data, "import") == 0) {
            return joint_parser_parse_import_declaration(parser);
        } else if (strcmp(parser->next_token->value->data, "let") == 0) {
            return joint_parser_parse_variable_declaration(parser);
        }
    } else {
        return joint_parser_parse_left_hand_side_expression_allow_call(parser);
    }

    return NULL;
}

static joint_node_t * joint_parser_parse_program(joint_parser_t * parser) {
    joint_node_t * program = joint_node_alloc(PROGRAM_NODE, parser->next_token->start_position);
    joint_node_t * program_element = NULL;

    while (parser->next_token->type != EOF_TOKEN) {
        program_element = joint_parser_parse_program_element(parser);
        joint_node_attach_node(program, "body", program_element);
    }

    if (program_element == NULL) {
        program->end_position = joint_node_clone_position(parser->next_token->end_position);
    } else {
        program->end_position = joint_node_clone_position(program_element->end_position);
    }

    return program;
}

joint_parser_t * joint_parser_alloc(joint_tokenizer_t * tokenizer) {
    assert(tokenizer->tokens_length > 0);

    joint_parser_t * parser = malloc(sizeof(joint_parser_t));
    assert(parser);

    parser->tokenizer = tokenizer;
    parser->current_token = 0;
    parser->next_token = tokenizer->tokens[0];

    return parser;
}

void joint_parser_parse(joint_parser_t * parser) {
    parser->program = joint_parser_parse_program(parser);
}

void joint_parser_print(const joint_parser_t * parser) {
    joint_node_print(parser->program, 0);
}

void joint_parser_free(joint_parser_t * parser) {
    joint_node_free(parser->program);
    free(parser);
}
