// =============================================================================
//                    TREINADOR DE CÓDIGO MORSE - CW TRAINER V-1
// =============================================================================
// 
//  Desenvolvido por Everton Rodrigues
// 
//  REDES SOCIAIS:
//  • Facebook : https://www.facebook.com/everton.trx
//  • YouTube  : https://www.youtube.com/@evertonee
//  • TikTok   : https://www.tiktok.com/@evertoneletronica
//  • Github   : https://github.com/EvertonPiloto01
//  • Site     : https://www.eletronicaelementar.com.br
// 
//  CONTATO PARA PROJETOS:
//  • Desenvolvimento de circuitos e design de placas
//  • Email: evertonspeedyrage@gmail.com
// 
//  Compartilhe este código com todos os amigos radioamadores,
//  radioescutas e entusiastas da comunicação!
// 
//  Aproveite bem!
// 
// =============================================================================

#include <Arduino.h>
#include <U8x8lib.h>

// Configuração do Display OLED
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);// Para oled com driver SH1106

// Definições de pinos
#define keyLineInput 2       // Entrada manipulador D2 ao GND
#define audioOutputPin 9     // Saída de áudio no pino 9 (PWM) usar capacitor de 10uf
#define indicatorLED 13      // LED indicador (led pin 13 onboard)

/// CONFIGURAÇÕES DE ÁUDIO ////////////////////

int audioFrequency = 600;           // Frequência do tom em Hz
bool toneOutput = true;             // Controle de áudio
bool sidetoneEnabled = true;        // Sidetone sempre ligado

/// CONFIGURAÇÕES DE DEBOUNCE ////////////////////

const int DEBOUNCE_DELAY = 20;        // 20ms de debounce
const int MIN_PULSE_DURATION = 15;    // Duração mínima de pulso válido
const unsigned long CLEAR_DISPLAY_HOLD_TIME = 1000;  // //PARA APAGAR AS LETRAS DO DISPLEY SEGURE PRECIONADO POR 1 SEGUNDO!

/// VARIÁVEIS DE DEBOUNCE PARA KEY ////////////////////

int buttonState;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

bool keyLine = false;
bool lastStableState = HIGH;

unsigned long keyDownTime = 0;
unsigned long keyUpTime = 0;
unsigned long pulseDuration = 0;
bool manualModeActive = false;  // Flag para indicar quando o modo manual está ativo
bool clearDisplayTriggered = false;  // Flag para indicar que a limpeza foi acionada

/// TIMING VARIABLES ////////////////////

unsigned long timeTrack = millis();

long thresholdGeometricMean = 100;

long keyLineNewEvent = 0;
long keyLinePriorEvent = 0;

long shortEventHistogramList[10];
long longEventHistogramList[10];
long spaceEventHistogramList[10];
int eventHistogramTrack = 0;

unsigned long spaceDuration = 0;
unsigned long oldSpaceDuration = 0;
unsigned long spaceDurationReference = 0;

float wordSpaceTiming = 6.5; // detecta espaço entre as palavras
unsigned long wordSpaceDuration = 0;
unsigned long wordSpaceDurationReference = 0;

/// DECODE VARIABLES ////////////////////

float compareFactor = 1.8;
bool characterStep = false;
char elementSequence[10] = "";
byte decodeChar = 0;
String decodeProSign = "";
bool wordStep = false;

int WPM = 0;
int WPMHistogramList[20];
int wpmHistogramTrack = 0;

/// DISPLAY OLED VARIABLES ///////////////////////

const int OLED_COLS = 16;
const int OLED_ROWS = 8;
int cursorCol = 0;
int cursorRow = 2;  // Começar na linha 2 para as letras
String morseElements = "";
String decodedText = "";

/// PROTÓTIPOS DAS FUNÇÕES ////////////////////

void printDecodedChar(int charASCII);
void clearDisplayArea();
void processPulse();
void morseDecode();

/// Função para limpar toda a área de texto do display ////////////////////

