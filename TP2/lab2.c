#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "sys/time.h"
#include <omp.h>

#define matrix_size 10

int main(int argc, char ** argv){
	if(argc == 4){
		//acceder aux parametres 
		int prob = (int)atoi(argv[1]);
		int valeur = (int)atoi(argv[2]);
		int alteration = (int)atoi(argv[3]);
		
		printf("================================================\n");
		printf("OPENMP: prob %d, init %d, iter %d\n\n",prob, valeur, alteration);
		
		int i,j,k;
		int matrix[matrix_size][matrix_size];
		double timeStart, timeEnd, Texec;
		struct timeval tp;
		double tempsSeq, tempsPar;
		
		//set les variables pour OpenMP
		int nb_proc = omp_get_num_procs();
		omp_set_num_threads(nb_proc);
		printf("Il y a %d threads disponibles. \n\n", nb_proc);
		printf("================================================\n");
		
		////////////////////////////////////////////////////////////////////////////
		//Partie Sequentielle
		//
		//Code tire de l'exemple minuteur.c fourni sur le site du cours
		gettimeofday (&tp, NULL); // Debut du chronometre
		timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		
		// Initialisation de la matrice de depart
		for( i = 0; i < matrix_size; i++){
			for( j = 0; j < matrix_size; j++){
				matrix[i][j] = valeur;
			}
		}
		
		if(prob == 1){			
			for( k = 1; k <= alteration; k++){
				for( i = 0; i < matrix_size; i++){
					for( j = 0; j < matrix_size; j++){
						matrix[i][j] = matrix[i][j] + i + j;
						usleep(50);
					}
				}
			}
		}
		else if(prob==2){
			for( k = 1; k <= alteration; k++){
				for( j = matrix_size-1; j >= 0; j--){
					for( i = 0; i < matrix_size; i++){
						if(j==9){
							matrix[i][j] = matrix[i][j] + i;
						}else{
							matrix[i][j] = matrix[i][j] + matrix[i][j+1];
						}
						usleep(50);
					}
				}
			}
		}
		
		//Code tire de l'exemple minuteur.c fourni sur le site du cours
		gettimeofday (&tp, NULL); // Fin du chronometre
		timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		Texec = timeEnd - timeStart; //Temps d'execution en secondes
		tempsSeq = Texec;
		
		printf("Matrice finale sequentiel:\n");
		for( i = 0; i < matrix_size; i++){
			for( j = 0; j < matrix_size; j++){
				printf("%d ", matrix[i][j]);
			}
			printf("\n");
		}
		printf("\n\n");
		printf("Temps d'execution sequentiel: %f\n",Texec);
		printf("================================================\n");
		
		
		/////////////////////////////////////////////////////////////////////////
		// Partie OpenMP
		//
		//Code tire de l'exemple minuteur.c fourni sur le site du cours
		gettimeofday (&tp, NULL); // Debut du chronometre
		timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		
		// Initialisation de la matrice de depart
		for( i = 0; i < matrix_size; i++){
			for( j = 0; j < matrix_size; j++){
				matrix[i][j] = valeur;
			}
		}
		
		if(prob == 1){			
			for( k = 1; k <= alteration; k++){
				for( i = 0; i < matrix_size; i++){
					#pragma omp parallel for
					for( j = 0; j < matrix_size; j++){
						matrix[i][j] = matrix[i][j] + i + j;
						usleep(50);
					}
				}
			}
		}
		
		else if(prob==2){
			for( k = 1; k <= alteration; k++){
				for( j = matrix_size-1; j >= 0; j--){
					#pragma omp parallel for
					for( i = matrix_size-1; i >= 0; i--){
						if(j==9){
							matrix[i][j] = matrix[i][j] + i;
						}else{
							matrix[i][j] = matrix[i][j] + matrix[i][j+1];
						}
						usleep(50);
					}
				}
			}
		}
		
		//Code tire de l'exemple minuteur.c fourni sur le site du cours
		gettimeofday (&tp, NULL); // Fin du chronometre
		timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		Texec = timeEnd - timeStart; //Temps d'execution en secondes
		tempsPar = Texec;
		
		printf("Matrice finale OpenMP:\n");
		for( i = 0; i < matrix_size; i++){
			for( j = 0; j < matrix_size; j++){
				printf("%d ", matrix[i][j]);
			}
			printf("\n");
		}
		printf("\n\n");
		printf("Temps d'execution OpenMP: %f\n",Texec);
		printf("================================================\n");
		
		double acceleration;
		acceleration = tempsSeq/tempsPar;
		printf("L'acceleration est de %f!\n",acceleration);
		printf("================================================\n");
		
		return 0;
	}
	else{
		return -1;
	}
}