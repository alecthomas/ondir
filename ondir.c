#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <regex.h>
#include "conf.h"

/* Declarations */
struct odpath_t *root;

char *substitute(const char *in, const char *var, const char *val);
int add_envar(const char *var, const char *val);
int check_regex(const char *regex, const char *line, regmatch_t match[10]);
struct odpath_t *find_path(struct odpath_t *last, const char *path, pathtype_t type, regmatch_t match[10]);
void usage(const char *msg);

#ifdef USE_ONENTERLEAVE
void display_file(const char *path);
int exec_ok(const char *path);
#endif

/* Definitions */
int main(int argc, const char **argv) {
#ifdef USE_ONENTERLEAVE
char onenter[PATH_MAX + 1];
#endif
char working[PATH_MAX + 1], cwd[PATH_MAX + 1];
int len, i;
const char *src = NULL, *dst = NULL, *home = NULL;

	if (argc < 2)
		usage("Not enough arguments");

	for (i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-V")) {
			printf("This is OnDir version %s\n", VERSION);
			return 0;
		}

		if (argv[i][0] == '-')
			usage("Unknown argument");

		if (!src) src = argv[i];
		else
		if (!dst) dst = strdup(argv[i]);
		else
			usage("You have already specified src and dst directories");
	}

	if (!dst) {
		getcwd(cwd, PATH_MAX);
		dst = cwd;
	}
	len = strlen(dst);
	// If `dst` ends with '/', trim to simplify checks in loop head
	if (len && dst[len] == '/')
		((char*)dst)[len] = 0;

	if (src[0] != '/' || dst[0] != '/')
		fatal("either the source or destination directory is not absolute");

	root = load_conf(GLOBAL_CONF, root);

	if ((home = getenv("HOME"))) {
		snprintf(working, PATH_MAX, "%s/.ondirrc", home);
		root = load_conf(working, root);
	}

	/*
		The basic algorithm is thus:

		1. Traverse up source path until it matches the lead of dest path 
		2. Traverse from the join to the end of dest path
	*/

	strcpy(working, src);
	len = strlen(working);

	/* Traverse up source path */
	while (strncmp(working, dst, len) || (dst[len] && dst[len] != '/')) {
	regmatch_t match[10];
	struct odpath_t *p;

		for (p = find_path(NULL, working, PT_LEAVE, match); p;
			 p = find_path(p, working, PT_LEAVE, match)) {
		char *sub;

			/* Construct temporary envars */
			add_envar("ONDIRWD", working);
			for (i = 0; match[i].rm_so != -1 && i < 10; ++i) {
			int mlen = match[i].rm_eo - match[i].rm_so;
			char var[4], val[mlen];

				sprintf(var, "%i", i);
				strncpy(val, working + match[i].rm_so, mlen);
				val[mlen] = 0;
				add_envar(var, val);
			}
			/* Expand envars */
			sub = expand_envars(p->content);
			/* Destroy temporary envars */
			putenv("ONDIRWD");
			for (i = 0; match[i].rm_so != -1 && i < 10; ++i) {
			char var[4];

				sprintf(var, "%i", i);
				putenv(var);
			}
			printf("%s\n", sub);
			free(sub);
			if (p->final)
				break;
		}
#ifdef USE_ONENTERLEAVE
		snprintf(onenter, PATH_MAX, "%s/.onleave", working);
		if (exec_ok(onenter))
			display_file(onenter);
#endif
		while (len && working[len] != '/')
			--len;
		working[len] = 0;
	}

	if (dst[len]) ++len;

	/* Move up to destination */
	while (dst[len] != 0) {
	regmatch_t match[10];
	struct odpath_t *p;

		while (dst[len] != 0 && dst[len] != '/')
			++len;
		strncpy(working, dst, len);
		working[len] = 0;
		for (p = find_path(NULL, working, PT_ENTER, match); p;
			 p = find_path(p, working, PT_ENTER, match)) {
		char *sub;
		int i = 0;

			/* Construct temporary envars */
			add_envar("ONDIRWD", working);
			for (i = 0; match[i].rm_so != -1 && i < 10; ++i) {
			int mlen = match[i].rm_eo - match[i].rm_so;
			char var[4], val[mlen];

				sprintf(var, "%i", i);
				strncpy(val, working + match[i].rm_so, mlen);
				val[mlen] = 0;
				add_envar(var, val);
			}
			/* Expand envars */
			sub = expand_envars(p->content);
			/* Destroy temporary envars */
			putenv("ONDIRWD");
			for (i = 0; match[i].rm_so != -1 && i < 10; ++i) {
			char var[4];

				sprintf(var, "%i", i);
				putenv(var);
			}
			printf("%s", sub);
			free(sub);
			if (p->final)
				break;
		}
#ifdef USE_ONENTERLEAVE
		snprintf(onenter, PATH_MAX, "%s/.onenter", working);
		if (exec_ok(onenter))
			display_file(onenter);
#endif
		if (dst[len] == '/') ++len;
	}

	if (dst && dst != cwd)
		free((void*)dst);
	return 0;
}

