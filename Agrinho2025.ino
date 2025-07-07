#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===================== CONFIGURAÇÃO DO DISPLAY OLED =====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===================== CONFIGURAÇÃO DOS SERVOS =====================
#define pinGarra 2
#define pinBase 3
#define pinBraco 4
#define pinAntebraco 5
#define pinSeparador 26  // Pino para o servo da porta

#define RED_MAX 150      // R não pode passar disso para ser vermelho
#define MIN_GB 150       // G e B devem ser maiores que isso

// ===================== CONFIGURAÇÃO DA ESTEIRA =====================
const int ENA = 6;
const int IN1 = 7;
const int IN2 = 8;

// ===================== CONFIGURAÇÃO DO SENSOR ULTRASSÔNICO =====================
const int trigPin = 22;
const int echoPin = 23;

// ===================== CONFIGURAÇÃO DO SENSOR DE COR =====================
const int S0 = 9;
const int S1 = 10;
const int S2 = 11;
const int S3 = 12;
const int outPinCor = 13;

// ===================== CONFIGURAÇÃO DOS BOTÕES =====================
const int botaoModo = 24;    // Pino para o botão de modo
const int botaoManual = 25;  // Pino para o botão manual
int modoAtual = 0;           // 0 = Automático, 1 = Manual

// Variáveis para debounce dos botões
int ultimoEstadoBotao = HIGH;
int estadoBotao;
unsigned long ultimoDebounceTime = 0;
const int debounceDelay = 50;

int estadoBotaoManual = HIGH;
int ultimoEstadoBotaoManual = HIGH;
unsigned long ultimoDebounceManual = 0;

// ===================== CONFIGURAÇÃO DOS POTENCIÔMETROS (MODO MANUAL) =====================
const int potGarra = A0;     // Potenciômetro 1 - Garra
const int potBase = A1;      // Potenciômetro 2 - Base
const int potBraco = A2;     // Potenciômetro 3 - Braço
const int potAntebraco = A3; // Potenciômetro 4 - Antebraço

Servo garra, base, braco, antebraco, porta;  // Renomeado separador para porta

// ===================== POSIÇÕES DA PORTA =====================
const int PORTA_ABERTA = 90;    // Posição normal (aberta)
const int PORTA_FECHADA = 60
;    // Posição para objetos vermelhos

// ===================== POSIÇÕES DAS ROTINAS =====================
const int rotinas[17][4] = {
  // Garra, Base,  Braço,  Antebraço
  {2496, 1008, 2290, 1460}, {2496, 1008, 1823, 1460}, 
  {2496, 1008, 1823, 1460}, {2496, 1008, 1823, 1460},
  {2496, 1008, 1700, 1460}, {2496, 1008, 1700, 1530},
  {2496, 1008, 1700, 1530}, {1747, 1008, 1700, 1530},
  {1747, 1008, 2036, 1530}, {1747, 1008, 2036, 1300},
  {1747, 1008, 2036, 1300}, {1747, 1930, 2036, 1300},
  {1747, 2000, 2279, 1300}, {1747, 1930, 2279, 1428},
  {1747, 2000, 1802, 1999}, {1747, 1930, 1802, 1999},
  {2496, 2000, 1802, 1999}
};

const int posicaoInicial[4] = {2496, 1008, 2290, 1460};

// ===================== PARÂMETROS DE MOVIMENTO =====================
const int velocidadeMaxima = 60;
const int velocidadeMinima = 5;
const int aceleracao = 3;
const int delayEntrePassos = 10;
const int tolerancia = 15;

// ===================== PARÂMETROS DA ESTEIRA =====================
const int velocidadeEsteira = 150;
const int velocidadePosicionamento = 100;
const float distanciaLimite = 10.0;
const unsigned long tempoLeituraCor = 2000;
const unsigned long tempoMovimentoFinal = 3000;
const unsigned long tempoPosicionamento = 650;  //distancia do sensor cor
const int delayAntesEsteira = 500;

// ===================== VARIÁVEIS DE CONTROLE =====================
enum EstadoSistema {
  EXECUTANDO_ROTINA,
  ESPERANDO_OBJETO,
  POSICIONANDO_OBJETO,
  LENDO_COR,
  MOVENDO_OBJETO
};

