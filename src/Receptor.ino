#include "Temporizador.h"

#define BAUD_RATE 1
#define HALF_BAUD 1000/(2*BAUD_RATE)
#define TOTAL_N_BITS_CHAR 8
#define CTS 11
#define RTS 12
#define PINO_RX 13

enum TransmissionState {
  Idle,
  InProgress,
  Finished,
}

// global variables
int n_data_bits = 0;
char data = 0;
boolean parity_bit;
TransmissionState transmissionState = Idle;

bool calculateParityBit(char data){
  int qnt_ones = 0;
  for(int i = 0; i < TOTAL_N_BITS_CHAR; i++) {
    if (bitRead(data, i) == 1) {
      qnt_ones++;
    }
  }

  // the parity bit 0 for a pair
  // quantity and 1 for an odd one
  return qnt_ones % 2;
}

ISR(TIMER1_COMPA_vect){
  if (transmissionState == InProgress) {
    // delay necessary to maintain sync with the emissor - necessary to avoid
    // the attempt of reading a bit before they were able to transmit it
    delay(HALF_BAUD);

    // if not all data bits were yet read
    if (n_data_bits < TOTAL_N_BITS_CHAR) {
      boolean data_bit = digitalRead(PINO_RX);
      bitWrite(data, n_data_bits, data_bit); // write from LSB to MSB
    } else if (n_data_bits == TOTAL_N_BITS_CHAR) { // if the parity bit is missing
      parity_bit = digitalRead(PINO_RX);
      transmissionState = Finished;
    }
    n_data_bits++;
  }
}

void setup(){
  // disable interruptions
  noInterrupts();
  // set the serial port up
  Serial.begin(9600);
  // set the pins up (RX, RTS and CTS)
  pinMode(PINO_RX, INPUT);
  pinMode(RTS, INPUT);
  pinMode(CTS, OUTPUT);
  // set the timer up
  configuraTemporizador(BAUD_RATE);
  // enable interruptions
  interrupts();
}

void loop () {
  if (digitalRead(RTS) == HIGH) {
    // start transmission
    if (transmissionState == Idle) {
      n_data_bits = 0;
      transmissionState = InProgress; /* started_transmission = true; */
      digitalWrite(CTS, HIGH);
      iniciaTemporizador();
    }
  } else if (transmissionState == Finished) {
    // if the data was successfully transmitted,
    // print it to the serial monitor (output)
    if (calculateParityBit(data) == parity_bit) {
      Serial.println(data);
    }

    // stop transmission
    digitalWrite(CTS, LOW);
    paraTemporizador();
    transmissionState = Idle;
  }
}