void clearDisplayArea() {
  // Limpar linha superior (Morse)
  u8x8.setCursor(0, 0);
  u8x8.print("                ");
  
  // Limpar área de texto (linhas 2-7)
  for (int r = 2; r < OLED_ROWS; r++) {
    u8x8.setCursor(0, r);
    u8x8.print("                ");
  }
  
  // Resetar cursor
  cursorCol = 0;
  cursorRow = 2;
  morseElements = "";
  decodedText = "";
  
  // Limpar sequência de decodificação
  elementSequence[0] = '\0';
  decodeChar = 0;
  decodeProSign = "";
}

/// Setup /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  
  // Inicializar random seed
  randomSeed(analogRead(0));
  
  pinMode(keyLineInput, INPUT_PULLUP);
  pinMode(indicatorLED, OUTPUT);
  pinMode(audioOutputPin, OUTPUT);
  
  digitalWrite(indicatorLED, LOW);
  noTone(audioOutputPin);
  
  // Estado inicial
  lastButtonState = digitalRead(keyLineInput);
  lastStableState = lastButtonState;
  
  // Inicialização do OLED
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_8x13B_1x2_f);
  
  // Limpar display
  u8x8.clear();
  
  // Mostrar "CW TRAINER" centralizado
  u8x8.setCursor(3, 2);
  u8x8.print("CW TRAINER");
  
  // Aguardar 1 segundo
  delay(1000);
  
  // Limpar display
  u8x8.clear();
  
  // Mostrar "EVERTON RODRIGUES" centralizado (dividido em duas linhas)
  u8x8.setCursor(4, 1);
  u8x8.print("EVERTON");
  u8x8.setCursor(3, 3);
  u8x8.print("RODRIGUES");
  
  // Aguardar 1 segundo
  delay(1000);
  
  // LIMPAR A TELA APÓS O DELAY
  u8x8.clear();
  
  // Restaurar layout inicial do decodificador Morse
  u8x8.setCursor(0, 0);
  u8x8.print("                ");  // Linha superior vazia para Morse
  
  u8x8.setCursor(0, 1);
  u8x8.print("-");    // Linha separadora
  
  // Teste rápido de áudio com a frequência configurada
  tone(audioOutputPin, audioFrequency, 200);
  delay(500);
  
  // Inicializar médias
  for (int i = 0; i < 10; i++) {
    shortEventHistogramList[i] = 60;
    longEventHistogramList[i] = 180;
    spaceEventHistogramList[i] = 60;
  }
  for (int i = 0; i < 20; i++) {
    WPMHistogramList[i] = 15;
  }
  
  // Inicializar timeTrack
  timeTrack = millis();
  
  manualModeActive = false;
  clearDisplayTriggered = false;
}

////////////////////////////////// Loop Principal //////////////////////////////////////////////////////////////////////////