EstadoSistema estadoAtual = EXECUTANDO_ROTINA;
unsigned long tempoEstado = 0;
int rotinaAtual = 0;
bool portaFechada = false; // Variável para controlar o estado da porta

// ===================== VARIÁVEIS DO SENSOR DE COR =====================
int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;
String corDetectada = "";

// ===================== CONTADORES DE OBJETOS =====================
int contadorVermelhos = 0;
int contadorOutros = 0;

void setup() {
  Serial.begin(9600);

  // Inicializa o display OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Falha na alocação do SSD1306"));
    for(;;); // Trava o programa se não conseguir inicializar o display
  }

 // Limpa o buffer do display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Inicializa servos
  garra.attach(pinGarra, 500, 2500);
  base.attach(pinBase, 500, 2500);
  braco.attach(pinBraco, 500, 2500);
  antebraco.attach(pinAntebraco, 500, 2500);
  porta.attach(pinSeparador);  // Inicializa servo da porta
  porta.write(PORTA_ABERTA); // Inicia com a porta aberta
  
  // Configura pinos da esteira
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  // Configura sensor ultrassônico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Configura sensor de cor
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(outPinCor, INPUT);
  
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
  
  // Configura botões
  pinMode(botaoModo, INPUT_PULLUP);
  pinMode(botaoManual, INPUT_PULLUP);
  
  // Configura potenciômetros (modo manual)
  pinMode(potGarra, INPUT);
  pinMode(potBase, INPUT);
  pinMode(potBraco, INPUT);
  pinMode(potAntebraco, INPUT);
  
  // Move para posição inicial
  moverParaPosicao(posicaoInicial[0], posicaoInicial[1], posicaoInicial[2], posicaoInicial[3]);
  
  Serial.println("Sistema Iniciado");
  Serial.println("Modo atual: Automático");
  
  // Atualiza display com informações iniciais
  atualizarDisplay("Agrinho", "Modo: Automatico", "Cor: --", "V:0 O:0");
}

void loop() {
  verificarBotaoModo();
  
  if (modoAtual == 0) {
    executarModoAutomatico();
  } else {
    executarModoManual();
    verificarBotaoManual();
  }
  
  // Atualiza o display periodicamente
  static unsigned long ultimaAtualizacaoDisplay = 0;
  if (millis() - ultimaAtualizacaoDisplay > 500) {
    atualizarDisplayPrincipal();
    ultimaAtualizacaoDisplay = millis();
  }
}

// ===================== FUNÇÕES DO DISPLAY =====================
void atualizarDisplay(String linha1, String linha2, String linha3, String linha4) {
  display.clearDisplay();
  
  // Linha 1 em tamanho maior
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println(linha1);
  
  // Linhas restantes em tamanho normal
  display.setTextSize(1);
  display.setCursor(0,18);
  display.println(linha2);
  display.setCursor(0,28);
  display.println(linha3);
  display.setCursor(0,38);
  display.println(linha4);
  
  display.display();
}

void atualizarDisplayPrincipal() {
  String modo = (modoAtual == 0) ? "Automatico" : "Manual";
  String cor = (corDetectada == "") ? "--" : corDetectada;
  atualizarDisplay(
    "Agrinho",
    "Modo: " + modo,
    "Cor: " + cor,
    "V:" + String(contadorVermelhos) + " O:" + String(contadorOutros)
  );
}

// ===================== FUNÇÕES DO MODO MANUAL =====================
void executarModoManual() {
  // Lê os valores dos potenciômetros
  int valGarra = map(analogRead(potGarra), 0, 1023, 1500, 2500);
  int valBase = map(analogRead(potBase), 0, 1023, 1000, 2000);
  int valBraco = map(analogRead(potBraco), 0, 1023, 1400, 2200);
  int valAntebraco = map(analogRead(potAntebraco), 0, 1023, 1300, 2000);
  
  // Move os servos
  garra.writeMicroseconds(valGarra);
  base.writeMicroseconds(valBase);
  braco.writeMicroseconds(valBraco);
  antebraco.writeMicroseconds(valAntebraco);
  
  delay(50);
}

