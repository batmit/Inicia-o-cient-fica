#include<stdio.h>
#include <stdlib.h>
#include "cv_1.h"
#include<math.h>
#include<float.h>
#include <ncurses.h>

//Teremos a função gulosa que usa a heurística do vizinho mais próximo para inicializar, e usa branch and bound para podar todas que forem maior do que o valor dela

struct cidade{

    double latitude, longitude;
    int visitado;

};



int main(){

    Cidade *vetorCidades = malloc(CIDADES* sizeof(Cidade));
    int *caminho = malloc(CIDADES * sizeof(int));
    int *resultadoFinal = malloc(CIDADES * sizeof(int));

    //Apenas lerei os valores e colocarei o identificador em cada Cidade
    for(int i = 0; i < CIDADES; i++){
        resultadoFinal[i] = 0;
        caminho[i] = -1;
        vetorCidades[i].visitado = 0;
        scanf("%lf", &vetorCidades[i].latitude);
        scanf("%lf", &vetorCidades[i].longitude);
    }

    double **matrizDistancia = malloc(CIDADES * sizeof(double*));
    for(int i = 0; i < CIDADES; i++){

        matrizDistancia[i] = malloc(CIDADES * sizeof(double));
        for(int j = 0; j < CIDADES; j++){

            matrizDistancia[i][j] = distanciaEuclidiana(vetorCidades[i].latitude, vetorCidades[j].latitude, vetorCidades[i].longitude, vetorCidades[j].longitude);


        }
    }

    //terei que fixar a primeira posição da cidade, por questões de complexidad
    caminho[0] = 0;
    vetorCidades[0].visitado = 1;
    double melhorCusto = vizinhoMaisProximo(vetorCidades, matrizDistancia, resultadoFinal);

    for(int i = 1; i < CIDADES; i++){
        vetorCidades[i].visitado = 0;
    }

    branchAndBound(vetorCidades, 1, caminho, 0, matrizDistancia, &melhorCusto, resultadoFinal);
    printf("Menor distancia encontrada: %.6lf\n", melhorCusto);
    printf("Caminho: ");
    for (int i = 0; i < CIDADES; i++) {
        printf("%d ", resultadoFinal[i]);
    }
    printf("\n");





    free(resultadoFinal);
    free(caminho);
    free(vetorCidades);
    for(int i = 0; i < CIDADES; i++){

        free(matrizDistancia[i]);

    }
    free(matrizDistancia);
    return 0;

}


double distanciaEuclidiana(double x1, double y1, double x2, double y2){

    double  resposta;


    resposta = (pow((x1 - y1), 2)) + (pow((x2 - y2), 2));
    resposta = sqrt(resposta);



    return resposta;
}

//melhor custo deve ser iniciado com um valor muito alto
void branchAndBound(Cidade *vetorCidades, int nivel, int caminho[], double custoAtual, double **matrizDistancia, double *melhorCusto, int resultadoFinal[]){

    if(nivel == CIDADES){
        double custoTotal = custoAtual + matrizDistancia[caminho[nivel - 1]][caminho[0]];

        if(custoTotal < *melhorCusto){
            *melhorCusto = custoTotal;

            for(int i = 0; i < CIDADES; i++){
                resultadoFinal[i] = caminho[i];
            }
        }

        return;
    }

    for(int i = 1; i < CIDADES; i++){


        //Se a cidade já não está no caminho
        if(vetorCidades[i].visitado == 0){

            double novoCusto = custoAtual + matrizDistancia[caminho[nivel -1]][i];

            caminho[nivel] = i;
            vetorCidades[i].visitado = 1;
            double limite = calcularLimiteInferior(vetorCidades, caminho, nivel + 1, novoCusto, matrizDistancia);
            if(limite <  *melhorCusto){
                branchAndBound(vetorCidades, nivel + 1, caminho, novoCusto, matrizDistancia, melhorCusto, resultadoFinal);
            }

            vetorCidades[i].visitado = 0;
            caminho[nivel] = -1;

        }




    }


}

double calcularLimiteInferior(Cidade *vetorCidades, int caminho[], int nivel, double custoAtual, double **matrizDistancia) {
    double limite = custoAtual;

    int ultimaCidade = caminho[nivel - 1];

    // menor saída da última cidade para alguma não visitada
    double menorSaidaUltima = DBL_MAX;
    for(int i = 0; i < CIDADES; i++){
        if(vetorCidades[i].visitado == 0){
            if(matrizDistancia[ultimaCidade][i] < menorSaidaUltima){
                menorSaidaUltima = matrizDistancia[ultimaCidade][i];
            }
        }
    }

    if(menorSaidaUltima != DBL_MAX){
        limite += menorSaidaUltima;
    }

    // para cada cidade não visitada, soma sua menor saída
    for(int i = 0; i < CIDADES; i++){
        if(vetorCidades[i].visitado == 0){
            double menor = DBL_MAX;

            for(int j = 0; j < CIDADES; j++){
                if(i != j){
                    if(matrizDistancia[i][j] < menor){
                        menor = matrizDistancia[i][j];
                    }
                }
            }

            limite += menor;
        }
    }

    // retorno à origem (estimativa simples)
    double menorRetorno = DBL_MAX;
    for(int i = 0; i < CIDADES; i++){
        if(vetorCidades[i].visitado == 0){
            if(matrizDistancia[i][caminho[0]] < menorRetorno){
                menorRetorno = matrizDistancia[i][caminho[0]];
            }
        }
    }

    //Não estou considerando o retorno, porque pode levar a um valor acima do ideal, visto que eu estou somando mais acima do que o ideal

    if(menorRetorno != DBL_MAX){
       // limite += menorRetorno;
    }

    return limite;
}


double vizinhoMaisProximo(Cidade *vetorCidades, double **matrizDistancia, int *resultadoFinal){

    double soma = 0;
    int cont = 0;
    int i = 0;
    resultadoFinal[0] = 0;
    while(cont < CIDADES - 1){
        double distancia = DBL_MAX;
        int cidade = -1;

        for(int j = 1; j < CIDADES; j++){

            if(matrizDistancia[i][j] < distancia && vetorCidades[j].visitado == 0){

                distancia = matrizDistancia[i][j];
                cidade = j;
            }


        }

        if(cidade == -1){
            break;
        }

        soma+= distancia;
        vetorCidades[cidade].visitado = 1;
        cont++;
        i = cidade;
        resultadoFinal[cont] = cidade;


    }

    soma+= matrizDistancia[i][0];
    return soma;

}