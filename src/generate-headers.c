/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <error.h>

typedef struct {
	int size;
	int last_symbol;
	int num_symbols;
	char **symbols;
} symbol_table_t;

void version() {
	printf
	    ("%s %s\n"
	     "Copyright (C) 2014 Hein-Pieter van Braam <hp@tmm.cx>.\n"
	     "License GPLv3+: GNU GPL version 3 or later "
	     "<http://gnu.org/licenses/gpl.html>.\n"
	     "This is free software: you are free to change and redistribute it.\n"
	     "There is NO WARRANTY, to the extent permitted by law.\n"
	     "\n"
	     "Written by Hein-Pieter van Braam.\n", PACKAGE, PACKAGE_VERSION);
	exit(0);
}

void usage(const char *progname) {
	printf
	    ("Usage: %s INFILE [--output OUTFILE] [--table-name NAME] [--append]\n"
	     "Generate a table of symbolic names for syscall numbers from INFILE.\n"
	     "The table in the generated header is named NAME.\n"
	     "\n"
	     "Options:\n"
	     "  -o, --output OUTFILE  Write output to this file.\n"
	     "  -t, --table NAME      Name the table NAME in the generated header.\n"
	     "  -a, --append          Append to OUTFILE.\n"
	     "  -h, --help            Print this help screen.\n"
	     "      --version         Output version information and exit.\n"
	     "\n"
	     "With no INFILE, or when INFILE is -, read standard input.\n"
	     "With no OUTFILE, or when OUTFILE is -, write to standard output.\n"
	     "With no NAME, name the table 'syscall_list'.\n"
	     "\n" "Report bugs to <hp@tmm.cx>.\n", progname);
	exit(0);
}

void open_file(FILE ** outfp, const char *filename, const char *mode) {
	FILE *fp;

	if (strlen(filename) == 1 && filename[0] == '-')
		return;

	if (!(fp = fopen(filename, mode)))
		error(1, errno, "cannot access %s", filename);

	*outfp = fp;
}

void write_file(const char *filename, FILE * fp, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	if (vfprintf(fp, fmt, args) < 0)
		error(1, errno, "error writing %s", filename);

	va_end(args);
}

void symbol_table_insert(symbol_table_t * symbol_table, int num, char *sym) {
	int i;

	while (num >= symbol_table->size) {
		symbol_table->symbols =
		    realloc(symbol_table->symbols,
			    (symbol_table->size * 2) * sizeof(const char *));
		for (i = symbol_table->size; i < symbol_table->size * 2; ++i) {
			symbol_table->symbols[i] = NULL;
		}
		symbol_table->size *= 2;
	}

	if (num > symbol_table->last_symbol)
		symbol_table->last_symbol = num;

	++symbol_table->num_symbols;
	symbol_table->symbols[num] = sym;
}

int main(int argc, char *argv[]) {
	char *table = NULL;
	char *syscall, *output = NULL, *input = NULL;
	int syscall_nr = 0, c, append = 0;
	FILE *in_fp = stdin, *out_fp = stdout;
	symbol_table_t symbol_table;

	symbol_table.size = 100;
	symbol_table.last_symbol = 0;
	symbol_table.num_symbols = 0;
	symbol_table.symbols = calloc(symbol_table.size, sizeof(const char *));

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"output", required_argument, 0, 'o'},
			{"table", required_argument, 0, 't'},
			{"append", no_argument, 0, 'a'},
			{"version", no_argument, 0, 'v'},
			{"help", optional_argument, 0, 'h'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "o:t:ah", long_options,
				&option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'o':
			output = strdup(optarg);
			break;
		case 't':
			table = strdup(optarg);
			break;
		case 'a':
			append = 1;
			break;
		case 'h':
			usage(argv[0]);
			break;
		case 'v':
			version();
			break;
		case '?':
			fprintf(stderr,
				"Try '%s --help' for more information.\n",
				argv[0]);
			exit(1);
			break;
		}
	}

	if (optind < argc) {
		input = argv[optind];
		open_file(&in_fp, input, "rb");
	} else {
		input = strdup("-");
	}

	if (output) {
		if (append)
			open_file(&out_fp, output, "ab");
		else
			open_file(&out_fp, output, "wb");
	} else {
		output = strdup("-");
	}

	if (!table)
		table = strdup("syscall_list");

	while (!feof(in_fp)) {
		if (fscanf
		    (in_fp, "%*s __NR_%ms %i\n", &syscall, &syscall_nr) != 2) {
			if (errno)
				error(1, errno, "error reading %s", input);

			continue;
		}

		symbol_table_insert(&symbol_table, syscall_nr, syscall);
	}

	if (!symbol_table.num_symbols)
		error(1, 0, "No matching lines found in %s", input);

	write_file(output, out_fp, "const char *%s[] = {\n", table);
	for (c = 0; c <= symbol_table.last_symbol; ++c) {
		if (symbol_table.symbols[c]) {
			write_file(output, out_fp, "\t\"%s\",\n",
				   symbol_table.symbols[c]);
			free(symbol_table.symbols[c]);
		} else {
			write_file(output, out_fp, "\tNULL,\n");
		}
	}
	write_file(output, out_fp, "};\n");

	free(symbol_table.symbols);
	free(table);
	free(output);
	fclose(in_fp);
	fclose(out_fp);

	return 0;
}