void loop() {
  
  timeTrack = millis();
  
  // ===== LEITURA COM DEBOUNCE DA CHAVE CW =====
  int reading = digitalRead(keyLineInput);
  
  if (reading != lastButtonState) {
    lastDebounceTime = timeTrack;
  }
  
  if ((timeTrack - lastDebounceTime) > DEBOUNCE_DELAY) {
    
    if (reading != lastStableState) {
      lastStableState = reading;
      
      if (reading == LOW) {  // KEY DOWN
        
        // Se é a primeira vez que pressiona no modo manual, limpar display
        if (!manualModeActive) {
          clearDisplayArea();
          manualModeActive = true;
        }
        
        keyDownTime = timeTrack;
        keyLine = true;
        clearDisplayTriggered = false;  // Resetar flag quando pressiona
        digitalWrite(indicatorLED, HIGH);
        
        // Ligar áudio com frequência ajustável
        if (toneOutput && sidetoneEnabled) {
          tone(audioOutputPin, audioFrequency);
        }
        
        if (keyUpTime > 0) {
          spaceDuration = keyDownTime - keyUpTime;
          spaceDurationReference = keyDownTime;
        }
        
      } else {  // KEY UP
        
        keyUpTime = timeTrack;
        keyLine = false;
        digitalWrite(indicatorLED, LOW);
        
        // Desligar áudio
        noTone(audioOutputPin);
        
        pulseDuration = keyUpTime - keyDownTime;
        
        // Verificar se foi uma pressão longa (3 segundos)
        if (pulseDuration >= CLEAR_DISPLAY_HOLD_TIME) {
          clearDisplayArea();
          manualModeActive = false;
          clearDisplayTriggered = true;
        }
        // Se não foi pressão longa e não foi acionada a limpeza, processar pulso normal
        else if (!clearDisplayTriggered && pulseDuration >= MIN_PULSE_DURATION) {
          
          keyLinePriorEvent = keyLineNewEvent;
          keyLineNewEvent = pulseDuration;
          
          processPulse();
          
        }
      }
    }
  }
  
  lastButtonState = reading;
  
  // ===== VERIFICAR TIMEOUT PARA DECODIFICAÇÃO =====
  
  if (!keyLine && strlen(elementSequence) > 0) {
    unsigned long timeSinceLastKey = timeTrack - keyUpTime;
    
    if (timeSinceLastKey >= thresholdGeometricMean * 1.5) {
      
      morseDecode();
      
      if (decodeChar != 7) {
        printDecodedChar(decodeChar);
      } else {
        if (decodeProSign.length() > 0) {
          // Prosign - mostrar entre colchetes
          printDecodedChar('[');
          for (int i = 0; i < decodeProSign.length(); i++) {
            printDecodedChar(decodeProSign.charAt(i));
          }
          printDecodedChar(']');
        } else {
          printDecodedChar('?');
        }
      }
      
      // Limpar sequência e linha Morse
      decodeChar = 0;
      decodeProSign = "";
      elementSequence[0] = '\0';
      morseElements = "";
      
      // Limpar linha superior
      u8x8.setCursor(0, 0);
      u8x8.print("                ");
      
      characterStep = false;
    }
  }
  
  // ===== VERIFICAR ESPAÇO ENTRE PALAVRAS =====
  
  if (!keyLine && wordStep) {
    unsigned long timeSinceLastKey = timeTrack - keyUpTime;
    
    if (timeSinceLastKey >= thresholdGeometricMean * wordSpaceTiming) {
      printDecodedChar(' ');
      wordStep = false;
    }
  }
}

/// Processar pulso recebido ///////////////////////////////////

void processPulse() {
  
  if ((keyLineNewEvent >= keyLinePriorEvent * compareFactor) ||
      (keyLinePriorEvent >= keyLineNewEvent * compareFactor)) {
    
    if (keyLineNewEvent >= keyLinePriorEvent) {
      longEventHistogramList[eventHistogramTrack] = keyLineNewEvent;
      shortEventHistogramList[eventHistogramTrack] = keyLinePriorEvent;
    } else {
      longEventHistogramList[eventHistogramTrack] = keyLinePriorEvent;
      shortEventHistogramList[eventHistogramTrack] = keyLineNewEvent;
    }
    
    long longEventAverage = 0;
    for (int i = 0; i < 10; i++) longEventAverage += longEventHistogramList[i];
    longEventAverage /= 10;
    
    long shortEventAverage = 0;
    for (int i = 0; i < 10; i++) shortEventAverage += shortEventHistogramList[i];
    shortEventAverage /= 10;
    
    thresholdGeometricMean = sqrt(shortEventAverage * longEventAverage);
    
    eventHistogramTrack = (eventHistogramTrack + 1) % 10;
  }
  
  // Classificar como DIT ou DAH
  if (keyLineNewEvent <= thresholdGeometricMean) {
    strcat(elementSequence, ".");
    morseElements += ".";
  } else {
    strcat(elementSequence, "-");
    morseElements += "-";
  }
  
  characterStep = true;
  wordStep = true;
  
  // Mostrar símbolos Morse na linha superior
  u8x8.setCursor(0, 0);
  u8x8.print(morseElements);
}

/// Imprimir caractere decodificado no OLED ///////////////////////////////////

