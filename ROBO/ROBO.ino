#include <TFT_eSPI.h>  // Biblioteca para controle do display TFT
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <HardwareSerial.h>
#include <Arduino.h>
#include <ESP32Servo.h>

// Pinos dos servos
#define SERVO_DIREITA 4
#define SERVO_ESQUERDA 15

Servo servo1; // Servo da direita
Servo servo2; // Servo da esquerda

HardwareSerial BTSerial(1); // UART1

// Tela TFT ---------------------------------------------------------

// Inicialização do display TFT
TFT_eSPI tft = TFT_eSPI();

#define M1_IN1 13
#define M1_IN2 14
#define M2_IN1 27
#define M2_IN2 26

// Sensores de linha
#define LINHA_DIR 34
#define LINHA_ESQ 35

int limiteLinha = 1000; // quanto menor, mais sensível

// Sensor ultrassônico
#define TRIG 4
#define ECHO 15

// criar objeto sensor de cor (ganho/pulse time padrão)
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// Classificação da cor
String corDetectada = "Desconhecida";


// ======== VARIÁVEIS ========
unsigned long tempoPulso;
float distancia;

// Medir distância com HC-SR04
float medirDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  tempoPulso = pulseIn(ECHO, HIGH, 30000); // timeout 30ms
  return tempoPulso * 0.034 / 2;
}

int leituraDir;
int leituraEsq;
bool linhaDir;
bool linhaEsq;

bool cor;
int fala;

// Variáveis de controle da piscada
unsigned long ultimoBlink = 0;
int estadoOlho = 0; // 0 = aberto, 1 = fechado

// Tempos diferentes
const unsigned long tempoAberto = 200; 
const unsigned long tempoFechado = 2000;  

void setup() {
  Serial.begin(115200); // Inicializa a comunicação serial para depuração
  // Anexa os servos aos pinos
  servo1.attach(SERVO_DIREITA, 500, 2400);  // (pino, pulso mínimo, pulso máximo)
  servo2.attach(SERVO_ESQUERDA, 500, 2400);

  // Posição inicial
  servo1.write(90); 
  servo2.write(90);

  // Inicializa o display TFT e exibe uma mensagem de inicialização
  tft.init();
  tft.setRotation(1); // Ajuste a rotação conforme necessário
  tft.fillScreen(TFT_BLACK);

  tft.fillEllipse(234, 224, 136, 43, 0xFFFF);
  tft.fillEllipse(237, 206, 145, 39, 0x0);
  tft.fillRoundRect(64, 60, 118, 126, 50, 0xFFFF);
  tft.fillRoundRect(298, 61, 118, 126, 50, 0xFFFF);

  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);
  pinMode(M2_IN1, OUTPUT);
  pinMode(M2_IN2, OUTPUT);

  pinMode(LINHA_DIR, INPUT);
  pinMode(LINHA_ESQ, INPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  parar();

  // inicializar I2C em pinos alternativos: SDA=19, SCL=16
  Wire.begin(19, 16);

  if (!tcs.begin()) {
    Serial.println("TCS34725 não encontrado. Verifique as ligações.");
    while (1);
  }
  Serial.println("TCS34725 OK");

  // RX=32, TX=33
  BTSerial.begin(9600, SERIAL_8N1, 32, 33);



}

