#define PINO_RX 13
#define BAUD_RATE 1
#define HALF_BAUD 1000/(2*BAUD_RATE)
#define N_BITS 8

#include "Temporizador.h"

int n_bit = 0;
char data = 0;
int parity_bit;

// Calcula bit de paridade - Par ou impar
bool bitParidade(char dado){
  int qnt_1 = 0;

  for(int i = 0; i < N_BITS; i++) {
    if (bitRead(dado, i) == 1) {
      qnt_1++;
    }
  }

  return !!(qnt_1 % 2);
}

// Rotina de interrupcao do timer1
// O que fazer toda vez que 1s passou?
ISR(TIMER1_COMPA_vect){
  if (digitalRead(CTS) == HIGH) {
    if (n_bit < N_BITS) {
      bitWrite(data, N_BITS - 1 - n_bit, digitalRead(PINO_RX));
      n_bit++;
    } else {
      parity_bit = digitalRead(PINO_RX);
      n_bit = 0;
    }
  }
}

// Executada uma vez quando o Arduino reseta
void setup(){
  //desabilita interrupcoes
  noInterrupts();
  // Configura porta serial (Serial Monitor - Ctrl + Shift + M)
  Serial.begin(9600);
  // Inicializa RX
  pinMode(PINO_RX, [INPUT, OUTPUT])
  // Configura timer
  configuraTemporizador(BAUD_RATE);
  // habilita interrupcoes
  interrupts();
}

// O loop() eh executado continuamente (como um while(true))
void loop ( ) {
  if (digitalRead(RTS) == HIGH) {
    digitalWrite(CTS, HIGH);
  } else {
    if (bitParidade(data) == parity_bit) {
      Serial.println(data);
    }
    digitalWrite(CTS, LOW);
  }
}
