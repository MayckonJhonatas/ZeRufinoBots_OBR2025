#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

SoftwareSerial mp3Serial(11, 10); // RX, TX para DFPlayer
DFRobotDFPlayerMini mp3;

#define LED 2

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(9600);     // Aqui está o Bluetooth (HC-05 ligado em RX/TX)
  mp3Serial.begin(9600);  // Aqui está o DFPlayer

  if (!mp3.begin(mp3Serial)) {
    Serial.println("Falha ao inicializar DFPlayer Mini!");
    while (true);
  }
  
  mp3.volume(25); // Volume de 0 a 30
  Serial.println("Sistema pronto, aguardando comando via Bluetooth...");
}

void loop() {
  if (Serial.available()) {   // Bluetooth manda dados pelo Serial
    char comando = Serial.read();

    switch (comando) {
      case 'B':
        mp3.play(11);   // 0001.mp3
        Serial.println("Tocando BOAS VINDAS");
        break;

      case 'D':
        mp3.play(12);   // 0001.mp3
        Serial.println("Tocando música DANCE GALINHA");
        break;
        
      case '1':
        mp3.play(1);   // 0001.mp3
        Serial.println("Tocando música 1 e LED ligado");
        break;

      case '2':
        mp3.play(2);   // 0002.mp3
        Serial.println("Tocando música 2");
        break;

      case '3':
        mp3.play(3);   // 0003.mp3
        Serial.println("Tocando música 3" );
        break;

      case '4':
        mp3.play(4);   // 0003.mp3
        Serial.println("Tocando música 4");
        break;
      case '5':
        mp3.play(5);   // 0003.mp3
        Serial.println("Tocando música 5");
        break;
      case '6':
        mp3.play(6);   // 0003.mp3
        Serial.println("Tocando música 6");
        break;
      case '7':
        mp3.play(7);   // 0003.mp3
        Serial.println("Tocando música 7");
        break;
      case '8':
        mp3.play(8);   // 0003.mp3
        Serial.println("Tocando música 8");
        break;
      case '9':
        mp3.play(9);   // 0003.mp3
        Serial.println("Tocando música 9");
        break;
      case '0':
        mp3.play(10);   // 0003.mp3
        Serial.println("Tocando som do sino e ligando o led");
        digitalWrite(LED, HIGH);
        delay(800);
        digitalWrite(LED, LOW);
        delay(500);
        digitalWrite(LED, HIGH);
        delay(700);
        digitalWrite(LED, LOW);
        delay(600);
        digitalWrite(LED, HIGH);
        delay(700);
        digitalWrite(LED, LOW);
        delay(200);
        digitalWrite(LED, HIGH);
        delay(900);
        digitalWrite(LED, LOW);
        delay(200);
        digitalWrite(LED, HIGH);
        delay(800);
        digitalWrite(LED, LOW);
        delay(500);
        digitalWrite(LED, HIGH);
        delay(700);
        digitalWrite(LED, LOW);
        delay(600);
        digitalWrite(LED, HIGH);
        delay(700);
        digitalWrite(LED, LOW);
        delay(200);
        digitalWrite(LED, HIGH);
        delay(900);
        digitalWrite(LED, LOW);
        break;

      default:
        Serial.println("Comando inválido recebido");
        break;
    }
  }
}