void printDecodedChar(int charASCII) {
  
  // Posicionar na área de texto
  u8x8.setCursor(cursorCol, cursorRow);
  u8x8.write(charASCII);
  
  cursorCol++;
  
  // Se chegou ao final da linha, pular para próxima
  if (cursorCol >= OLED_COLS) {
    cursorCol = 0;
    cursorRow += 2;  // Pular uma linha (espaçamento duplo)
    
    // Se chegou ao final da tela, fazer scroll
    if (cursorRow >= OLED_ROWS) {
      cursorRow = 2;  // Voltar para linha 2
      
      // Limpar área de texto
      for (int r = 2; r < OLED_ROWS; r++) {
        u8x8.setCursor(0, r);
        u8x8.print("                ");
      }
      cursorCol = 0;
    }
  }
}

/// Tabela de decodificação Morse ///////////////////////////////////

void morseDecode() {
  
  decodeChar = 7;
  decodeProSign = "";
  
  // Letras
  if (strcmp(elementSequence, ".-") == 0) decodeChar = 'A';
  else if (strcmp(elementSequence, "-...") == 0) decodeChar = 'B';
  else if (strcmp(elementSequence, "-.-.") == 0) decodeChar = 'C';
  else if (strcmp(elementSequence, "-..") == 0) decodeChar = 'D';
  else if (strcmp(elementSequence, ".") == 0) decodeChar = 'E';
  else if (strcmp(elementSequence, "..-.") == 0) decodeChar = 'F';
  else if (strcmp(elementSequence, "--.") == 0) decodeChar = 'G';
  else if (strcmp(elementSequence, "....") == 0) decodeChar = 'H';
  else if (strcmp(elementSequence, "..") == 0) decodeChar = 'I';
  else if (strcmp(elementSequence, ".---") == 0) decodeChar = 'J';
  else if (strcmp(elementSequence, "-.-") == 0) decodeChar = 'K';
  else if (strcmp(elementSequence, ".-..") == 0) decodeChar = 'L';
  else if (strcmp(elementSequence, "--") == 0) decodeChar = 'M';
  else if (strcmp(elementSequence, "-.") == 0) decodeChar = 'N';
  else if (strcmp(elementSequence, "---") == 0) decodeChar = 'O';
  else if (strcmp(elementSequence, ".--.") == 0) decodeChar = 'P';
  else if (strcmp(elementSequence, "--.-") == 0) decodeChar = 'Q';
  else if (strcmp(elementSequence, ".-.") == 0) decodeChar = 'R';
  else if (strcmp(elementSequence, "...") == 0) decodeChar = 'S';
  else if (strcmp(elementSequence, "-") == 0) decodeChar = 'T';
  else if (strcmp(elementSequence, "..-") == 0) decodeChar = 'U';
  else if (strcmp(elementSequence, "...-") == 0) decodeChar = 'V';
  else if (strcmp(elementSequence, ".--") == 0) decodeChar = 'W';
  else if (strcmp(elementSequence, "-..-") == 0) decodeChar = 'X';
  else if (strcmp(elementSequence, "-.--") == 0) decodeChar = 'Y';
  else if (strcmp(elementSequence, "--..") == 0) decodeChar = 'Z';
  
  // Números
  else if (strcmp(elementSequence, ".----") == 0) decodeChar = '1';
  else if (strcmp(elementSequence, "..---") == 0) decodeChar = '2';
  else if (strcmp(elementSequence, "...--") == 0) decodeChar = '3';
  else if (strcmp(elementSequence, "....-") == 0) decodeChar = '4';
  else if (strcmp(elementSequence, ".....") == 0) decodeChar = '5';
  else if (strcmp(elementSequence, "-....") == 0) decodeChar = '6';
  else if (strcmp(elementSequence, "--...") == 0) decodeChar = '7';
  else if (strcmp(elementSequence, "---..") == 0) decodeChar = '8';
  else if (strcmp(elementSequence, "----.") == 0) decodeChar = '9';
  else if (strcmp(elementSequence, "-----") == 0) decodeChar = '0';
  
  // Pontuação
  else if (strcmp(elementSequence, "--..--") == 0) decodeChar = ',';
  else if (strcmp(elementSequence, ".-.-.-") == 0) decodeChar = '.';
  else if (strcmp(elementSequence, "..--..") == 0) decodeChar = '?';
  else if (strcmp(elementSequence, "-....-") == 0) decodeChar = '-';
  else if (strcmp(elementSequence, "-..-.") == 0) decodeChar = '/';
}