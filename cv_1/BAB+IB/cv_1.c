#include<stdio.h>
#include <stdlib.h>
#include "cv_1.h"
#include<math.h>
#include<float.h>

//Teremos a função gulosa que usa a heurística de inserção mais barata para inicializar, e usa branch and bound para podar todas que forem maior do que o valor dela

struct cidade{

    double latitude, longitude;
    int visitado;

};



int main(){
    //vetor de cidades
    Cidade *vetorCidades = malloc(CIDADES* sizeof(Cidade));
    //Vetor de inteiros que representa o caminho que estou percorrendo
    int *caminho = malloc(CIDADES * sizeof(int));
    //vetor de inteiros que fica o resultado final
    int *resultadoFinal = malloc(CIDADES * sizeof(int));

    //Apenas lerei os valores e colocarei o identificador em cada Cidade
    for(int i = 0; i < CIDADES; i++){
        resultadoFinal[i] = 0;
        caminho[i] = -1;
        vetorCidades[i].visitado = 0;
        scanf("%lf", &vetorCidades[i].latitude);
        scanf("%lf", &vetorCidades[i].longitude);
    }

    //calculo as distancias e coloco numa matriz
    double **matrizDistancia = malloc(CIDADES * sizeof(double*));
    for(int i = 0; i < CIDADES; i++){

        matrizDistancia[i] = malloc(CIDADES * sizeof(double));
        for(int j = 0; j < CIDADES; j++){

            matrizDistancia[i][j] = distanciaEuclidiana(vetorCidades[i].latitude, vetorCidades[j].latitude, vetorCidades[i].longitude, vetorCidades[j].longitude);


        }
    }

    //terei que fixar a primeira posição da cidade, por questões de complexidade
    caminho[0] = 0;
    vetorCidades[0].visitado = 1;
    double melhorCusto = heuristicaInsercaoBarata(vetorCidades, matrizDistancia, resultadoFinal);

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

    //se estiver cheio
    if(nivel == CIDADES){
        //Custo atual + o da volta
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

    //cidade antes da atual
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


double heuristicaInsercaoBarata(Cidade *vetorCidades, double **matrizDistancia, int *resultadoFinal){

    for(int i = 0; i < CIDADES; i++){
        //coloco o vetor do resultado final como -1
        resultadoFinal[i] = -1;

    }


    //Como nesse código eu preciso de um ciclo mínimo, no caso de 2 cidades, 
    //eu vou pegar a primeira e também a mais próxima da primeira
    resultadoFinal[0] = 0;
    vetorCidades[0].visitado = 1;
    double menosDist = matrizDistancia[0][1];
    int cidade = 1;
    //apenas  acho a cidade mais proxima da primeira
    for(int i = 2; i < CIDADES; i++){

        if(matrizDistancia[0][i] < menosDist){

            menosDist = matrizDistancia[0][i];
            cidade = i;

        }


    }
    vetorCidades[cidade].visitado = 1;
    resultadoFinal[1] = cidade;
    int tamCiclo = 2;
    //coloco a cidade 0 e a mais proxima dela no ciclo e começo

    //Enquanto o ciclo for menor que o numero de cidades total
    while(tamCiclo < CIDADES){

        //variáveis para eu achar a melhor cidade na melhor posição
        int melhorCidade = -1;
        int melhorPosicao = -1;
        double melhorDelta = DBL_MAX;

        for(int k = 0; k < CIDADES; k++){

            //se não tiver sido visitado
            if(vetorCidades[k].visitado == 0){


                //vou passando por todas as posições do ciclo
                for(int pos = 0; pos < tamCiclo; pos++){

                    //pego a cidade na posicao pos
                    int i = resultadoFinal[pos];
                    int j;

                    if(pos == tamCiclo - 1){

                        j = resultadoFinal[0];

                    }else{

                        j = resultadoFinal[pos+1];

                    }

                    //Eu vou inserir entre o i e o j, nesse caso eu estou tentando calcular o acréscimo da distância, subtraindo pela distancia
                    //entre o i e o j, que era o que estava antes
                    double delta = matrizDistancia[i][k] + matrizDistancia[k][j] - matrizDistancia[i][j];

                    if(delta < melhorDelta){
                        //salvo a cidade e a posição
                        melhorDelta = delta;
                        melhorCidade = k;
                        melhorPosicao = pos;

                    }

                }


            }

        }
        //empurro todos os valores para colocar na posição certa
        for(int m = tamCiclo; m > melhorPosicao + 1; m--){
            resultadoFinal[m] = resultadoFinal[m - 1];
        }

        resultadoFinal[melhorPosicao + 1] = melhorCidade;
        vetorCidades[melhorCidade].visitado = 1;
        tamCiclo++;

    }

    double soma = 0;
    //Depois da inserção eu coloco eu calculo a distancia
    for(int i = 0; i < CIDADES -1; i++){

        soma += matrizDistancia[resultadoFinal[i]][resultadoFinal[i + 1]];


    }
    
    //tem que somar a volta
    soma += matrizDistancia[resultadoFinal[CIDADES - 1]][resultadoFinal[0]];
    return soma;
}

void empurrarVet(int* resultadoFinal, int nivel){

    for(int i =nivel + 1; i > 0; i++){

        resultadoFinal[i] = resultadoFinal[i-1];

    }

}