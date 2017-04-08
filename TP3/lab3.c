#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "sys/time.h"
#include "mpi.h"

// Constants
const int TEMPS_ATTENTE = 5;
const int SEND_TAG = 0;
const int STOP_TAG = 1;

int main(int argc, const char* argv[]) {
	if (argc != 9){
		printf("Il manque des arguments! \n");
		return 0;
	}

	int err, nbp, mon_id, master;
    int m, n, np, nbproc, seq, affichage;
    int i,j,k;
    float td, h;
    double timeStart, timeEnd, TexecSeq, TexecPar;
    struct timeval tp;

	// MPI_Init
	err = MPI_Init(&argc, (void*) &argv);
	if(err != MPI_SUCCESS){
		printf("Probleme lors de l'initialisation de MPI. \n");
		return -1;
	}

	err = MPI_Comm_rank(MPI_COMM_WORLD, &mon_id);
	err = MPI_Comm_size(MPI_COMM_WORLD, &nbp);

	//Validation des parametres
	n           = (int)atoi(argv[1]); // nombre de lignes
	m           = (int)atoi(argv[2]); // nombre de colonnes
	np          = (int)atoi(argv[3]); // nombre de pas de temps
	td          = (float)atof(argv[4]); // temps discretise
	h           = (float)atof(argv[5]); // taille dun cotes dune subdivision
	nbproc      = (int)atoi(argv[6]); // nombre de processus a utiliser
	seq         = (int)atoi(argv[7]); // si on le resultat seq
	affichage   = (int)atoi(argv[8]); //si on veut laffichage

	float matrix[m][n][2];
	master = 0;

    if(mon_id == master){
        //Initialiser la matrice
        for( i = 0; i < m; i++){
            for( j = 0; j < n; j++) {
                usleep(TEMPS_ATTENTE);
                matrix[i][j][0] = i*(m - i -1) * j*(n - j - 1);
                matrix[i][j][1] = i*(m - i -1) * j*(n - j - 1);
            }
        }

        //Affichage de la matrice initiale
        if(affichage==1){
            printf("Init. Matrice Seq \n");

            for(i = 0; i < m; i++){
                for(j = 0; j < n;j++){
                    printf("%5.2f\t",matrix[i][j][0]);
                }
                printf("\n");
            }
        }

        //Debut du timer
        gettimeofday (&tp, NULL);
        timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;

        //Traitement sequentiel
        if(seq == 1){
            for( k = 1; k <= np; k++){
                for( i = 1; i < m-1; i++){
                    for( j = 1; j < n-1; j++) {
                        usleep(TEMPS_ATTENTE);
                        matrix[i][j][k%2] = ((1-4*td/(h*h)) * (matrix[i][j][(k+1)%2])) + ((td/(h*h)) * (matrix[i-1][j][(k+1)%2] + matrix[i+1][j][(k+1)%2] + matrix[i][j+1][(k+1)%2] + matrix[i][j-1][(k+1)%2]));
                    }
                }
            }
        }

        //Arret du timer
        gettimeofday (&tp, NULL);
        timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
        TexecSeq = timeEnd - timeStart;

        //Affichage de la matrice finale
        if(affichage==1 && seq==1){
            printf("Matrice Seq \n");

            k = 0;
            if(np%2 ==1) {
                k = 1;
            }
            for(i = 0; i < m; i++){
                for(j = 0; j < n;j++){
                    printf("%5.2f\t",matrix[i][j][k]);
                }
                printf("\n");
            }
        }

        //Initialiser la matrice
        for( i = 0; i < m; i++){
            for( j = 0; j < n; j++) {
                usleep(TEMPS_ATTENTE);
                matrix[i][j][0] = i*(m - i -1) * j*(n - j - 1);
                matrix[i][j][1] = i*(m - i -1) * j*(n - j - 1);
            }
        }

        //Afficher la matrice initiale
        if(affichage==1){
            printf("Init. Matrice Par \n");

            for(i = 0; i < m; i++){
                for(j = 0; j < n;j++){
                    printf("%5.2f\t",matrix[i][j][0]);
                }
                printf("\n");
            }
        }
    }

	//MPI_Barrier()
	err = MPI_Barrier(MPI_COMM_WORLD);

	//Demarrer le timer
	gettimeofday (&tp, NULL);
	timeStart = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;

	//Effectuer le traitement parallele
	// process maitre qui gerent la reponse
	if(mon_id == master){
		int proc_id = 1;
		MPI_Request requestList[(m-2)*(n-2)]; // -2 because of frontier
		MPI_Status statList[(m-2)*(n-2)]; // -2 because of frontier

		for(int k = 1; k <= np; k++){
            int sendCount = 0;
            for(int i = 1; i < m-1; i++){
                for(int j = 1; j < n-1; j++){
                    float data[5];
                    //set data
                    data[0] = matrix[i][j][(k+1)%2];   // cell to calculate
                    data[1] = matrix[i][j-1][(k+1)%2]; // left neighbor
                    data[2] = matrix[i-1][j][(k+1)%2]; // top neighbor
                    data[3] = matrix[i][j+1][(k+1)%2]; // right neigbor
                    data[4] = matrix[i+1][j][(k+1)%2]; // bottom neighbor

                    //send data to slave
                    MPI_Send(data,5,MPI_FLOAT,proc_id,SEND_TAG,MPI_COMM_WORLD);

                    //receive result from slave
                    MPI_Irecv(&matrix[i][j][k%2],1,MPI_FLOAT,proc_id,MPI_ANY_TAG,MPI_COMM_WORLD,&requestList[sendCount]);
                    sendCount++;
                    proc_id = (proc_id%(nbproc-1))+1;//get next process
                }
            }

            //Active wait for all data to be calculate to go to next delta time
            MPI_Waitall(sendCount,requestList,statList);
        }

        //Envoie larret des calculs
        for(proc_id = 1; proc_id< nbproc; proc_id++){
            MPI_Send(0,0,MPI_INT, proc_id, STOP_TAG, MPI_COMM_WORLD);
        }

        //Arreter le timer
        gettimeofday(&tp, NULL);
        timeEnd = (double) (tp.tv_sec) + (double) (tp.tv_usec) / 1e6;
        TexecPar = timeEnd - timeStart;

        //Afficher la matrice finale
        if(affichage==1){
            int k = 0;
            if(np%2 ==1) {
                k = 1;
            }
            for(i = 0; i < m; i++){
                for(j = 0; j < n;j++){
                    printf("%5.2f\t",matrix[i][j][k]);
                }
                printf("\n");
            }
        }

        //Afficher les temps seq,par et acceleration
        printf("Temps sequentiel = %f\n", TexecSeq);
        printf("Temps parallele = %f\n", TexecPar);
        double acc = TexecSeq/TexecPar;
        printf("Acceleration = %f\n", acc);
	}
	else if (mon_id>master && mon_id<nbproc){
        // Process des slaves
	    while(1){
            MPI_Status statut;
            float data[5];
            float result;

            // Recevoir les infos de demarage
            MPI_Recv(data,5,MPI_FLOAT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&statut);
            // Si recoit le signal darret
            if(statut.MPI_TAG == STOP_TAG){
                break;
            }

            usleep(TEMPS_ATTENTE);
            result = ((1-4*td/(h*h)) * (data[0])) + ((td/(h*h)) * (data[1] + data[2] + data[3] + data[4]));
            //Renvoie le resultat au master
            MPI_Send(&result, 1, MPI_FLOAT, 0, SEND_TAG, MPI_COMM_WORLD);
        }
	}

	//Fermeture du Comm_World
	MPI_Finalize();

	return 0;
}