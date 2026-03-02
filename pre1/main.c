/**
 * @authors
 * Odin Bjerke <odin.bjerke@uit.no>
 */

#ifdef LOG_LEVEL
#  undef LOG_LEVEL
#endif /* LOG_LEVEL */

#define LOG_LEVEL LOG_LEVEL_INFO

#include "tests.h"
#include "printing.h"
#include "defs.h"
#include "common.h"
#include "futil.h"
#include "list.h"
#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>


#define ARG_TEST     "--test"
#define ARG_FPATH    "--fpath="
#define CLI_CMD_EXIT ".exit"

/**
 * @brief Enter the interactive part of the program.
 * @param freq_map: map of { key: char *, value: uint32_t }, where key refers to a term
 * and the corresponding value to its frequency.
 * @returns 0 on success, otherwise a negative error code.
 */
static int enter_interactive_cli(map_t *freq_map) {
    printf("Starting interpreter ..\n");
    printf("Exit with \"ctrl+C\" or with the \"%s\" command\n", CLI_CMD_EXIT);
    printf("Type a term followed by enter to search for it\n");

    char lbuf[LINE_MAX];

    while (1) {
        printf("%s>>>%s ", ANSI_COLOR_PUR, ANSI_COLOR_RESET);
        char *term = fgets(lbuf, LINE_MAX, stdin);

        if (term == NULL) {
            printf("Failed to read from stdin: %s\n", strerror(errno));
            return -1;
        }

        size_t input_len = strlen(term);

        if (term[input_len - 1] != '\n') {
            printf("Error: input exceeds max length of %d chars\n", LINE_MAX - 2);
            while (fgetc(stdin) != '\n') {
                /* discard characters until we encounter a newline */
                ;
            }
        } else if (strstr(term, " ") || input_len == 1) {
            printf("Error: type a single term, not separated by spaces, to search for it.\n");
        } else {
            /* convert term to lowercase. Since we use fgets, we know that there is a newline. */
            char *c = term;
            while (*c != '\n') {
                if (isascii(*c)) {
                    *c = tolower(*c);
                }
                c++;
            }
            /* replace newline with string terminator */
            *c = 0;

            /* check for exit command */
            if (strcmp(term, CLI_CMD_EXIT) == 0) {
                break;
            }

            /* search for the term */
            uint32_t *freq = map_get(freq_map, term);
            if (freq == NULL) {
                printf("Term \"%s\" was not found\n", term);
            } else {
                printf("Frequency of %s: %u\n", term, *freq);
            }
        }
        /* else, discard empty line */
    }

    printf("\nExiting ..\n");
    return 0;
}

/**
 * @brief Construct a frequency map from the list of terms.
 * @param terms: list of strings
 * @returns NULL on failure/error, otherwise a newly created map { key: char *, value: uint32_t },
 * where key refers to a term and the corresponding value to its frequency in the list of terms.
 * @note the caller retains ownership of the given list of terms, as well as any allocated strings
 * contained in the list.
 */
static map_t *create_termfreq_map(list_t *terms) {
    map_t *freq_map = map_create((cmp_fn) strcmp, hash_string_fnv1a64);
    if (!freq_map) return NULL;

    list_iter_t *iter = list_createiter(terms);
    if (!iter) {
        map_destroy(freq_map, free);
        return NULL;
    }

    while (list_hasnext(iter)) {
        char *word = list_next(iter);

        uint32_t *freq = map_get(freq_map, word);
        if (freq == NULL) {
            /* First occurrence: allocate a counter and insert */
            freq = malloc(sizeof(uint32_t));
            if (!freq) {
                list_destroyiter(iter);
                map_destroy(freq_map, free);
                return NULL;
            }
            *freq = 1;
            map_insert(freq_map, word, strlen(word) + 1, freq);
        } else {
            (*freq)++;
        }
    }

    list_destroyiter(iter);
    return freq_map;
}

/**
 * @brief Runs the interactive, main part of the program
 * @param fpath: path to a file to to use as input for the index
 * @returns 0 on success, otherwise a negative error code
 */
