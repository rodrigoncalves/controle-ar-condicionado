/*
  UnB - Universidade de Brasilia
  FGA
  Prova Pratica 1 - Sistemas Embarcados
  Prof. Renato Sampaio
  
  Data: 09/10/2015

  Codigo do Microcontrolador Arduino que e controlado via UART 
  para enviar o valor da temperatura do sensor e ligar/desligar a chave.
  
*/

void setup() {
  pinMode(13, OUTPUT); // LED
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // Start each software serial port
  Serial1.begin(9600);
  //portTwo.begin(9600);
}
const char nome[7] = "Renato";
unsigned char dado = 65;
unsigned char dado_recebe;
int i = 0;
String nome_s = nome;
float temperatura = 25.8;

typedef union {
 float floatingPoint;
 byte bytes[4];
} bytesFloat;

bytesFloat valor;

void loop() {
  
  valor.floatingPoint = temperatura;
  
  if(Serial1.available()) {
    dado_recebe = Serial1.read();
    Serial.print("Dado recebido: ");
    Serial.println(dado_recebe);
    
    if(dado_recebe == 0x05) {
      // Trata requisicao de temepratura
      // Le temepratura do Sensor
      // ..... implementar ....

      // Envia temepratura pela Serial
      Serial1.write(valor.bytes,4);
    } // End IF 1
    else if(dado_recebe == 0xA0) {
      // Trata requisicao de Ligar Chave
      delay(10);
      if (Serial1.available()) {
        dado_recebe = Serial1.read();
        Serial.print(" ");
        Serial.println(dado_recebe);
        
        if(dado_recebe == 0x01) {
          Serial.println("LIGA A CHAVE");
          digitalWrite(13, HIGH);
          Serial1.write(0xA1);
        }
        else if(dado_recebe == 0x00) {
          Serial.println("DESLIGA A CHAVE");
          digitalWrite(13, LOW);
          Serial1.write(0xA1);
        }
      }
    } // End ELSIF 2
    else
      Serial1.write(0xE1);
  }
  delay(10);
}

