#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define Mandelbrot 0
#define Julia 1
#define NUM_COLORS 256

typedef struct complex{
		long double a;
		long double b;
	} Complex;

typedef struct data{
		int setType;
		long double xMin;
		long double xMax;
		long double yMin;
		long double yMax;
		long double resolution;
		long double juliaA;
		long double juliaB;
		int maxSteps;
		int height;
		int width;
	} Data;

/* Suma a doua numere complexe */
Complex complexSum(Complex x, Complex y){
		Complex z;
		z.a = x.a + y.a;
		z.b = x.b + y.b;
	
		return z;
}

/* Produsul a doua numere complexe */
Complex complexsquare(Complex x, Complex y){
		Complex z;
		z.a = x.a * y.a - x.b * y.b;
		z.b = x.a * y.b + x.b * y.a;
		
		return z;
}

/* Modulul unui numar complex */
long double complexAbs(Complex z){
		long double sum = (z.a * z.a) + (z.b * z.b);
		if(sum > 0)	
			return sqrt(sum);
			
		return 0;	
}

/* Functie care genereaza multimea de tip Mandelbrot */
int lin, col;
void MandelbrotSet(unsigned char **image, Data data){
	Complex c, z, square;
	long double i, j, abs;
	int step;
	unsigned char color;
	
	lin = 0; col = 0;
	for(j = data.yMin; j < data.yMax - data.resolution; j += data.resolution){
		for(i = data.xMin; i < data.xMax - data.resolution; i += data.resolution){	
			c.a = i; c.b = j;
			z.a = 0; z.b = 0;
			step = 0;
			abs = complexAbs(z);
			while(abs < 2 && step < data.maxSteps){
				square = complexsquare(z, z);
				z = complexSum(square, c);
				step++;
				abs = complexAbs(z);
			}
			color = (unsigned char)(step % NUM_COLORS);
			image[lin][col++] = color;
			
		}
		col = 0;
		lin++;
	}
}

/* Functie care genereaza multimea de tip Julia */
void JuliaSet(unsigned char **image, Data data){
	Complex c, z, square;
	long double i, j, abs;
	int step;
	unsigned char color;
	
	lin = 0; col = 0;
	c.a = data.juliaA; c.b = data.juliaB;
	for(j = data.yMin; j < data.yMax - data.resolution; j += data.resolution){
		for(i = data.xMin; i < data.xMax - data.resolution; i += data.resolution){
			z.a = i; z.b = j;
			step = 0;
			abs = complexAbs(z);
			while(abs < 2 && step < data.maxSteps){
				square = complexsquare(z, z);
				z = complexSum(square, c);
				step++;
				abs = complexAbs(z);
			}
			color = (unsigned char)(step % NUM_COLORS);
			image[lin][col++] = color;
		}
		col = 0;
		lin++;
	}	
}