void verificarBotaoManual() {
  int leitura = digitalRead(botaoManual);
  
  if (leitura != ultimoEstadoBotaoManual) {
    ultimoDebounceManual = millis();
  }
  
  if ((millis() - ultimoDebounceManual) > debounceDelay) {
    if (leitura != estadoBotaoManual) {
      estadoBotaoManual = leitura;
      
      if (estadoBotaoManual == LOW) {
        Serial.println("Botão manual pressionado - Executando rotina da esteira");
        
        ligarEsteira();
        delay(tempoPosicionamento);
        desligarEsteira();
        
        detectarCor();
        Serial.print("Cor detectada: ");
        Serial.println(corDetectada);
        
        // Atualiza contador
        if (corDetectada == "VERMELHO") {
          contadorVermelhos++;
        } else {
          contadorOutros++;
        }
        
        // Ação da porta no modo manual
        if (corDetectada == "VERMELHO") {
          porta.write(PORTA_FECHADA);
          portaFechada = true;
          Serial.println("Porta FECHADA para objeto VERMELHO");
        } else {
          porta.write(PORTA_ABERTA);
          portaFechada = false;
          Serial.println("Porta mantida ABERTA");
        }
        
        ligarEsteira();
        delay(tempoMovimentoFinal);
        desligarEsteira();
        
        // Reabre a porta após o movimento
        porta.write(PORTA_ABERTA);
        portaFechada = false;
        
        Serial.println("Rotina manual da esteira concluída");
      }
    }
  }
  
  ultimoEstadoBotaoManual = leitura;
}

void verificarBotaoModo() {
  int leitura = digitalRead(botaoModo);
  
  if (leitura != ultimoEstadoBotao) {
    ultimoDebounceTime = millis();
  }
  
  if ((millis() - ultimoDebounceTime) > debounceDelay) {
    if (leitura != estadoBotao) {
      estadoBotao = leitura;
      
      if (estadoBotao == LOW) {
        modoAtual = !modoAtual;
        
        if (modoAtual == 0) {
          Serial.println("Modo Automático ativado");
          estadoAtual = EXECUTANDO_ROTINA;
          rotinaAtual = 0;
          moverParaPosicao(posicaoInicial[0], posicaoInicial[1], posicaoInicial[2], posicaoInicial[3]);
          porta.write(PORTA_ABERTA); // Garante que a porta está aberta
          portaFechada = false;
        } else {
          Serial.println("Modo Manual ativado");
          desligarEsteira();
          porta.write(PORTA_ABERTA); // Garante que a porta está aberta
          portaFechada = false;
        }
      }
    }
  }
  
  ultimoEstadoBotao = leitura;
}

// ===================== FUNÇÕES DO MODO AUTOMÁTICO =====================
void executarModoAutomatico() {
  switch(estadoAtual) {
    case EXECUTANDO_ROTINA:
      executarProximaRotina();
      break;
      
    case ESPERANDO_OBJETO:
      monitorarSensorUltrassonico();
      break;
      
    case POSICIONANDO_OBJETO:
      if (millis() - tempoEstado >= tempoPosicionamento) {
        desligarEsteira();
        estadoAtual = LENDO_COR;
        tempoEstado = millis();
        Serial.println("Objeto posicionado. Lendo cor...");
      }
      break;
      
    case LENDO_COR:
      detectarCor();
      if (millis() - tempoEstado >= tempoLeituraCor) {
        Serial.print("Cor detectada: ");
        Serial.println(corDetectada);
        
        // Atualiza contador
        if (corDetectada == "VERMELHO") {
          contadorVermelhos++;
        } else {
          contadorOutros++;
        }
        
        // Controla a porta baseado na cor detectada
        if (corDetectada == "VERMELHO") {
          porta.write(PORTA_FECHADA);
          portaFechada = true;
          Serial.println("Porta FECHADA para objeto VERMELHO");
        } else {
          porta.write(PORTA_ABERTA);
          portaFechada = false;
          Serial.println("Porta mantida ABERTA");
        }
        
        estadoAtual = MOVENDO_OBJETO;
        ligarEsteira();
        tempoEstado = millis();
        Serial.println("Movimentando objeto por 3 segundos...");
      }
      break;
      
    case MOVENDO_OBJETO:
      if (millis() - tempoEstado >= tempoMovimentoFinal) {
        desligarEsteira();
        
        // Reabre a porta após o movimento
        porta.write(PORTA_ABERTA);
        portaFechada = false;
        
        estadoAtual = EXECUTANDO_ROTINA;
        rotinaAtual = 0;
        Serial.println("Movimento completo. Reiniciando ciclo.");
      }
      break;
  }
}

