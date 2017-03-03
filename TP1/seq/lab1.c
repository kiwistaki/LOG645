#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "sys/time.h"

int main(int argc, const char* argv[]) {
	//printf("%d", argc);
	//fflush(stdout);
	if(argc == 4){
		//Code tire de l'exemple minuteur.c fourni sur le site du cours
		double timeStart, timeEnd, Texec;
		struct timeval tp;
		gettimeofday (&tp, NULL); // Debut du chronometre
		timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		
		//acceder aux parametres 
		int prob = (int)atoi(argv[1]);
		int valeur = (int)atoi(argv[2]);
		int alteration = (int)atoi(argv[3]);
		
		printf("================================================\n");
		printf("SEQ: prob %d, init %d, iter %d\n\n",prob, valeur, alteration);
		
		int i,j,k;

		// Initialisation de la matrice de depart
		int matrix[8][8];
		for( i = 0; i < 8; i++){
			for( j = 0; j < 8; j++){
				usleep(1000);
				matrix[i][j] = valeur;
			}
		}
		
		if(prob == 1){			
			for( k = 1; k <= alteration; k++){
				for( i = 0; i < 8; i++){
					for( j = 0; j < 8; j++){
						usleep(1000);
						matrix[i][j] = matrix[i][j] + (i+j) * k;
					}
				}
			}
		}
		else if(prob==2){
			for( k = 1; k <= alteration; k++){
				for( i = 0; i < 8; i++){
					for( j = 0; j < 8; j++){
						if(j==0){
							usleep(1000);
							matrix[i][j] = matrix[i][j] + (i*k);
						}else{
							usleep(1000);
							matrix[i][j] = matrix[i][j] + matrix[i][j-1] * k;
						}
					}
				}
			}
		}
		
		//Code tire de l'exemple minuteur.c fourni sur le site du cours
		gettimeofday (&tp, NULL); // Fin du chronometre
		timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		Texec = timeEnd - timeStart; //Temps d'execution en secondes
		
		printf("Matrice finale:\n");
		for( i = 0; i < 8; i++){
			printf("%d %d %d %d %d %d %d %d\n",matrix[i][0],matrix[i][1],matrix[i][2],matrix[i][3],matrix[i][4],matrix[i][5],matrix[i][6],matrix[i][7]);
		}
		printf("\n\n");
		printf("Temps d'execution : %f\n",Texec);
		printf("================================================\n");
		return 0;
	}
	else{
		return -1;
	}
}