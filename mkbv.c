#include <stdio.h>

int main(void) {
	FILE *f = fopen("test.binvox", "wb");

	if(f) {
		unsigned char value, count;

		fprintf(f, "#binvox 1\n");
		fprintf(f, "dim %d %d %d\n", 5, 3, 5);
		fprintf(f, "translate %f %f %f\n", 0.0, 0.0, 0.0);
		fprintf(f, "scale %f\n", 1.0);
		fprintf(f, "data\n");

		//Ligne 1
		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);
		
		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);
		
		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);
		
		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);
		
		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);
		
		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		//Ligne 2
		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		//Ligne 3
		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 5;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		//Ligne 4
		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		//Ligne 5
		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 1;
		count = 1;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		value = 0;
		count = 2;
		fwrite(&value, sizeof(unsigned char), 1, f);
		fwrite(&count, sizeof(unsigned char), 1, f);

		fclose(f);
	}
}
