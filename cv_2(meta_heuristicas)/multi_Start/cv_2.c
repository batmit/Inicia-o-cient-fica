#include<stdio.h>
#include <stdlib.h>
#include "cv_2.h"
#include<math.h>
#include<float.h>


//Utilizarei o multi start com o vizinho mais próximo como inicial e logo após 
// o 2-opt para refinar

struct cidade{

    double latitude, longitude;
    int visitado;

};



int main(){

    Cidade *vetorCidades = malloc(CIDADES* sizeof(Cidade));
    int *caminho = malloc(CIDADES * sizeof(int));
    int *resultadoFinal = malloc(CIDADES * sizeof(int));
    int **vizinhosOrdenados = malloc(CIDADES * sizeof(int*));
    for(int i = 0; i < CIDADES; i++){
        vizinhosOrdenados[i] = malloc(CIDADES * sizeof(int));
        vizinhosOrdenados[i][0] = i;
    }
    //Apenas lerei os valores e colocarei o identificador em cada Cidade
    for(int i = 0; i < CIDADES; i++){
        resultadoFinal[i] = 0;
        caminho[i] = -1;
        vetorCidades[i].visitado = 0;
        scanf("%lf", &vetorCidades[i].latitude);
        scanf("%lf", &vetorCidades[i].longitude);
    }

    //Calculos da matriz distancia
    double **matrizDistancia = malloc(CIDADES * sizeof(double*));
    for(int i = 0; i < CIDADES; i++){

        matrizDistancia[i] = malloc(CIDADES * sizeof(double));
        for(int j = 0; j < CIDADES; j++){

            matrizDistancia[i][j] = distanciaEuclidiana(vetorCidades[i].latitude, vetorCidades[j].latitude, vetorCidades[i].longitude, vetorCidades[j].longitude);


        }
    }

    ordenarVizinhos(vizinhosOrdenados, vetorCidades, matrizDistancia);
    //terei que fixar a primeira posição da cidade, por questões de complexidad
    caminho[0] = 0;
    vetorCidades[0].visitado = 1;
    int *resultadoParcial = malloc(sizeof(int) * CIDADES);
    double melhorCusto = multiStart(vetorCidades, matrizDistancia, resultadoParcial);
    resultadoFinal[0] = resultadoParcial[0];
    for(int i = 1; i < CIDADES; i++){
        resultadoFinal[i] = resultadoParcial[i];
        vetorCidades[i].visitado = 0;
    }
    double **matrizReducao = copiarMatriz(matrizDistancia);
    double limiteRaiz = reduzirMatriz(matrizReducao);

    branchAndBound(vetorCidades, 1, caminho, 0, matrizDistancia, &melhorCusto, resultadoFinal, resultadoParcial, matrizReducao, limiteRaiz, vizinhosOrdenados);
    
    printf("Menor distancia encontrada: %.6lf\n", melhorCusto);
    printf("Caminho: ");
    for (int i = 0; i < CIDADES; i++) {
        printf("%d ", resultadoFinal[i]);
    }
    printf("\n");




    free(resultadoParcial);
    free(resultadoFinal);
    free(caminho);
    free(vetorCidades);
    for(int i = 0; i < CIDADES; i++){

        free(matrizDistancia[i]);

    }
    free(matrizDistancia);
    liberarMatriz(matrizReducao);
    return 0;

}


double distanciaEuclidiana(double x1, double y1, double x2, double y2){

    double  resposta;


    resposta = ((x1 - y1)*(x1 - y1)) + ((x2 - y2) * (x2 - y2));
    resposta = sqrt(resposta);



    return resposta;
}

