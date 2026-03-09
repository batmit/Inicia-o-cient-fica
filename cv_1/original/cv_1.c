#include<stdio.h>
#include <stdlib.h>
#include "cv_1.h"
#include<math.h>


//Fixado o início como 0
//Não é circular, tem início de tem fim, não retorna à primeira cidade
//Custo extremamente elevado
//Sempre fica recalculando o valor da distância
//As coordenadas da cidade são inteiros

struct cidade{

    int latitude, longitude;

};



int main(){

    Cidade *vetorCidades = malloc(CIDADES* sizeof(Cidade));
    int *caminho = malloc(CIDADES * sizeof(int));
    int *resultadoFinal = malloc(CIDADES * sizeof(int));

    //Apenas lerei os valores e colocarei o identificador em cada Cidade
    for(int i = 0; i < CIDADES; i++){
        resultadoFinal[i] = 0;
        caminho[i] = -1;
        scanf("%d", &vetorCidades[i].latitude);
        scanf("%d", &vetorCidades[i].longitude);
    }


    //terei que fixar a primeira posição da cidade, por questões de complexidad
    caminho[0] = 0;
    double n = 0;
    int posicao = 1;
    cacheiroViajante(vetorCidades, caminho, &n, posicao, resultadoFinal);

    printf("Menor distancia encontrada: %lf\nCaminho: ", n);
    for(int i = 0; i < CIDADES; i++){

        printf("%d, ", resultadoFinal[i]);

    }




    free(resultadoFinal);
    free(caminho);
    free(vetorCidades);
    return 0;

}

void cacheiroViajante(Cidade *vetorCidades, int *caminho, double *n, int pos, int *resultadoFinal){

    if(pos == CIDADES){

        double distancia = distanciaEuclidiana(vetorCidades, caminho);
        if(distancia < *n || *n == 0){

            *n = distancia;

            //Copia para o resultado final o caminho
            for(int i = 0; i < CIDADES; i++){

                resultadoFinal[i] = caminho[i];

            }

        }
        return;
    }


    for(int i = 0; i < CIDADES; i++){
        int notParticipate = 0;
        for(int j = 0; j < pos; j++){

            if(caminho[j] == i){
                notParticipate++;
            }


        }

        //Se a cidade já não está no caminho
        if(notParticipate == 0){

            caminho[pos] = i;
            cacheiroViajante(vetorCidades, caminho, n, pos + 1, resultadoFinal);


        }




    }



}

//Não estou considerando a distãncia da volta
double distanciaEuclidiana(Cidade *vetorCidades, int *caminho){

    double resposta = 0, intermediario;

    for(int i = 0; i < CIDADES -1; i++){

        intermediario = (pow((vetorCidades[caminho[i]].latitude - vetorCidades[caminho[i+1]].latitude), 2)) + (pow((vetorCidades[caminho[i]].longitude - vetorCidades[caminho[i+1]].longitude), 2));
        intermediario = sqrt(intermediario);

        resposta+= intermediario;

    }


    return resposta;
}
