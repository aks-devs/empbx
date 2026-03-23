/**
 ** based on the Freeswitch regex
 **
 ** (C)2026 akstel.org
 **/

#include <empbx.h>

empbx_regex_t *empbx_regex_compile(const char *pattern, int options, const char **errorptr, int *erroroffset, const unsigned char *tables) {

    return (empbx_regex_t *)pcre_compile(pattern, options, errorptr, erroroffset, tables);
}

int empbx_regex_copy_substring(const char *subject, int *ovector, int stringcount, int stringnumber, char *buffer, int size) {

    return pcre_copy_substring(subject, ovector, stringcount, stringnumber, buffer, size);
}

void empbx_regex_free(void *re) {
    if(re) {
        pcre_free(re);
    }
}

empbx_status_t empbx_regex_match(const char *target, const char *expression) {
	int partial = 0;
	return empbx_regex_match_partial(target, expression, &partial);
}

bool empbx_regex_ast2regex(const char *pat, char *rbuf, size_t len) {
    const char *p = pat;

    if(zstr(pat)) {
        return false;
    }

    memset(rbuf, 0, len);

    *(rbuf + strlen(rbuf)) = '^';
    while (p && *p) {
        if (*p == 'N') {
            strncat(rbuf, "[2-9]", len - strlen(rbuf));
        } else if (*p == 'X') {
            strncat(rbuf, "[0-9]", len - strlen(rbuf));
        } else if (*p == 'Z') {
            strncat(rbuf, "[1-9]", len - strlen(rbuf));
        } else if (*p == '.') {
            strncat(rbuf, ".*", len - strlen(rbuf));
        } else if (strlen(rbuf) < len - 1) {
            *(rbuf + strlen(rbuf)) = *p;
        }
        p++;
    }

    *(rbuf + strlen(rbuf)) = '$';
    return strcmp(pat, rbuf) ? true : false;
}

int empbx_regex_perform(empbx_regex_t **new_re, const char *field, const char *expression, int *ovector, uint32_t olen) {
	const char *error = NULL;
	int erroffset = 0;
	pcre *re = NULL;
	int match_count = 0;
	char *tmp = NULL;
	uint32_t flags = 0;
	char abuf[256] = "";

	if(!(field && expression)) {
		return 0;
	}

	if(*expression == '_') {
        if(empbx_regex_ast2regex(expression + 1, abuf, sizeof(abuf))) {
			expression = abuf;
		}
	}

	if(*expression == '/') {
		char *opts = NULL;

		if(str_dup(&tmp, expression + 1)) {
            log_mem_fail();
            goto out;
		}

		if((opts = strrchr(tmp, '/'))) {
			*opts++ = '\0';
		} else {
			log_error("Expression error (%s) [missing ending '/' delimeter]", expression);
			goto out;
		}

		expression = tmp;
		if(*opts) {
			if(strchr(opts, 'i')) { flags |= PCRE_CASELESS; }
			if(strchr(opts, 's')) { flags |= PCRE_DOTALL; }
		}
	}

	re = pcre_compile(expression,	/* the pattern */
					  flags,	    /* default options */
					  &error,	    /* for error message */
					  &erroffset,	/* for error offset */
					  NULL);	    /* use default character tables */
	if(error) {
		log_error("Regex compilation error: %d [%s][%s]\n", erroffset, error, expression);
		empbx_regex_free(re);
		goto out;
	}

	match_count = pcre_exec(re,	                /* result of pcre_compile() */
							NULL,	            /* we didn't study the pattern */
							field,	            /* the subject string */
							(int) strlen(field),/* the length of the subject string */
							0,	                /* start at offset 0 in the subject */
							0,	                /* default options */
							ovector,	        /* vector of integers for substring information */
							olen);	            /* number of elements (NOT size in bytes) */


	if(match_count <= 0) {
		empbx_regex_free(re);
		match_count = 0;
	}

	*new_re = (empbx_regex_t *) re;

out:
	mem_deref(tmp);
	return match_count;
}