//melhor custo deve ser iniciado com um valor muito alto
void branchAndBound(Cidade *vetorCidades, int nivel, int caminho[], double custoAtual, double **matrizDistancia, double *melhorCusto, int resultadoFinal[], int resultadoParcial[], double **matrizAtual, double limiteAtual, int **vizinhosOrdenados){

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
        
        int candidato = vizinhosOrdenados[caminho[nivel - 1]][i];

        //Se a cidade já não está no caminho
        if(vetorCidades[candidato].visitado == 0){

            double **novaMatriz = copiarMatriz(matrizAtual);
            for(int k = 0; k < CIDADES; k++){
                novaMatriz[caminho[nivel-1]][k] = DBL_MAX; // linha
                novaMatriz[k][candidato] = DBL_MAX;               // coluna
            }
            // Evita subciclo prematuro
            novaMatriz[candidato][caminho[0]] = DBL_MAX;
            double custaAresta = matrizAtual[caminho[nivel -1]][candidato];
            double reducao = reduzirMatriz(novaMatriz);

            double novoLimite = limiteAtual + custaAresta + reducao;
            
            
            double novoCusto = custoAtual + matrizDistancia[caminho[nivel -1]][candidato];

            //caminho[nivel] = resultadoParcial[i];
            //vetorCidades[resultadoParcial[i]].visitado = 1;
            //double limite = calcularLimiteInferior(vetorCidades, caminho, nivel + 1, novoCusto, matrizDistancia);
            if(novoLimite <  *melhorCusto){
                caminho[nivel] = candidato;
                vetorCidades[resultadoParcial[i]].visitado = 1;
                branchAndBound(vetorCidades, nivel + 1, caminho, novoCusto, matrizDistancia, melhorCusto, resultadoFinal, resultadoParcial, novaMatriz, novoLimite, vizinhosOrdenados);
            
                vetorCidades[resultadoParcial[i]].visitado = 0;
                caminho[nivel] = -1;
            }

            liberarMatriz(novaMatriz);
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
    limite+=menorRetorno;


    return limite;
}


double vizinhoMaisProximo(Cidade *vetorCidades, double **matrizDistancia, int *resultadoFinal, int posInicial){

    double soma = 0;
    int cont = posInicial - 1;

    int atual = resultadoFinal[posInicial - 1];
    //resultadoFinal[0] = 0;
    for(int pos = posInicial; pos < CIDADES; pos++){
        double distancia = DBL_MAX;
        int cidade = -1;

        for(int j = 1; j < CIDADES; j++){

            if(matrizDistancia[atual][j] < distancia && vetorCidades[j].visitado == 0){

                distancia = matrizDistancia[atual][j];
                cidade = j;
            }


        }

        if(cidade == -1){
            break;
        }
        resultadoFinal[pos] = cidade;
        soma+= distancia;
        vetorCidades[cidade].visitado = 1;
        cont++;
        atual = cidade;
        

    }

    soma+= matrizDistancia[atual][0];
    return soma;

}

double multiStart(Cidade *vetorCidades, double **matrizDistancia, int *resultadoFinal){
    int *rotaAtual = malloc(sizeof(int) * CIDADES);
    int *vetAux = malloc(sizeof(int) * CIDADES);
    vetAux[0] = 0;
    rotaAtual[0] = 0;
    vetorCidades[0].visitado = 1;
    double melhorCusto = DBL_MAX;

    for(int i = 1; i < CIDADES ; i++){
            
        for (int j = 1; j < CIDADES; j++) {
            vetAux[j] = -1;
            rotaAtual[j] = -1;
            vetorCidades[j].visitado = 0;
        }
        vetAux[1] = i;
        rotaAtual[1] = i;
        vetorCidades[i].visitado = 1;

        vizinhoMaisProximo(vetorCidades, matrizDistancia, rotaAtual, 2);
        twoopt(vetorCidades, matrizDistancia, rotaAtual);
        double vmP = calcularCusto(matrizDistancia, rotaAtual);

        for(int j = 1; j < CIDADES; j++){
            if(j != i){
                vetorCidades[j].visitado = 0;

            }
        }

        heuristicaInsercaoBarata(vetorCidades, matrizDistancia, vetAux, 2);
        twoopt(vetorCidades, matrizDistancia, vetAux);
        double Insercao = calcularCusto(matrizDistancia, vetAux);
        double custo;
        int vmOuIns;

        if(vmP < Insercao){
            printf("Vmp\n");
            custo = vmP;
            vmOuIns = 1;
        }else{
            printf("Insercao\n");
            custo = Insercao;
            vmOuIns = 0;
        }

        if(custo < melhorCusto){
            melhorCusto = custo;
            printf("Custo: %lf\n", melhorCusto);
            if(vmOuIns == 1){
                for(int j = 0; j < CIDADES; j++){
                    resultadoFinal[j] = rotaAtual[j];

                }

            }else{
                for(int j = 0; j < CIDADES; j++){
                    resultadoFinal[j] = vetAux[j];

                }

            }


        }


    }

    free(rotaAtual);
    free(vetAux);
    return melhorCusto;

}


void twoopt(Cidade *vetorCidades, double **matrizDistancia, int *rota){

    int melhorou = 1;
    
    while (melhorou) {
        melhorou = 0;

        for (int i = 1; i < CIDADES - 1; i++) {
            for (int j = i + 1; j < CIDADES; j++) {
                int a = rota[i - 1];
                int b = rota[i];
                int c = rota[j];
                int d = rota[(j + 1) % CIDADES];

                double custoAtual = matrizDistancia[a][b] + matrizDistancia[c][d];
                double novoCusto  = matrizDistancia[a][c] + matrizDistancia[b][d];

                if (novoCusto < custoAtual) {
                    inverterTrecho(rota, i, j);
                    melhorou = 1;
                }
            }
        }
    }




}

void inverterTrecho(int *rota, int inicio, int fim) {
    while (inicio < fim) {
        int temp = rota[inicio];
        rota[inicio] = rota[fim];
        rota[fim] = temp;
        inicio++;
        fim--;
    }
}

double calcularCusto(double **matrizDistancia, int *resultado){

    double custo = 0;

    for(int i = 0; i < CIDADES - 1; i++){

        custo+= matrizDistancia[resultado[i]][resultado[i + 1]];

    }

    custo += matrizDistancia[resultado[CIDADES - 1]][resultado[0]];
    return custo;


}


void heuristicaInsercaoBarata(Cidade *vetorCidades, double **matrizDistancia, int *resultadoFinal, int nivel){




    //Como nesse código eu preciso de um ciclo mínimo, no caso de 2 cidades, 
    //eu vou pegar a primeira e também a mais próxima da primeira
    //resultadoFinal[0] = 0;
    //vetorCidades[0].visitado = 1;
    //double menosDist = matrizDistancia[0][1];
    //int cidade = 1;
    //apenas  acho a cidade mais proxima da primeira
    //for(int i = 2; i < CIDADES; i++){

    //    if(matrizDistancia[0][i] < menosDist){

    //        menosDist = matrizDistancia[0][i];
    //        cidade = i;

    //    }


    //}
    //vetorCidades[cidade].visitado = 1;
    //resultadoFinal[1] = cidade;
    int tamCiclo = nivel;
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

}


void ordenarVizinhos(int **vizinhosOrdenados, Cidade *vetorCidades, double **matrizDistancia){

    for(int i = 0; i < CIDADES; i++){

        for(int j = 1; j < CIDADES; j++){
            int nearer;
            for(int w = 0; w < CIDADES; w++){
                if(vetorCidades[w].visitado == 0 && w != i){

                    nearer = w;
                    break;

                }
            }
            
            for(int z = 0; z < CIDADES; z++){

                if(matrizDistancia[i][nearer] > matrizDistancia[i][z] && vetorCidades[z].visitado == 0 && z != i){

                    nearer = z;

                }


            }

            vizinhosOrdenados[i][j] = nearer;
            vetorCidades[nearer].visitado = 1;


        }

        for(int j = 0; j < CIDADES; j++){

            vetorCidades[j].visitado = 0;

        }

    }


}

double reduzirMatriz(double **m){
    double total = 0;

    // Redução de linhas
    for(int i = 0; i < CIDADES; i++){
        double minVal = DBL_MAX;
        for(int j = 0; j < CIDADES; j++)
            if(m[i][j] < minVal) minVal = m[i][j];
        if(minVal != DBL_MAX && minVal > 0){
            for(int j = 0; j < CIDADES; j++)
                if(m[i][j] != DBL_MAX) m[i][j] -= minVal;
            total += minVal;
        }
    }

    // Redução de colunas
    for(int j = 0; j < CIDADES; j++){
        double minVal = DBL_MAX;
        for(int i = 0; i < CIDADES; i++)
            if(m[i][j] < minVal) minVal = m[i][j];
        if(minVal != DBL_MAX && minVal > 0){
            for(int i = 0; i < CIDADES; i++)
                if(m[i][j] != DBL_MAX) m[i][j] -= minVal;
            total += minVal;
        }
    }

    return total;
}

double **copiarMatriz(double **original) {
    double **copia = malloc(CIDADES * sizeof(double *));
    
    for (int i = 0; i < CIDADES; i++) {
        copia[i] = malloc(CIDADES * sizeof(double));
        for (int j = 0; j < CIDADES; j++) {
            copia[i][j] = original[i][j];
        }
    }
    
    return copia;
}

void liberarMatriz(double **matriz) {
    for (int i = 0; i < CIDADES; i++) {
        free(matriz[i]);
    }
    free(matriz);
}