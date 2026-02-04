#include <stdio.h>

void main()
{
	// load in palette colors and convert to dwords before writing out
	// in hexadecimal
	char pal[512];
	FILE *fp;
	fopen_s(&fp, "stella.pal", "rb");
	fread(pal, 1, 384, fp);
	fclose(fp);
	for (int p = 127; p >= 0; p--)
	{
		pal[p * 4] = pal[p * 3];
		pal[p * 4 + 1] = pal[p * 3 + 1];
		pal[p * 4 + 2] = pal[p * 3 + 2];
		pal[p * 4 + 3] = 0;
	}

	fopen_s(&fp, "tia_palette.h", "w");
	fprintf(fp, "\nint tia_colors[]={\n");
	for (int p = 0; p < 128; p++)
	{
		if (p)
			fprintf(fp, ",\n");
		int color = ((int*)pal)[p];
		fprintf(fp, "\t0x%08x", color);
	}
	fprintf(fp, "\n};");
	fclose(fp);
}