char *substitute(const char *in, const char *var, const char *val) {
int inlen = strlen(in), outmax = inlen * 2, varlen = strlen(var), 
	vallen = strlen(val), outlen = 0;
char *out = calloc(1, outmax),
	*var1 = calloc(1, varlen + 2),
	*var2 = calloc(1, varlen + 4),
	*offset1 = NULL, *offset2 = NULL;

	/* Construct $VAR */
	strcpy(var1, "$"); strcat(var1, var);

	/* Construct ${VAR} */
	strcpy(var2, "${"); strcat(var2, var); strcat(var2, "}");

	/* Do the replacement */
	while ((offset1 = strstr(in, var1)) || (offset2 = strstr(in, var2))) {
	char *offset = offset1;
	int vlen = varlen + 1;

		if (!offset || (offset2 && offset2 < offset1)) offset = offset2;

		if (offset[1] == '{') vlen += 2;

		if (outlen + vlen > outmax) {
			outmax *= 2;
			out = realloc(out, outmax);
		}

		out[outlen] = 0;

		/* Add difference between beginning of 'in' and 'offset' */
		strncat(out, in, offset - in);
		outlen += offset - in;
		in = offset;

		out[outlen] = 0;

		strcat(out, val);
		outlen += vallen;
		in += vlen;

		offset1 = offset2 = NULL;
	}

	strcat(out, in);
	return out;
}

/* Check if a given regex matches a given line.
 *
 * Returns:
 * 0 if they don't match.
 * 1 if they match.
 */
int check_regex(const char *regex, const char *line, regmatch_t match[10]) {
regex_t preg;
int retval = 0;

	if (!regex || !line) {
		/* FAILURE */
		return (0);
	}

	if ((retval = regcomp(&preg, regex, REG_EXTENDED))) {
	char err_buf[512];

		regerror(retval, &preg, err_buf, 512);
		fprintf(stderr, "ondir: regcomp failed, %s\n", err_buf);
		return 0;
	}

	retval = regexec(&preg, line, 10, match, 0);
	regfree(&preg);

	return retval == 0 && match[0].rm_so == 0 && match[0].rm_eo == strlen(line);
}

struct odpath_t *find_path(struct odpath_t *last, const char *path, pathtype_t type, regmatch_t match[10]) {
struct odpath_t *i;
int j;

	if (last)
		last = last->next;
	else
		last = root;

	for (i = last; i != NULL; i = i->next)
		if (i->type == type) {
			for (j = 0; j < i->npaths; ++j)
				if (!strcmp(i->paths[j], path)) {
					/* Emulate regex sub-pattern capturing */
					match[0].rm_so = 0;
					match[0].rm_eo = strlen(path);
					match[1].rm_so = match[1].rm_eo = -1;
					return i;
				} else
				if (check_regex(i->paths[j], path, match))
					return i;
		}
	return NULL;
}

int add_envar(const char *var, const char *val) {
char *set = malloc(strlen(var) + strlen(val) + 2);

	strcpy(set, var);
	strcat(set, "=");
	strcat(set, val);
	return putenv(set);
}

#ifdef USE_ONENTERLEAVE
void display_file(const char *path) {
FILE *fp;

	if ((fp = fopen(path, "r"))) {
	char buffer[512];
	int len;

		while ((len = fread(buffer, 1, 512, fp)) > 0)
			fwrite(buffer, len, 1, stdout);
		fclose(fp);
	}
}

int exec_ok(const char *path) {
struct stat s;

	return
		stat(path, &s) == 0 && /* exists.. */
		s.st_mode & (S_IRUSR | S_IRGRP | S_IROTH) && /* ...readable... */
		s.st_uid == getuid() && /* ...owned by me... */
		!(s.st_mode & (S_IWGRP | S_IWOTH)); /* ...not group/world writeable */
}
#endif

void usage(const char *msg) {
	printf(
		"%s\n"
		"usage: ondir [-V] <old-directory> [<new-directory>]\n"
		"\n"
		"  -V    Displays version information.\n"
		"\n"
		"  <old-directory> is the last working directory.\n"
		"\n"
		"  <new-directory> is the current working directory. If <new-directory> is\n"
		"  omitted, the current working directory is obtained via a call to getcwd().\n"
		"\n", msg
	);
	exit(1);
}

