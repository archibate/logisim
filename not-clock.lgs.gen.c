#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int N = argv[1] ? atoi(argv[1]) : 16;
	FILE *fp = fopen("not-clock.lgs", "w+");
	fprintf(fp, "; module not-clock");
	for (int n = 1; n < N; n++)
		fprintf(fp, " out%d", n);
	fprintf(fp, "\n");
	for (int n = 1; n < N; n++) {
		for (int i = 0; i < n; i++) {
			fprintf(fp, "%s t%d_%d t%d_%d\n", i ? "link" : "not",
					n, i, n, (i + 1) % n);
		}
		fprintf(fp, "link out%d t%d_0\n", n, n);
	}
	fprintf(fp, "; endmodule\n");
	fclose(fp);
}
