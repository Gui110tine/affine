#include <stdio.h>

int main() {

	FILE * fp = fopen("parsed_blocks.txt", "r");
	int temp, ret;
	size_t total = 0;

	ret = fscanf(fp, "%d", &temp);
	while (ret && temp) {
		total += temp;
		ret = fscanf(fp, "%d", &temp);
	}

	fclose(fp);

	fprintf(stdout, ", %f", total*512.0/(16.*1024*1024*1024));

	return 0;
}