void executarProximaRotina() {
  if (rotinaAtual < 17) {
    moverParaPosicao(rotinas[rotinaAtual][0], rotinas[rotinaAtual][1], 
                    rotinas[rotinaAtual][2], rotinas[rotinaAtual][3]);
    rotinaAtual++;
  } else {
    rotinaAtual = 0;
    delay(delayAntesEsteira);
    ligarEsteira();
    estadoAtual = ESPERANDO_OBJETO;
    Serial.println("Esteira ligada - Aguardando objeto...");
  }
}

void monitorarSensorUltrassonico() {
  float distancia = medirDistancia();
  
  if (distancia <= distanciaLimite) {
    estadoAtual = POSICIONANDO_OBJETO;
    tempoEstado = millis();
    analogWrite(ENA, velocidadePosicionamento);
    Serial.println("Objeto detectado! Posicionando para leitura de cor...");
  }
}

void detectarCor() {
  // Ler valores RGB do sensor
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  redFrequency = pulseIn(outPinCor, LOW);
  
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  greenFrequency = pulseIn(outPinCor, LOW);
  
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  blueFrequency = pulseIn(outPinCor, LOW);

  // Mostra valores no Serial (para calibração)
  Serial.print("R: ");
  Serial.print(redFrequency);
  Serial.print(" G: ");
  Serial.print(greenFrequency);
  Serial.print(" B: ");
  Serial.println(blueFrequency);

  // Lógica de detecção
  if (redFrequency <= RED_MAX && greenFrequency > MIN_GB && blueFrequency > MIN_GB) {
    corDetectada = "VERMELHO";
  } 
  else {
    corDetectada = "OUTRA";
  }
}

void moverParaPosicao(int targetGarra, int targetBase, int targetBraco, int targetAntebraco) {
  int currentGarra = garra.readMicroseconds();
  int currentBase = base.readMicroseconds();
  int currentBraco = braco.readMicroseconds();
  int currentAntebraco = antebraco.readMicroseconds();
  
  int velGarra = velocidadeMinima;
  int velBase = velocidadeMinima;
  int velBraco = velocidadeMinima;
  int velAntebraco = velocidadeMinima;

  while(!posicoesIguais(currentGarra, currentBase, currentBraco, currentAntebraco,
                       targetGarra, targetBase, targetBraco, targetAntebraco)) {
    
    atualizaPosicao(garra, currentGarra, targetGarra, velGarra);
    atualizaPosicao(base, currentBase, targetBase, velBase);
    atualizaPosicao(braco, currentBraco, targetBraco, velBraco);
    atualizaPosicao(antebraco, currentAntebraco, targetAntebraco, velAntebraco);
    
    delay(delayEntrePassos);
  }
}

void ligarEsteira() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, velocidadeEsteira);
}

void desligarEsteira() {
  analogWrite(ENA, 0);
}

float medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  float distancia = duration * 0.034 / 2;
  
  return (distancia > 400 || distancia <= 0) ? 999.9 : distancia;
}

void atualizaPosicao(Servo &servo, int &currentPos, int targetPos, int &velocidade) {
  if(abs(currentPos - targetPos) > tolerancia) {
    velocidade = calcularVelocidade(currentPos, targetPos, velocidade);
    currentPos += (currentPos < targetPos) ? velocidade : -velocidade;
    servo.writeMicroseconds(currentPos);
  }
}

int calcularVelocidade(int atual, int target, int velocidadeAtual) {
  int distancia = abs(atual - target);
  if(distancia > 400) return min(velocidadeAtual + aceleracao, velocidadeMaxima);
  else if(distancia < 200) return max(velocidadeAtual - aceleracao, velocidadeMinima);
  return velocidadeAtual;
}

bool posicoesIguais(int g1, int b1, int br1, int a1, int g2, int b2, int br2, int a2) {
  return (abs(g1 - g2) <= tolerancia) &&
         (abs(b1 - b2) <= tolerancia) &&
         (abs(br1 - br2) <= tolerancia) &&
         (abs(a1 - a2) <= tolerancia);
}
