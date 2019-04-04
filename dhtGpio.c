/**
 *  UNISAL 2018 - Sistemas Operacionais Embarcados - Linux Embarcado
 *  Controle de GPIO via Sysfs
 */

// Inclusao de bibliotecas necessarias
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "gpioFileSys.h"
#include "DHT.h"


// Definicoes de Referencias para o Codigo
//define APELIDO PRA_ALGUMA_COISA
#define IN		0
#define OUT 	1

#define LOW 	0
#define HIGH 	1

#define LED1	23
#define LED2	24
#define BTN1	25


#define DHT   21
#define DHT_SENSOR_TYPE  DHT_11
// Variaveis de Referencia para Operacao
bool estado_botao = true; // botao eh ativado em false/0 -> pull-up!
bool muda_estado_pisca = false; // inicialmente nao mudamos estado de pisca_led
int led_control = 0;
volatile bool terminateSignal = false;
pthread_mutex_t lock;
pthread_t thread_id_hb;
pthread_t thread_id_led;
pthread_t thread_id_btn;
pthread_t thread_id_dht;

enum LED_ESTADOS {
  VEL0 = 0,
  VEL1,
  VEL2,
  VEL3,
  VEL4,
  VEL5,
  NUM_ESTADOS
};

int VEL_LED[NUM_ESTADOS] = {0, 250, 500, 750, 1000, 1};

int estado_led = VEL0;

// Funcoes de Suporte para Thread de Aplicação
void *thread_heart_beat(void *arg);
void *thread_led_ctrl(void *arg);
void *thread_btn_read(void *arg);
void *thread_dht_read(void *arg);
void sigintHandler(int sig_num); 


int main(int argc, char *argv[]) {
  // Quantidade de vezes pra Execucao do codigo
  int repeat = 10;
  int i = 0;

  // Unexporta os pinos
  GPIOUnexport(LED1);
  GPIOUnexport(LED2);
  GPIOUnexport(BTN1);
  // Exporta agora, certo
  GPIOExport(LED1);
  GPIOExport(LED2);
  GPIOExport(BTN1);
  // Define as direcoes de cada um
  GPIODirection(LED1, OUT);
  GPIODirection(LED2, OUT);
  GPIODirection(BTN1, IN);
  
	// Inicializa threads e mutexes
  if (pthread_mutex_init(&lock, NULL) != 0) {
    fprintf(stderr, "Falha na iniciliazacao do Mutex \n");
    return 1;
  }
  
  if (pthread_create(&thread_id_hb, NULL, thread_heart_beat, NULL) != 0) {
    fprintf(stderr, "Falha na criacao da thread de heart beat \n");
    return 1;
  } 

  if (pthread_create(&thread_id_led, NULL, thread_led_ctrl, NULL) != 0) {
    fprintf(stderr, "Falha na criacao da thread de led control \n");
    return 1;
  }
 
  if (pthread_create(&thread_id_btn, NULL, thread_btn_read, NULL) != 0) {
    fprintf(stderr, "Falha na criacao da thread de botao \n");
    return 1;
  }
  if (pthread_create(&thread_id_dht, NULL, thread_dht_read, NULL) != 0) {
    fprintf(stderr, "Falha na criacao da thread de leitura DHT \n");
    return 1;
  }

  signal(SIGINT, sigintHandler);
  
  // Trava momentaneamente nosso programa aqui
  while(!terminateSignal) {}
  
  // Finaliza as threads
  pthread_join(thread_id_btn, NULL);
  pthread_join(thread_id_hb, NULL);
  pthread_join(thread_id_led, NULL);
  pthread_join(thread_id_dht, NULL);

  // Quando a gente ta manipulando IOs... Eh bom "unexport" quando acaba
  pthread_mutex_destroy(&lock);

  GPIOUnexport(LED1);
  GPIOUnexport(LED2);
  GPIOUnexport(BTN1);

  return 0;
}

void *thread_heart_beat(void *arg) {
  while(!terminateSignal) {
    usleep(0.5 * 1000 * 1000);
    GPIOWrite(LED1, HIGH);
    usleep(0.5 * 1000 * 1000);
    GPIOWrite(LED1, LOW);
  }

  return NULL;
}

void *thread_led_ctrl(void *arg) {
  while(!terminateSignal) {
    // Soh precisa da trava/destrava pra leitura de estado pra mudar o comportamento
		pthread_mutex_lock(&lock);
    if (muda_estado_pisca) {
      if (estado_led == VEL5) {
        estado_led = VEL0;
      } else {
      	estado_led++;
      }
      muda_estado_pisca = false;
    }
    pthread_mutex_unlock(&lock);
    
    switch(estado_led) {
      case VEL0:
        GPIOWrite(LED2, LOW);
        break;
      case VEL1:
      case VEL2:
      case VEL3:
      case VEL4:
        usleep(VEL_LED[estado_led] * 1000);
        GPIOWrite(LED2, HIGH);
        usleep(VEL_LED[estado_led] * 1000);
        GPIOWrite(LED2, LOW);
      	break;
      case VEL5:
        GPIOWrite(LED2, HIGH);
        break;
    }
  }

  return NULL;
}

void *thread_btn_read(void *arg) {
  while(!terminateSignal) {
		usleep(0.250 * 1000 * 1000);
    // ve se da pra mexer na variavel de estado do botao
    estado_botao = (GPIORead(BTN1) == 0 ? false : true);
    if (!estado_botao) {
      fprintf(stderr, "Botao pressionado!\n");
      // Usamos o muda_estado_pisca para controlar o estado do led2/pisca controlado
      pthread_mutex_lock(&lock);
      muda_estado_pisca = true;
      pthread_mutex_unlock(&lock); 
    }
    // Liberamos a trava de acesso
  }
  
  return NULL;
}


void *thread_dht_read(void *arg) {
  float temperatura = 0;
  float umidade = 0;

  dht_configure(DHT_SENSOR_TYPE, DHT);

  while(!terminateSignal)
  {
    if(read_dht_data(&temperatura, &umidade))
    {
      fprintf(stderr, "PROBLEMA NA LEITURA DO DHT");
    }else{
      fprintf(stderr, "Leitura bem sucedida: \n");
      fprintf(stderr, "Temperatura: %.1f\n", temperatura);
      fprintf(stderr, "Umidade: %.1f\n", umidade);

    }
    usleep(2*1000*1000);

  } 

  return NULL;
}

// Função para lidar com sinalização de eventos
void sigintHandler(int sig_num) 
{ 
  fprintf(stderr, "\n Terminate \n"); 
  terminateSignal = 1;
} 