int main(int argc, char*argv[]){
	
	int h, k, width, height;
	int rank, np, tag = 11;
	MPI_Status status;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if(rank == 0){ // Procesul Master-------------------------------------------/

		/* Citirea datelor din fisierul de intrare */
		FILE *in = fopen(argv[1], "r");
		Data data;
		
		fscanf(in, "%d", &data.setType);
		fscanf(in, "%Lf%Lf%Lf%Lf", &data.xMin, &data.xMax, &data.yMin, &data.yMax);
		fscanf(in, "%Lf", &data.resolution);
		fscanf(in, "%d", &data.maxSteps);
		if(data.setType == Julia)
			fscanf(in, "%Lf%Lf", &data.juliaA, &data.juliaB);
		
		fclose(in);
		
		/* Crearea si alocarea de memorie pentru matricea ce va fi 
		 * scrisa in fisierul de iesire
		 */
		unsigned char **image;
		
		width =floor((data.xMax - data.xMin)/data.resolution);
		height =floor((data.yMax - data.yMin)/data.resolution);	
		
		image = (unsigned char**)malloc(height * sizeof(unsigned char*));
		for(h = 0; h < height; h++)
			image[h] = (unsigned char*)malloc(width * sizeof(unsigned char));
		
		data.height = height;
		data.width  = width;
		
		/* Trimite datele citite celorlalte procese */
		for(h = 1; h < np; h++)
			MPI_Send(&data, sizeof(struct data), MPI_BYTE, h, tag, MPI_COMM_WORLD);
			
		/* Determinarea intervalului din matrice pe care lucreaza acest proces */
		long double intervalSize;
		
		intervalSize = (data.yMax - data.yMin)/np;		
		data.yMax = data.yMin + intervalSize;
		
		for(h = 0; h < data.height % np; h++)
			data.yMax += data.resolution;
		
		/* Aplicarea algoritmului corespunzator asupra intervalului */
		if(data.setType == Mandelbrot)
			MandelbrotSet(image, data);
		else
			JuliaSet(image, data);

		/* Primeste de la celelalte procese intervalele calculate */
		unsigned char **imageRecv;
		unsigned char *line;
		int widthRecv, heightRecv, l;
		
		widthRecv =floor((data.xMax - data.xMin)/data.resolution);
		heightRecv = data.height/np;
		
		line = (unsigned char *)malloc(height * width * sizeof(unsigned char));
		imageRecv = (unsigned char**)malloc(height * sizeof(unsigned char *));
		for(h = 0; h < height; h++)
				imageRecv[h] = &(line[width * h]);
		
		lin = heightRecv;
		int src;
		
		for(h = 1; h < np; h++){
			MPI_Recv(&(imageRecv[0][0]), widthRecv * heightRecv, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
  		col = 0;
  		src = status.MPI_SOURCE;
  		lin = heightRecv * src; 
  		
			for(l = 0; l < heightRecv; l++){
				for(k = 0; k < widthRecv; k++){
					image[lin][col++] = imageRecv[l][k];
				}
				col = 0;
				lin++;
			}
			lin = heightRecv;
		}
		
		/* Scrierea matricii in fisierul de iesire */
		FILE *out = fopen(argv[2], "w");
		fprintf(out, "P2\n");
		fprintf(out, "%d %d\n", data.width, data.height);
		fprintf(out, "%d\n", 255);
		for(h = data.height - 1; h >= 0; h--){
			for(k = 0; k < data.width; k++){
				fprintf(out, "%d ", image[h][k]);
			}
			fprintf(out, "\n");
		}		
		
		fclose(out);
			
	}
	else {// Celelalte procese--------------------------------------------------/
		
		/* Primeste de la master datele citite din fisierul de intrare */
		Data data;
		
		MPI_Recv(&data, sizeof(struct data), MPI_BYTE, 0, tag, MPI_COMM_WORLD, &status);
		
		/* Determinarea intervalului din matrice pe care lucreaza acest proces */
		long double intervalSize;

		intervalSize = (data.height/np) * data.resolution;
		data.yMin = data.yMin + rank * intervalSize;		
		data.yMax = data.yMin + intervalSize;
		
		/* Crearea si alocarea matricii pe care acest proces o va trimite 
		 * procesului Master
		 */  
		unsigned char **image;
		unsigned char *line;
		
		width =floor((data.xMax - data.xMin)/data.resolution);
		height =data.height/np;
		
		line = (unsigned char *)malloc(height * width * sizeof(unsigned char));
		image = (unsigned char**)malloc(height * sizeof(unsigned char *));
		for(h = 0; h < height; h++)
				image[h] = &(line[width * h]);
			
		/* Aplicarea algoritmului corespunzator asupra intervalului */
		if(data.setType == Mandelbrot)
			MandelbrotSet(image, data);
		else
			JuliaSet(image, data);
		
		
		/* Trimite portiunea de matrice calculata procesului Master */
		MPI_Send(&(image[0][0]), width * height, MPI_UNSIGNED_CHAR, 0, tag, MPI_COMM_WORLD);
			
	}
	
	MPI_Finalize();
	return 0;
}