void empbx_perform_substitution(empbx_regex_t *re, int match_count, const char *data, const char *field_data, char *substituted, size_t len, int *ovector) {
	char index[10] = "";
	const char *replace = NULL;
	size_t x, y = 0, z = 0;
	int num = 0;
	int brace;

	for(x = 0; y < (len - 1) && x < strlen(data);) {
		if(data[x] == '$') {
			x++;

			brace = data[x] == '{';
			if(brace) {
				x++;
			}

			if(!(data[x] > 47 && data[x] < 58)) {
				x -= brace;
				substituted[y++] = data[x - 1];
				continue;
			}

			while(data[x] > 47 && data[x] < 58 && z < sizeof(index) - 1) {
				index[z++] = data[x];
				x++;
			}
			if(brace) {
				if(data[x] != '}') {
					x -= z - 1;
					substituted[y++] = data[x - 1];
					continue;
				}
				else {
					x++;
				}
			}
			index[z++] = '\0';
			z = 0;
			num = atoi(index);

			if (num < 0 || num > 256) {
				num = -1;
			}

			if(pcre_get_substring(field_data, ovector, match_count, num, &replace) >= 0) {
				if(replace) {
					size_t r = 0;
					for(r = 0; r < strlen(replace) && y < (len - 1); r++) {
						substituted[y++] = replace[r];
					}
					pcre_free_substring(replace);
				}
			}
		} else {
			substituted[y++] = data[x];
			x++;
		}
	}
	substituted[y++] = '\0';
}


empbx_status_t empbx_regex_match_partial(const char *target, const char *expression, int *partial) {
	empbx_status_t status = EMPBX_STATUS_FALSE;
	const char *error = NULL;	/* Used to hold any errors                                           */
	int error_offset = 0;		/* Holds the offset of an error                                      */
	pcre *pcre_prepared = NULL;	/* Holds the compiled regex                                          */
	int match_count = 0;		/* Number of times the regex was matched                             */
	int offset_vectors[255];	/* not used, but has to exist or pcre won't even try to find a match */
	int pcre_flags = 0;
	uint32_t flags = 0;
	char *tmp = NULL;

	if(*expression == '/') {
		char *opts = NULL;

		if(str_dup(&tmp, expression + 1)) {
            mem_fail_goto_status(EMPBX_STATUS_MEM_FAIL, out);
		}

		if((opts = strrchr(tmp, '/'))) {
			*opts++ = '\0';
		} else {
			log_error("Expression error (%s) (missing ending '/' delimeter)", expression);
			goto out;
		}

		expression = tmp;
		if(*opts) {
			if(strchr(opts, 'i')) { flags |= PCRE_CASELESS; }
			if(strchr(opts, 's')) { flags |= PCRE_DOTALL; }
		}
	}

	/* Compile the expression */
	pcre_prepared = pcre_compile(expression, flags, &error, &error_offset, NULL);

	/* See if there was an error in the expression */
	if(error != NULL) {
		if (pcre_prepared) {
			pcre_free(pcre_prepared);
			pcre_prepared = NULL;
		}

		log_error("Expression error (%s) (error=[%s] location=[%d]))", expression, error, error_offset);
		goto out;
	}

	if(*partial) {
		pcre_flags = PCRE_PARTIAL;
	}

	/* So far so good, run the regex */
	match_count = pcre_exec(pcre_prepared, NULL, target, (int) strlen(target), 0, pcre_flags, offset_vectors, sizeof(offset_vectors) / sizeof(offset_vectors[0]));

	/* Clean up */
	if(pcre_prepared) {
		pcre_free(pcre_prepared);
		pcre_prepared = NULL;
	}

	/* Was it a match made in heaven? */
	if(match_count > 0) {
		*partial = 0;
		empbx_goto_status(EMPBX_STATUS_SUCCESS, out);
	} else if (match_count == PCRE_ERROR_PARTIAL || match_count == PCRE_ERROR_BADPARTIAL) {
		*partial = 1;   /* yes it is already set, but the code is clearer this way */
		empbx_goto_status(EMPBX_STATUS_SUCCESS, out);
	}

out:
	mem_deref(tmp);
	return status;
}


