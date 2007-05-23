#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "conf.h"

static int count_ch(const char *str, int ch) {
int count = 0;

	for (; str && *str; ++str)
		if (*str == ch)
			++count;
	return count;
}

struct odpath_t *load_conf(const char *file, struct odpath_t *root) {
char *buffer = malloc(2048), *section = malloc(16384);
struct odpath_t *path = root, *current = NULL;
FILE *fp;
int line = 0, sec_size = 0;

	if (path) while (path->next) path = path->next;

	if (!(fp = fopen(file, "r"))) {
		free(buffer);
		free(section);
		return root;
	}

	while (fgets(buffer, 2048, fp)) {
	int line_size = strlen(buffer);

		++line;
		/* empty lines and comments are ignored */
		if (!strchr("#\n\r", buffer[0]) && line_size) {
			/* continuation... */
			if (isspace(buffer[0])) {
			char *sol = buffer;

				if (!current) {
					error("%s:%i: data outside section", file, line);
					continue;
				}
				if (sec_size + line_size >= MAX_SECTION_SIZE)
					fatal("section is greater than MAX_SECTION_SIZE (%i)", MAX_SECTION_SIZE);
				/* strip leading spaces */
				while (isspace(*sol)) ++sol;
				strcat(section, sol);
			} else {
			const char *home = getenv("HOME");
			char *tok;
			char **paths;
			int npaths, pathi;

				/* new section... */
				if (current) {
					current->content = strdup(section);
					if (path) {
						path->next = current;
						path = current;
					} else {
						path = current;
						if (!root) root = path;
					}
				}
				if (!(current = calloc(1, sizeof(struct odpath_t))))
					fatal("malloc failed");

				memset(section, 0, MAX_SECTION_SIZE);

				/* read stuff */
				if (!(tok = strtok(buffer, " \t")) || (strcmp(tok, "enter") && strcmp(tok, "leave")))
					fatal("%s:%i: expected enter or leave", file, line);

				if (!strcmp(tok, "enter"))
					current->type = PT_ENTER;
				else
				if (!strcmp(tok, "leave"))
					current->type = PT_LEAVE;

				/* count :'s */
				tok = strtok(NULL, "\n");
				npaths = count_ch(tok, ':') + 1;

				paths = calloc(npaths, sizeof(char*));

				for (pathi = 0, tok = strtok(tok, ":"); pathi < npaths; ++pathi, tok = strtok(NULL, ":")) {
					if (home && tok[0] == '~') {
					char tmppath[strlen(tok) + strlen(home) + 1];

						strcpy(tmppath, home);
						strcat(tmppath, tok + 1);
						paths[pathi] = expand_envars(tmppath);
					} else
						paths[pathi] = expand_envars(tok);
				}
				current->paths = (const char**)paths;
				current->npaths = npaths;
			}
		}
	}
	if (current) {
		current->content = strdup(section);
		if (path) {
			path->next = current;
			path = current;
		} else {
			path = current;
			if (!root) root = path;
		}
	}
	fclose(fp);
	free(section);
	free(buffer);
	return root;
}

void fatal(const char *fmt, ...) {
va_list args;

	fprintf(stderr, "fatal: ");
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
	exit(1);
}

void error(const char *fmt, ...) {
va_list args;

	fprintf(stderr, "error: ");
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}

void warning(const char *fmt, ...) {
va_list args;

	printf("warning: ");
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void info(const char *fmt, ...) {
va_list args;

	printf("ondir: ");
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

/* Expand internal ONDIR environment variables */
char *expand_envars(const char *in) {
int inlen = strlen(in), outmax = inlen * 2, outlen = 0;
char *out = calloc(1, outmax);

	while (*in) {
	const char *mark = in;

		if (*in == '$') {
		int bracketed = 0, varlen = 0;
		const char *varstart;

			++in;
			if (*in == '{') { ++in; bracketed = 1; }
			varstart = in;
			while (in[varlen] && (isalnum(in[varlen]) || in[varlen] == '_'))
				++varlen;
			if (bracketed && in[varlen] != '}')
				in = mark;
			else {
			char var[varlen + 1];
			const char *val;

				strncpy(var, varstart, varlen);
				var[varlen] = 0;

				if (strcmp(var, "ONDIRWD") && (var[0] < '0' || var[0] > '9')) {
					in = mark;
				} else {
					val = getenv(var);
					if (val) {
					int vallen = strlen(val);

						if (outlen + vallen >= outmax) {
							outmax = outmax * 2 + vallen;
							out = realloc(out, outmax);
						}
						strcat(out, val);
						outlen += vallen;
						in = mark + varlen + bracketed * 2 + 1;
						continue;
					} else {
						in = mark;
						out[outlen] = 0;
					}
				}
			}
		}

		/* Add another character */
		if (outlen + 1 >= outmax) {
			outmax *= 2;
			out = realloc(out, outmax);
		}

		out[outlen++] = *in++;
		out[outlen] = 0;
	}
	out[outlen] = 0;
	return out;
}
