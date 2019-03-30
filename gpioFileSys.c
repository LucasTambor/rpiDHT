#include "gpioFileSys.h"


// Funcoes de Suporte para Execucao do Programa
int GPIOExport(int pin) {
  int BUFFER_MAX = 3;
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;
    
  // Abre o arquivo de export com permissao de somente escrita
  fd = open("/sys/class/gpio/export", O_WRONLY);
  if (fd == -1){
    fprintf(stderr, "Falha no export do pino: %d\n", pin);
    return -1;
  }
  
  // Escreve o numero do pino no arquivo de export do Linux em um buffer
  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);

  // E coloca esse buffer no arquivo propriamente
  if (write(fd, buffer, bytes_written) == -1) {
    fprintf(stderr, "Falha no export do pino: %d\n", pin);
    return -1;
  }

  close(fd);
  // Retorna 0 indicando OK
  return 0;
}

// Funcoes de Suporte para Execucao do Programa
int GPIOUnexport(int pin) {
	int BUFFER_MAX = 3;
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;
  
  // Abre o arquivo de export com permissao de somente escrita
  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (fd == -1) {
    fprintf(stderr, "Falha no export do pino: %d\n", pin);
    return -1;
  }
  
  // Escreve o numero do pino no arquivo de export do Linux em um buffer
  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  // E coloca esse buffer no arquivo propriamente
  write(fd, buffer, bytes_written);
  close(fd);
  // Retorna 0 indicando OK
  return 0;
}

int GPIODirection(int pin, int dir) {
  int DIRECTION_MAX = 35;
  int BYTES_MAX = 3;
  char path[DIRECTION_MAX];
  int fd;
  
  snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
  fd = open(path, O_WRONLY);
  
  if (fd == -1) {
    fprintf(stderr, "Falha em abrir o pino [%d] para direcao\n", pin);
    return -1;
  }
  
  if (write(fd, ((dir == IN) ? "in" : "out"), BYTES_MAX) == -1) {
    fprintf(stderr, "Falha em definir o pino [%d] para entrada/saida\n", pin);
    return -1;
  }
  
  close(fd);
  return 0;  
}

int GPIORead(int pin) {
  int fd;
  const int PIN_NUM_MAX = 30;
  const int PIN_VAL_MAX = 3;
  char path[PIN_NUM_MAX];
  char value_str[PIN_VAL_MAX];
  
  // Formata a nossa string de caminho com base no pino informado
  snprintf(path, PIN_NUM_MAX, "/sys/class/gpio/gpio%d/value", pin);
  
  // Abrindo arquivo com permissao de somente leitura
  fd = open(path, O_RDONLY);
  
  if (fd == -1) {
    fprintf(stderr, "Falha na abertura do pino [%d] para leitura\n", pin);
    return -1;
  }
  
  if (read(fd, value_str, PIN_VAL_MAX) == -1) {
    fprintf(stderr, "Falha na leitura do pino [%d]\n", pin);
    return -1;
  }
  
  close(fd);
  
  // Retorna o numero inteiro do valor lido do pino (0 ou 1)
  // atoi = Ascii to Integer - converte "texto" em inteiro
  return atoi(value_str);
}


int GPIOWrite(int pin, int value) {
  int fd;
  const int PIN_NUM_MAX = 30;
  const int PIN_VAL_MAX = 3;
  char path[PIN_NUM_MAX];

  snprintf(path, PIN_NUM_MAX, "/sys/class/gpio/gpio%d/value", pin);

  fd = open(path, O_WRONLY);
  if (fd == -1) {
    fprintf(stderr, "Falha na abertura do pino [%d] para escrita\n", pin);
    return -1;
  }

  if (write(fd, ((value == HIGH) ? "1" : "0"), PIN_VAL_MAX) == -1) {
    fprintf(stderr, "Falha na escrita digital do pino [%d]\n", pin);
    return -1;
  }

  close(fd);
  return 0;  
}
