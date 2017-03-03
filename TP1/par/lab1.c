#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "sys/time.h"
#include "mpi.h"

int main(int argc, const char* argv[]) {
	if (argc != 4){
		printf("Il manque des arguments! \n");
		return 0;
	}
	
	int err;
	int np;
	int mon_id;
	int master = 8;
	
	//acceder aux parametres 
	int prob = (int)atoi(argv[1]);
	int valeur = (int)atoi(argv[2]);
	int alteration = atoi(argv[3]);
		
	// Creation d'un Comm_World
	err = MPI_Init(&argc, &argv);
	if(err != MPI_SUCCESS){
		printf("Probleme lors de l'initialisation de MPI. \n");
		return -1;
	}
	MPI_Status statut;
	err = MPI_Comm_rank(MPI_COMM_WORLD, &mon_id);
	err = MPI_Comm_size(MPI_COMM_WORLD, &np);
	
	//Code tire de l'exemple minuteur.c fourni sur le site du cours
	double timeStart, timeEnd, Texec;
	struct timeval tp;
	gettimeofday (&tp, NULL); // Debut du chronometre
	timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
	
	// process maitre qui gerent la reponse
	if(mon_id == master){
		printf("================================================\n");
		printf("PAR: prob %d, init %d, iter %d\n\n", prob, valeur, alteration);
		
		int j;
		int resultat[8][8];
		//Reception des messages dans lordre pour avoir le bon resultat
		for(j=0;j<8;j++){
			MPI_Recv(resultat[j],8,MPI_INT,j,0,MPI_COMM_WORLD,&statut);
		}
		
		//Affichage de la matrice finale
		printf("Matrice finale:\n");
		int i;
		for( i = 0; i < 8; i++){
			printf("%d %d %d %d %d %d %d %d\n",resultat[i][0],resultat[i][1],resultat[i][2],resultat[i][3],resultat[i][4],resultat[i][5],resultat[i][6],resultat[i][7]);
		}
		
		//Code tire de l'exemple minuteur.c fourni sur le site du cours
		gettimeofday (&tp, NULL); // Fin du chronometre
		timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
		Texec = timeEnd - timeStart; //Temps d'execution en secondes	
		
		printf("\n");
		printf("Temps d'execution : %f\n",Texec);
		printf("================================================\n");
	}
	// Les 8 autres process gerent les calculs
	else if(mon_id != master){
		
		//Initialisation des lignes de la matrice (chaque ligne est identique au debut)
		int i,k;
		int matrix[8];
		for(i = 0; i < 8;i++) {
			matrix[i] = valeur;
		}
		
		
		//Dans le cas du probleme 1
		if(prob == 1){
			//Meme code que sequentielle sauf changement de j par mon_id
			for(k=0;k<=alteration;k++){
				for(i=0;i<8;i++){
					usleep(1000);
					matrix[i]= matrix[i] + (i+mon_id) * k;
				}
			}
		//Dans le cas du probleme 2
		}else if(prob == 2){
			//Meme code que sequentielle sauf changement de j par mon_id
			for(k=0;k<=alteration;k++){
				for(i=0;i<8;i++){
					if(i==0){
						usleep(1000);
						matrix[i] = matrix[i] + (mon_id*k);
					}else{
						usleep(1000);
						matrix[i] = matrix[i] + matrix[i-1] * k;
					}
				}
			}
		}
		//Dans le cas dun probleme inexistant
		else{
			printf("Numero de probleme inexistant!!! \n");
			return -1;
		}
		
		//envoie les donnees au process maitre
		MPI_Send(&matrix, 8, MPI_INT, 8, 0, MPI_COMM_WORLD);
	}
	
	
	//Fermeture du Comm_World
	MPI_Finalize();
	
	return 0;
}