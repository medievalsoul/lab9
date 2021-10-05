#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

#define TAM 3   //tamanho do buffer
#define NPROD 2  //número de produtores
#define NCONS 5  //numero de consumidores

int buf[TAM]; //fiz o buffer mais simples possível
int indice = 0;  //índice do primeiro espaço livre no buffer
sem_t cond1, cond2, mutex;

//função que realiza a intereção do produtor com o Buffer
void produz(int id, int ind){
  buf[ind]=1; //novamente, fiz a implementação mais simples possível
  printf("Produtor %d produziu\n", id+1); //log de execução
  sem_post(&cond1); //produtor avisa que já acabou de produzir
}

//função executada pelo produtor
void* produtor(void* arg){
  int id = *(int*)arg;
  while(1){
    sem_wait(&mutex); //entrada de seção crítica
    if(indice<TAM-1){
      printf("Produtor %d produzirá em buf[%d]\n", id+1, indice); //log de execução
      int ind = indice; //variável necessária para que a função produz() seja chamada fora do trecho com exclução mútua
      indice++;
      sem_post(&mutex);//saída da seção crítica
      produz(id,ind); //chama a função que produz
    }
    else{         //produtor que for trabalhar coma última posição do buffer
      printf("Produtor %d produzirá em buf[%d]\n", id+1, indice);  //log de execução
      produz(id,indice);  //chama a função que produz
      for(int i=0; i<NPROD ; i++){ //espera todas as produtoras acabarem as suas tarefas
        sem_wait(&cond1);
      }
      sem_post(&cond2); //libera as threads consumidoras
    }
  }
  pthread_exit(NULL);
}

//função executada pelo consumidor
void* consumidor(void* arg){
  int id = *(int*)arg;
  while(1){
    sem_wait(&cond2);
    printf("Consumidor %d\n", id+1);
    for(int i=TAM-1; i>=0 ; i--){
      buf[i]=0;
    }
    indice=0;
    sleep(1);
    sem_post(&mutex);
  }
  pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char const *argv[]) {

  //id das threads
  pthread_t pid[NPROD];
  pthread_t cid[NCONS];

  //inicia o semáforo
  sem_init(&cond1, 0, NPROD);
  sem_init(&cond2, 0, 0);
  sem_init(&mutex, 0, 1);

  //criação das threads
  int i;
  int idp[NPROD]; //argumentos das funções executadas pelas threads
  int idc[NCONS];
  for(i=0 ; i<NPROD ; i++){
    idp[i]=i;
    if(pthread_create(&pid[i], NULL, produtor,(void*) &idp[i]))
      { printf("--ERRO: pthread_create()\n"); return 1; }
  }
  for(i=0 ; i<NCONS ; i++){
    idc[i]=i;
    if(pthread_create(&cid[i], NULL, consumidor,(void*) &idc[i]))
      { printf("--ERRO: pthread_create()\n"); return 1; }
  }


  //espera término das threads
  for (i=0; i<NPROD; i++) {
    if (pthread_join(pid[i], NULL))
      {printf("--ERRO: pthread_join() \n"); return 2;}
  }
  for (i=0; i<NCONS; i++) {
    if (pthread_join(cid[i], NULL))
      {printf("--ERRO: pthread_join() \n"); return 2;}
  }

  return 0;
}