void loop() {  

  int leituraDir = analogRead(LINHA_DIR);
  int leituraEsq = analogRead(LINHA_ESQ);

  bool linhaDir = leituraDir < limiteLinha;
  bool linhaEsq = leituraEsq < limiteLinha;

  //Serial.print("D: "); Serial.print(leituraDir); Serial.print(" E: "); Serial.println(leituraEsq); 

  //ler a cor no sensor de cores
  while(corDetectada == "Branco" || corDetectada == "Desconhecida"){
    
    atualizaBlink();

    // ======== LEITURA DO SENSOR DE COR ========
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);

    float soma = r + g + b;
    float redNorm   = (float)r / soma;
    float greenNorm = (float)g / soma;
    float blueNorm  = (float)b / soma;

    // ======== DETECÇÃO DE COR ========
    if (redNorm > greenNorm * 1.2 && redNorm > blueNorm * 1.2) {
      corDetectada = "Vermelho";
    } 
    else if (greenNorm > redNorm * 1.2 && greenNorm > blueNorm * 1.2) {
      corDetectada = "Verde";
    } 
    else if (blueNorm > redNorm * 1.2 && blueNorm > greenNorm * 1.2) {
      corDetectada = "Azul";
    } 
    else if (redNorm > 0.3 && greenNorm > 0.3 && blueNorm < 0.25) {
      corDetectada = "Amarelo";
    } 
    else if (redNorm > 0.25 && greenNorm > 0.25 && blueNorm > 0.25 &&
             fabs(redNorm - greenNorm) < 0.1 &&
             fabs(greenNorm - blueNorm) < 0.1) {
      corDetectada = "Branco";
    } 
    else if (soma < 500 || c < 500) {
      corDetectada = "Preto";
    } 
    else {
      corDetectada = "Indefinido";
    }

    // ======== DEBUG SERIAL ========
    /*Serial.print("R: "); Serial.print(r);
    Serial.print(" G: "); Serial.print(g);
    Serial.print(" B: "); Serial.print(b);
    Serial.print(" C: "); Serial.print(c);
    Serial.print(" | Rn: "); Serial.print(redNorm, 2);
    Serial.print(" Gn: "); Serial.print(greenNorm, 2);
    Serial.print(" Bn: "); Serial.print(blueNorm, 2);
    Serial.print(" Soma: "); Serial.print(soma);
    Serial.print(" => Cor: "); Serial.println(corDetectada);*/


    
  }

  Serial.println(corDetectada);
  //distancia = medirDistancia();
  leituraDir = analogRead(LINHA_DIR);
  leituraEsq = analogRead(LINHA_ESQ);
    
  linhaDir = leituraDir < limiteLinha;
  linhaEsq = leituraEsq < limiteLinha;



  if (Serial.available()) {   // Bluetooth manda dados pelo Serial
    char comando = Serial.read();

    switch (comando) {

      case 'D':
        
        Serial.println("Tocando música DANCE GALINHA");
        break;
        
      case '1':
        corDetectada = "Verde";
        cor = false;
        Serial.println("Tocando música 1 e LED ligado");
        break;

      case '2':
        corDetectada = "Amarelo";
        cor = false;
        Serial.println("Tocando música 2");
        break;

      case '3':
          dancar();
          Serial.println("dançando");
          break;

      default:
        Serial.println("Comando inválido recebido");
        break;
    }
  }

  //proxima fala
  if((corDetectada == "Verde" || corDetectada == "Azul") && cor == false ){ 
    fala++;
    cor = true;

    leituraDir = analogRead(LINHA_DIR);
    leituraEsq = analogRead(LINHA_ESQ);

    linhaDir = leituraDir < limiteLinha;
    linhaEsq = leituraEsq < limiteLinha;

    if(fala == 3){ 
      while (linhaEsq && linhaDir) {
        frente();
        leituraDir = analogRead(LINHA_DIR);
        leituraEsq = analogRead(LINHA_ESQ);
        linhaDir = leituraDir < limiteLinha;
        linhaEsq = leituraEsq < limiteLinha;

        if(!linhaEsq || !linhaDir){ parar(); break; }
      } 
      parar();
    }

    while(true){
     
      leituraDir = analogRead(LINHA_DIR);
      leituraEsq = analogRead(LINHA_ESQ);
  
      if((fala != 1 && fala != 3)){ corDetectada = "Desconhecida"; cor = false; break; }
      else if(corDetectada == "Azul"){ corDetectada = "Desconhecida"; cor = false; break; }
  
      linhaDir = leituraDir < limiteLinha;
      linhaEsq = leituraEsq < limiteLinha;
  
      //distancia = medirDistancia();
        // ======== LÓGICA DO SEGUIDOR ========
        if (!linhaDir && !linhaEsq) {
          // Nenhum sensor detecta linha (ambos no branco) → segue em frente
          frente();
          Serial.println("➡️ Frente");
        }
        else if (linhaEsq && !linhaDir) {
          // Apenas esquerda detecta linha (corrigir para esquerda)
          esquerda();
          Serial.println("⬅️ Esquerda");
        }
        else if (!linhaEsq && linhaDir) {
          // Apenas direita detecta linha (corrigir para direita)
          direita();
          Serial.println("➡️ Direita");
        }
        else if (linhaEsq && linhaDir) {
          // Ambos detectam preto → cruzamento / fim de linha
          parar();
          Serial.println("✖ Parado (cruzamento)");
          cor = false;
          break;
        }
      
  }
    parar();

    if(fala < 10 ){
     BTSerial.println(fala);
    }


    //primeira fala precisa da tela azul
    if(fala == 1){
      delay(6000);
      
      tft.fillScreen(TFT_BLUE);

      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(5);
      tft.setFreeFont();
      tft.drawString("reiniciando...", 48, 221);

      tft.setTextSize(10);
      tft.drawString("Erro!", 101, 96);


      delay(2000);
      tft.fillScreen(TFT_BLACK);

      tft.fillEllipse(234, 224, 136, 43, 0xFFFF);
      tft.fillEllipse(237, 206, 145, 39, 0x0);
      tft.fillRoundRect(64, 60, 118, 126, 50, 0xFFFF);
      tft.fillRoundRect(298, 61, 118, 126, 50, 0xFFFF);
      fala++;
      BTSerial.println(fala);
    }
    else if(fala == 4) { delay(3000);}
    else if(fala == 6){
      delay(500);
      direita();
      delay(600);
      parar();
      delay(600);
      esquerda();
      delay(600);
      parar();
    }
    else if(fala == 7){
      delay(4500);
      fala++;
      BTSerial.println(fala);
    }
    
    
    corDetectada = "Desconhecida";
    cor = false;
  }
  //Toca o sino
  else if(corDetectada == "Amarelo" && cor == false){
    BTSerial.println("0");
    corDetectada = "Desconhecida";
    delay(1000);

  }

  delay(1000);

  corDetectada = "Desconhecida";


}