int app_run_cli(const char *fpath) {
    FILE *infile = fopen(fpath, "r");
    if (!infile) {
        pr_error("Failed to open %s: %s\n", fpath, strerror(errno));
        return -1;
    }

    list_t *terms = list_create((cmp_fn) strcmp);
    if (!terms) {
        pr_error("Failed to create list\n");
        fclose(infile);
        return -2;
    }

    /* tokenize: split at whitespace, include only alphanumeric chars, convert to lowercase */
    int rc = ftokenize(infile, terms, 1, isspace, isalnum, tolower);
    fclose(infile);

    if (rc < 0) {
        return -3;
    }

    if (list_length(terms) == 0) {
        pr_warn(
            "List of terms is empty. If the file contains tokens, list_addlast has not worked. Exiting ..\n");
        list_destroy(terms, free);
        return 0;
    }

    /* Build a map from the list of terms, then destroy the list and all its values. */
    map_t *freq_map = create_termfreq_map(terms);
    list_destroy(terms, free);
    if (!freq_map) {
        return -4;
    }

    /* initialization OK. enter the interactive part of the program. */
    rc = enter_interactive_cli(freq_map);
    map_destroy(freq_map, free);

    return rc;
}

/**
 * @brief Runs tests on implementations relevant to the program
 * @returns 0 on success, otherwise a negative error code
 */
static int app_run_tests() {
    pr_info("[Beginning tests]\n");
    pr_warn("If your list/map implementation contains info/debug prints, temporarily set LOG_LEVEL to "
            "LOG_LEVEL_ERROR (at the top of the file with your implementation) to avoid a "
            "messy output from these tests.\n\n");

    if (test_interface_list()) {
        return -1;
    }
    if (test_interface_map()) {
        return -1;
    }

    pr_info("[All tests passed]\n");
    pr_info(
        "Note: This is not a guarantee your implementations are working, but is a good start. You should add "
        "additional tests to verify edge cases specific to your implementation, in addition to the "
        "general ones just performed.\n"
        "Tip: Write some tests of your own to verify the list iterator is working.\n\n");

    return 0;
}

/**
 * @brief print info describing the program usage to stderr
 */
static void print_usage(char **argv) {
    fprintf(stderr, "Usage: ./%s [%s, %s<fpath>]\n", basename(argv[0]), ARG_TEST, ARG_FPATH);
    fprintf(stderr, "* `%s`: If this flag is present, `test_map` is run.\n", ARG_TEST);
    fprintf(stderr,
            "* `%s<fpath>`: Build a map of terms from the content of the file, then enter "
            "interactive mode.\n",
            ARG_FPATH);
    fprintf(stderr, "---\n");
    fprintf(stderr, "Example A: %s %ssrc/main.c %s\n", argv[0], ARG_FPATH, ARG_TEST);
    fprintf(stderr, "Example B: %s %s\n", argv[0], ARG_TEST);
    fprintf(stderr, "Example C: %s %sdata/oxford_dict.txt", argv[0], ARG_FPATH);
}

/**
 * @brief parse command line arguments
 * @param flag_tests: will be set to 1 if ARG_TEST was found
 * @param flag_fpath: will be set to a path if ARG_FPATH was found
 * @returns 0 on success, or a negative status code on failure.
 */
static int parse_args(int argc, char **argv, int *flag_tests, char **flag_fpath) {
    if (argc < 2 || argc > 3) {
        pr_error("The program requires at least one argument/flag.\n");
        return -1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ARG_TEST) == 0) {
            *flag_tests = 1;
        } else if (strncmp(argv[i], ARG_FPATH, sizeof(ARG_FPATH) - 1) == 0) {
            if (*flag_fpath) {
                pr_error("Multiple %s args present\n", ARG_FPATH);
                return -2;
            }
            *flag_fpath = argv[i] + sizeof(ARG_FPATH) - 1;
        } else {
            pr_error("Unknown argument \"%s\"", argv[i]);
            return -3;
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    int flag_run_tests = 0;
    char *flag_fpath = NULL;

    int args_error = parse_args(argc, argv, &flag_run_tests, &flag_fpath);

    if (args_error) {
        print_usage(argv);
        /* main should return a positive error code, so inverse the response. */
        return args_error * -1;
    }

    /* if the test flag was found, run tests first. */
    if (flag_run_tests && app_run_tests() < 0) {
        return 4;
    }

    /* run the interactive command line app, if a fpath arg was specified. */
    if (flag_fpath && app_run_cli(flag_fpath) < 0) {
        return 5;
    }

    return 0;
}
