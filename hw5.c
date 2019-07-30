#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <math.h>

#define ROWS	480
#define COLS	640
#define pi      3.14

#define sqr(x)	((x)*(x))

void clear(unsigned char image[][COLS]);
void header(int row, int col, unsigned char head[32]);

int main(int argc, char** argv)
{
	int				i, j, max=0, angle, rho;
	static unsigned char  dedx, dedy, sgm[ROWS][COLS];
	int				sgm_threshold = 150, hough_threshold = 100, voting[180][800];
	FILE*			fp;
	static unsigned char  image[ROWS][COLS], head[32], bimage[ROWS][COLS], hough_bimage[ROWS][COLS];
	char			filename[50],ifilename[50], ch;

	strcpy(filename, "image.raw");
	memset(voting, 0, sizeof(int) * 180 * 800);
	header(ROWS, COLS, head);


	if (!(fp = fopen(filename, "rb")))
	{
		fprintf(stderr, "error: couldn't open %s\n", argv[1]);
		exit(1);
	}

	for (i = 0; i < ROWS; i++)
		if (!(COLS == fread(image[i], sizeof(char), COLS, fp)))
		{
			fprintf(stderr, "error: couldn't read %s\n", argv[1]);
			exit(1);
		}
	fclose(fp);

	for (i = 1; i < (ROWS - 1); i++)
		for (j = 1; j < (COLS - 1); j++)
		{
			dedx =
				abs(image[i - 1][j + 1] + 2 * image[i][j + 1] + image[i + 1][j + 1] - image[i - 1][j - 1] - 2 * image[i][j - 1] - image[i + 1][j - 1]);

			dedy =
				abs(image[i + 1][j - 1] + 2 * image[i + 1][j] + image[i + 1][j + 1] - image[i - 1][j - 1] - 2 * image[i - 1][j] - image[i - 1][j + 1]);

			sgm[i][j] = sqr(dedx) + sqr(dedy);

			if (max < sgm[i][j])
				max = sgm[i][j];
		}

	for (i = 1; i < (ROWS - 1); i++)
		for (j = 1; j < (COLS - 1); j++)
			sgm[i][j] = sgm[i][j] * 255 / max;

	/* Write SGM to a new image */
	strcpy(ifilename, "image_SGM.ras");
	if (!(fp = fopen(ifilename, "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", ifilename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++) fwrite(sgm[i], 1, COLS, fp);
	fclose(fp);


	/* Compute the binary image */
	for (i = 0; i < ROWS; i++)
		for (j = 0; j < COLS; j++)
		{
			if (i == 0 || j == 0 || i == ROWS - 1 || j == COLS - 1)
				sgm[i][j] = 0;
			if (sgm[i][j] < sgm_threshold)
				bimage[i][j] = 0;
			else bimage[i][j] = 255;
		}


	/* Write the binary image to a new image */
	strcpy(ifilename, "image_binary.ras");
	if (!(fp = fopen(ifilename, "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", ifilename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++) fwrite(bimage[i], 1, COLS, fp);
	fclose(fp);


	/* Save the original image as ras */
	strcpy(ifilename,"image.ras");
	if (!(fp = fopen(ifilename, "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", ifilename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++) fwrite(image[i], 1, COLS, fp);
	fclose(fp);

	printf("SGM threshold          %d\n", sgm_threshold);
	printf("Hough threshold        %d\n\n", hough_threshold);
	printf("  angle    rho    value of votes\n");


	/*Take vote*/
	for (i = 0; i < ROWS; i++) 
		for (j =0; j < COLS; j++)
			if (bimage[i][j] > 0)
				for (angle = 0; angle < 180; angle++)
				{
					rho = - (ROWS - 1 - i) * cos(angle*pi/180) + j * sin(angle*pi/180);
					if (rho < 800 ) 
						voting[angle][rho] ++;

				}
				
	for (i = 0; i < 180; i++)
		for (j = 0; j < 800; j++)
			if (voting[i][j] > hough_threshold)
				printf("   %d        %d             %d\n", i, j, voting[i][j]);
	  
	
	/*Print out binary image for Hough*/
	for (i = 0; i < ROWS; i++)
		for (j = 0; j < COLS; j++)
		{
			if ( abs( j * sin(52 * pi / 180) - (ROWS - 1 - i) * cos(52 * pi / 180)) < 0.1 ||
				 abs( j * sin(129 * pi / 180) - (ROWS - 1 - i) * cos(129 * pi / 180) - 472) < 0.1 ||
				 abs( j * sin(167 * pi / 180) - (ROWS - 1 - i) * cos(167 * pi / 180) - 146) < 0.1
				)
				hough_bimage[i][j] = 255;
			else
				hough_bimage[i][j] = 0;
		}
			
	strcpy(ifilename, "Hough_binary.ras");
	if (!(fp = fopen(ifilename, "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", ifilename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++) fwrite(hough_bimage[i], 1, COLS, fp);
	fclose(fp);



	printf("Press any key to exit: ");
	gets(&ch);

	return 0;
}

void clear(unsigned char image[][COLS])
{
	int	i, j;
	for (i = 0; i < ROWS; i++)
		for (j = 0; j < COLS; j++) image[i][j] = 0;
}

void header(int row, int col, unsigned char head[32])
{
	int *p = (int *)head;
	char *ch;
	int num = row * col;

	/* Choose little-endian or big-endian header depending on the machine. Don't modify this */
	/* Little-endian for PC */

	*p = 0x956aa659;
	*(p + 3) = 0x08000000;
	*(p + 5) = 0x01000000;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8000000;

	ch = (char*)&col;
	head[7] = *ch;
	ch++;
	head[6] = *ch;
	ch++;
	head[5] = *ch;
	ch++;
	head[4] = *ch;

	ch = (char*)&row;
	head[11] = *ch;
	ch++;
	head[10] = *ch;
	ch++;
	head[9] = *ch;
	ch++;
	head[8] = *ch;

	ch = (char*)&num;
	head[19] = *ch;
	ch++;
	head[18] = *ch;
	ch++;
	head[17] = *ch;
	ch++;
	head[16] = *ch;


	/* Big-endian for unix */
	/*
	*p = 0x59a66a95;
	*(p + 1) = col;
	*(p + 2) = row;
	*(p + 3) = 0x8;
	*(p + 4) = num;
	*(p + 5) = 0x1;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8;
*/
}