void parar(){
  // Parar
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, LOW);
  
}

void tras(){
  // Frente
  digitalWrite(M1_IN1, HIGH);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, HIGH);
  digitalWrite(M2_IN2, LOW);
}

void frente(){
  // Trás
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, HIGH);

}

void direita() {
  // gira para esquerda
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, LOW);
}

void esquerda() {
  // gira para direita
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, HIGH);
}


void desenhaOlhoAberto() {
  tft.fillScreen(TFT_BLACK);
  tft.fillEllipse(234, 224, 136, 43, 0xFFFF);
  tft.fillEllipse(237, 206, 145, 39, 0x0);
  tft.fillEllipse(357, 112, 60, 3, 0xFFFF);
  tft.fillEllipse(123, 112, 60, 3, 0xFFFF);
}

void desenhaOlhoFechado() {
  tft.fillScreen(TFT_BLACK);
  tft.fillEllipse(234, 224, 136, 43, 0xFFFF);
  tft.fillEllipse(237, 206, 145, 39, 0x0);
  tft.fillRoundRect(64, 60, 118, 126, 50, 0xFFFF);
  tft.fillRoundRect(298, 61, 118, 126, 50, 0xFFFF);
}

void atualizaBlink() {
  unsigned long agora = millis();

  if (estadoOlho == 0 && (agora - ultimoBlink >= tempoAberto)) {
    // Estava aberto, agora fecha
    desenhaOlhoFechado();
    estadoOlho = 1;
    ultimoBlink = agora;
  }
  else if (estadoOlho == 1 && (agora - ultimoBlink >= tempoFechado)) {
    // Estava fechado, agora abre
    desenhaOlhoAberto();
    estadoOlho = 0;
    ultimoBlink = agora;
  }
}

void dancar() {
  Serial.println("💃 Iniciando a dança!");

  unsigned long inicio = millis();           // Marca o tempo inicial
  unsigned long duracao = 220000;            // 3 min 40s em milissegundos

  while (millis() - inicio < duracao) {
    // Passo 1 - Vai pra frente
    frente();
    delay(500);
    parar();
    delay(300);

    // Passo 2 - Vai pra trás
    tras();
    delay(500);
    parar();
    delay(300);

    // Passo 3 - Gira para direita
    direita();
    delay(600);
    parar();
    delay(300);

    // Passo 4 - Gira para esquerda
    esquerda();
    delay(600);
    parar();
    delay(300);

    // Passo 5 - Frente e trás rapidinho (pulsando)
    for (int i = 0; i < 3; i++) {
      frente();
      delay(250);
      tras();
      delay(250);
    }
    parar();
    delay(400);

    // Passo final - Gira em círculo pequeno
    direita();
    delay(1200);
    parar();
    delay(500);
  }

  Serial.println("🎉 Dança finalizada após 3:40!");
}

void braco(){
  // Subindo de 0 até 50
  for (int pos = 0; pos <= 50; pos++) {
    servo1.write(pos);
    servo2.write(pos);
    delay(15);
  }

  delay(500); // pausa em 50°

  // Descendo de 50 até 0
  for (int pos = 50; pos >= 0; pos--) {
    servo1.write(pos);
    servo2.write(pos);
    delay(15);
  }
}
