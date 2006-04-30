#ifndef CONF_H__
#define CONF_H__

/* Where the global config resides. */
#ifndef GLOBAL_CONF
#define GLOBAL_CONF			"/etc/ondirrc"
#endif
/* If your code blocks exceed this, something weird is going on. */
#define MAX_SECTION_SIZE	16384
/* Use .onenter or .onleave? */
/* #define USE_ONENTERLEAVE */

typedef enum {
	PT_ENTER,
	PT_LEAVE,
} pathtype_t;

struct odpath_t {
	const char **paths, *content;
	int npaths;
	pathtype_t type;
	struct odpath_t *next;
};

/* load ondir configuration file */
struct odpath_t *load_conf(const char *file, struct odpath_t *root);

/* Expand all environment variables in 'in' */
char *expand_envars(const char *in);

void fatal(const char *fmt, ...);
void error(const char *fmt, ...);
void warning(const char *fmt, ...);
void info(const char *fmt, ...);

#endif
