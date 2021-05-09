#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

typedef struct barbearia {
  int n_barbeiros;
  int x_cadeiras;
  int z_clientes_min;
  int fechar;
  sem_t* cadeiras_de_espera;
  int* clientes_atendidos;
} barbearia;

typedef struct barbeiro {
  int  id;
  int* visor;
  pthread_t thread_barb;
  barbearia* barbearia;
  pthread_mutex_t* mutex;
  sem_t* servico_barbeiro;
  sem_t* cadeiras_barbeiro;
  sem_t* barbeiros_cortando_cabelo;
  sem_t* troca_visor;
  sem_t* le_visor;
} barbeiro;

typedef struct cliente {
  int  id;
  int* visor;
  pthread_t thread_cli;
  barbearia* barbearia;
  sem_t* servico_barbeiro;
  sem_t* cadeiras_barbeiro;
  sem_t* barbeiros_cortando_cabelo;
  sem_t* troca_visor;
  sem_t* le_visor;
} cliente;


int verifica_se_pode_fechar(barbearia* barb){
  int count_barbeiros_cumpriram_meta = 0;
  int i = 0;

  for (i=0; i<barb->n_barbeiros; i++){
    if (barb->clientes_atendidos[i] >= barb->z_clientes_min){
      count_barbeiros_cumpriram_meta += 1;
    }
  }

  if (count_barbeiros_cumpriram_meta == barb->n_barbeiros){
      return 1; // pode fechar
  }

  return 0; // continua rodando
}

void* barbeiro_func (void* arg) {
  barbeiro* barb = (barbeiro*) arg;
  int id = barb->id;

  while (barb->barbearia->fechar != 1) {
    // o nome diz tudo eu acho
    if (verifica_se_pode_fechar(barb->barbearia) == 1){

      pthread_mutex_lock(barb->mutex);

      // printf("Isso e tudo p-p-p-pessoal!\n");
      barb->barbearia->fechar = 1;

      pthread_mutex_unlock(barb->mutex);

      return NULL;
    }

    // define o barbeiro atual pro cliente
    sem_wait(barb->troca_visor);

    *barb->visor = id;
    sem_post(barb->le_visor);

    // espera (mimindo) algum cliente 
    // "deixa o serviço dele disponivel"
    sem_wait(&barb->servico_barbeiro[id]);

    // se algum cliente acordar ele, corta o cabelo do cliente e incrementa o counter
    sem_post(&barb->barbeiros_cortando_cabelo[id]);
    // printf("Barbeiro %d cortou o cabelo de um cliente.\n", id);
    barb->barbearia->clientes_atendidos[id]++;
  }

  return NULL;
}

void* cliente_func (void* arg) {
  cliente* cli = (cliente*) arg;
  int barbeiro_disponivel;
  // int id = cli->id; // so pros prints msm

  // antes de mais nada, a barbearia tem q estar aberta
  if (cli->barbearia->fechar != 1) {

    // ai o cliente va se ela ta cheia
    if (sem_trywait(cli->barbearia->cadeiras_de_espera) == 0) {
      // printf("Cliente %d entrou na barbearia\n", id);
      
      // Ve o visor pra ver qm ta disponivel e depois avisa q leu o visor
      sem_wait(cli->le_visor);
      barbeiro_disponivel = *(cli->visor);
      sem_post(cli->troca_visor);

      // Senta na cadeira do barbeiro pra ngm roubar o lugar
      sem_wait(&cli->cadeiras_barbeiro[barbeiro_disponivel]);
      // printf("Cliente %d sentou na cadeira do barbeiro %d.\n", id, barbeiro_disponivel);

      // Da um tapa no barbeiro caso ele ainda esteja dormindo pra ele cortar o cabelo logo
      sem_post(&cli->servico_barbeiro[barbeiro_disponivel]);

      // Libera uma cadeira pra alguem esperar
      sem_post(cli->barbearia->cadeiras_de_espera);

      // ~ cabelo sendo cortado ~
      sem_wait(&cli->barbeiros_cortando_cabelo[barbeiro_disponivel]);

      // Passa pro prox
      sem_post(&cli->cadeiras_barbeiro[barbeiro_disponivel]);
    }
    
    // else printf("Barbearia cheia! Cliente %d foi embora \n", id);
  }

  // como no final o cliente n volta mais, libera ele
  pthread_cancel(cli->thread_cli);
  pthread_exit(NULL);

  return NULL;
}

