/**
 **
 ** (C)2025 akstel.org
 **/
#ifndef EMPBX_UTILS_REGEX_H
#define EMPBX_UTILS_REGEX_H
#include <empbx-core.h>
#include <pcre.h>

typedef struct real_pcre empbx_regex_t;

void empbx_regex_free(void *re);

empbx_regex_t *empbx_regex_compile(const char *pattern, int options, const char **errorptr, int *erroroffset, const unsigned char *tables);
int empbx_regex_copy_substring(const char *subject, int *ovector, int stringcount, int stringnumber, char *buffer, int size);
void empbx_perform_substitution(empbx_regex_t *re, int match_count, const char *data, const char *field_data, char *substituted, size_t len, int *ovector);
bool empbx_regex_ast2regex(const char *pat, char *rbuf, size_t len);

int empbx_regex_perform(empbx_regex_t **new_re, const char *field, const char *expression, int *ovector, uint32_t olen);

/**
 * Function to evaluate an expression against a string
 *
 * @param  target The string to find a match in
 * @param  expression The regular expression to run against the string
 * @return Boolean if a match was found or not
*/
empbx_status_t empbx_regex_match(const char *target, const char *expression);

/**
 * Function to evaluate an expression against a string
 *
 * @param target The string to find a match in
 * @param expression The regular expression to run against the string
 * @param partial_match If non-zero returns SUCCESS if the target is a partial match, on successful return, this is set to non-zero if the match was partial and zero if it was a full match
 * @return Boolean if a match was found or not
*/
empbx_status_t empbx_regex_match_partial(const char *target, const char *expression, int *partial);



#endif
