#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define N 233

enum {
	T_NULL = 0,
	T_AND,
	T_OR,
	T_NAND,
	T_NOR,
	T_NOT,
	T_XOR,
	T_NXOR,
	T_LINK,
	/*** Special Command(s) ***/
	T_LOAD,
	T_SUB,
	T_ENDS,
	T_DEF,
	T_SHOW,
	T_RUN,
};

int x1[N], x2[N], *x = x1, *_x = x2, t[N], u[N], v[N], w[N], show[N];
char *sym[N], prefix[256];

int compute(int type, int a, int b, int c)
{
	switch (type) {
	case T_AND:
		return a & b & c;
	case T_OR:
		return a | b | c;
	case T_NAND:
		return !(a & b & c);
	case T_NOR:
		return !(a | b | c);
	case T_NOT:
		return !a;
	case T_XOR:
		return a ^ b ^ c;
	case T_NXOR:
		return !(a ^ b ^ c);
	case T_LINK:
		return a;
	default:
		return 0;
	}
}

int get_door_default_value(int type)
{
	switch (type) {
	case T_AND:
	case T_NAND:
	case T_NXOR:
		return 1;
	default:
		return 0;
	}
}

void show_sep(void)
{
	printf("+------+");
	for (int i = 2; i < N; i++) {
		if (sym[i] && show[i]) {
			for (int j = 0; j < strlen(sym[i]) + 1; j++) {
				printf("-");
			}
			printf("-+");
		}
	}
	printf("\n");
}

void show_title(void)
{
	printf("| #Cy. |");
	for (int i = 2; i < N; i++) {
		if (sym[i] && show[i]) {
			printf(" %s |", sym[i]);
		}
	}
	printf("\n");
}

void do_step(void)
{
	static int step;
	_x[0] = 0;
	_x[1] = 1;
	printf("| %4d |", step++);
	for (int i = 2; i < N; i++) {
		if (t[i])
			_x[i] = compute(t[i], x[u[i]], x[v[i]], x[w[i]]);
		if (sym[i] && show[i])
			printf(" %*d |", strlen(sym[i]), _x[i]);
	}
	printf("\n");
	int *t = _x;
	_x = x;
	x = t;
}

void run(int steps)
{
	show_sep();
	show_title();
	show_sep();
	for (int i = 0; i < steps; i++) {
		do_step();
	}
	show_sep();
}

int get_type(const char *name)
{
	if (0)  return T_NULL;
	else if (!strcmp(name, "and"))
		return T_AND;
	else if (!strcmp(name, "or"))
		return T_OR;
	else if (!strcmp(name, "nand"))
		return T_NAND;
	else if (!strcmp(name, "nor"))
		return T_NOR;
	else if (!strcmp(name, "not"))
		return T_NOT;
	else if (!strcmp(name, "xor"))
		return T_XOR;
	else if (!strcmp(name, "nxor"))
		return T_NXOR;
	else if (!strcmp(name, "link"))
		return T_LINK;
	else if (!strcmp(name, "load"))
		return T_LOAD;
	else if (!strcmp(name, "sub"))
		return T_SUB;
	else if (!strcmp(name, "ends"))
		return T_ENDS;
	else if (!strcmp(name, "def"))
		return T_DEF;
	else if (!strcmp(name, "show"))
		return T_SHOW;
	else if (!strcmp(name, "run"))
		return T_RUN;
	fprintf(stderr, "error: bad instruction `%s`\n", name);
	return T_NULL;
}

int alloc_id_for(const char *name)
{
	for (int i = 2; i < N; i++) {
		if (!sym[i]) {
			sym[i] = malloc(strlen(prefix) + strlen(name) + 1);
			strcpy(sym[i], prefix);
			strcat(sym[i], name);
			return i;
		}
	}
	fprintf(stderr, "error: cannot alloc id for `%s`\n", name);
}

int get_name_id(const char *name)
{
	if (isdigit(*name))
		return atoi(name);
	for (int i = 2; i < N; i++) {
		if (sym[i] && !strncmp(sym[i], prefix, strlen(prefix)) &&
				!strcmp(sym[i] + strlen(prefix), name))
			return i;
	}
	//fprintf(stderr, "warning: implicit declaration of `%s`\n", name);
	return alloc_id_for(name);
}

void set_sub_prefix(const char *subname)
{
	strcat(prefix, subname);
	strcat(prefix, ".");
}

void unset_sub_prefix(void)
{
	if (strlen(prefix) < 2 || prefix[strlen(prefix) - 1] != '.')
		return;
	int i;
	for (i = strlen(prefix) - 2; i >= 0; i--) {
		if (prefix[i] == '.') {
			prefix[i] = 0;
			return;
		}
	}
	prefix[0] = 0;
}

void parse_load(const char *code)
{
	int load(const char *source);
	char *token, *line, *s = strdup(code);
	int pid, *ps[] = {&pid, u, v, w, 0};
	while ((line = strsep(&s, "\r\n"))) {
		char *comment = strchr(line, ';');
		if (comment) *comment = 0;
		while ((token = strsep(&line, " ,\t")) && !*token);
		if (!token) continue;

		int type = get_type(token);
		if (type == T_LOAD) {
			token = strsep(&line, "");
			load(token);
			continue;
		} else if (type == T_SUB) {
			char *subname = strsep(&line, " ,\t");
			if (!subname)
				goto bad_syntax;
			set_sub_prefix(subname);
			continue;
		} else if (type == T_ENDS) {
			unset_sub_prefix();
			continue;
		} else if (type == T_RUN) {
			token = strsep(&line, " ,\t");
			int steps = token ? atoi(token) : 10;
			run(steps);
			continue;
		} else if (type == T_SHOW) {
			while ((token = strsep(&line, " ,\t"))) {
				int id = get_name_id(token);
				show[id] = 1;
			}
			continue;
		} else if (type == T_DEF) {
			while ((token = strsep(&line, " ,\t"))) {
				get_name_id(token);
			}
			continue;
		}

		pid = 0;
		int **pp = ps;
		while ((token = strsep(&line, " ,\t"))) {
			if (!*token) continue;
			int id = get_name_id(token);
			if (pp != ps && !*pp) {
				fprintf(stderr, "error: too much arguments\n");
				break;
			}
			pid[*pp++] = id;
		}
		if (pid < 2) {
bad_syntax:
			fprintf(stderr, "error: bad syntax\n");
			continue;
		}
		int def_id = get_door_default_value(type);
		while (*pp) {
			pid[*pp++] = def_id;
		}
		t[pid] = type;
	}
}

int load(const char *source)
{
	FILE *fp = source ? fopen(source, "r") : stdin;
	if (!fp) {
		perror(source ? source : "<stdin>");
		return -1;
	}
	int istty = !!ttyname(fileno(fp));
	char buf[256];
	while (!feof(fp)) {
		if (istty) {
			fprintf(stderr, "lgs> ");
			fflush(stderr);
		}
		if (fgets(buf, sizeof(buf), fp))
			parse_load(buf);
	}
	if (istty) {
		fprintf(stderr, "\n");
	}
	fclose(fp);
	return 0;
}

int main(int argc, char **argv)
{
	int escape = 0;
	if (argc <= 1)
		load(NULL);
	else for (int i = 1; i < argc; i++) {
		if (!escape && !strcmp(argv[i], "-c"))
			escape = 2;
		else if (!escape && !strcmp(argv[i], "--"))
			escape = 1;
		else if (escape == 2)
			parse_load(argv[i]);
		else
			load(argv[i]);
	}
}