int main(int argc, char* argv[]) {
  int i;
  int cli_count = 0; // pro id do cliente
  int visor = -1;

  int n_barbeiros = atoi(argv[1]);
  int x_cadeiras = atoi(argv[2]);
  int z_clientes_min = atoi(argv[3]);

  // printf("Barbeiros: %d\n", n_barbeiros);
  // printf("Cadeiras: %d\n", x_cadeiras);
  // printf("Clientes minimos a atender: %d\n", z_clientes_min);
  // printf("\n");

  if (n_barbeiros <= 0 || x_cadeiras <= 0 || z_clientes_min < 0) {
    // printf("Valor de entrada invalido, nao e possivel abrir a barbearia!\n");
    return 0;
  }

  barbearia* barba = (barbearia*) malloc(sizeof(barbearia));

  barbeiro* barbeiros = (barbeiro*) malloc(n_barbeiros * sizeof(barbeiro));
  sem_t* cadeiras_barbeiro = (sem_t*) malloc(n_barbeiros * sizeof(sem_t));
  sem_t* servico_barbeiro = (sem_t*) malloc(n_barbeiros * sizeof(sem_t));
  sem_t* barbeiros_cortando_cabelo = (sem_t*) malloc(n_barbeiros * sizeof(sem_t));
  int* clientes_atendidos = (int*) malloc(n_barbeiros * sizeof(int));

  pthread_mutex_t mutex;

  sem_t cadeiras_de_espera;
  sem_t troca_visor;
  sem_t le_visor;

  pthread_mutex_init(&mutex, NULL);

  sem_init(&cadeiras_de_espera, 0, x_cadeiras);
  sem_init(&troca_visor, 0, 1);
  sem_init(&le_visor, 0, 0);

  for (i=0; i<n_barbeiros; i++) {
    sem_init(&cadeiras_barbeiro[i], 0, 1);
    sem_init(&servico_barbeiro[i], 0, 0);
    sem_init(&barbeiros_cortando_cabelo[i], 0, 0);

    clientes_atendidos[i] = 0;
  }

  // inicia a barbearia
  barba->n_barbeiros = n_barbeiros;
  barba->x_cadeiras = x_cadeiras;
  barba->z_clientes_min = z_clientes_min;
  barba->fechar = 0; // ela começa aberta neh
  barba->cadeiras_de_espera = &cadeiras_de_espera;
  barba->clientes_atendidos = clientes_atendidos;


  for (i=0; i<n_barbeiros; i++){
    // printf("Criando barbeiro %d \n", i);

    barbeiros[i].id = i;
    barbeiros[i].visor = &visor;
    barbeiros[i].mutex = &mutex; 
    barbeiros[i].barbearia = barba; 
    barbeiros[i].cadeiras_barbeiro = cadeiras_barbeiro; 
    barbeiros[i].servico_barbeiro = servico_barbeiro; 
    barbeiros[i].barbeiros_cortando_cabelo = barbeiros_cortando_cabelo; 
    barbeiros[i].troca_visor = &troca_visor; 
    barbeiros[i].le_visor = &le_visor; 
  }

  for (i=0; i<n_barbeiros; i++){
    pthread_create(&barbeiros[i].thread_barb, NULL, barbeiro_func, &(barbeiros[i]));
  }

  while (barba->fechar != 1) {
    cliente* cli = (cliente*)malloc (sizeof(cliente));
    // printf("Criando cliente %d \n", cli_count);

    cli->id = cli_count;
    cli->visor = &visor;
    cli->barbearia = barba;
    cli->cadeiras_barbeiro = cadeiras_barbeiro;
    cli->servico_barbeiro = servico_barbeiro;
    cli->barbeiros_cortando_cabelo = barbeiros_cortando_cabelo;
    cli->troca_visor = &troca_visor;
    cli->le_visor = &le_visor;

    pthread_create(&cli[0].thread_cli, NULL, cliente_func, &(cli)[0]);

    cli_count++;
  }

  // encerra os semaforos 'ainda rodando'
  for(i=0; i<n_barbeiros; i++){
    sem_post(&servico_barbeiro[i]);
  }

  sem_post(&troca_visor);


  for(i=0; i<n_barbeiros; i++) {
    pthread_join(barbeiros[i].thread_barb, NULL);
  }

  for(i=0; i<n_barbeiros; i++) {
    printf("Barbeiro %d atendeu %d clientes \n", i, barba->clientes_atendidos[i]);
  }

  return 0;
}