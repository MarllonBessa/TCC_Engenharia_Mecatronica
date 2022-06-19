#include <SoftwareSerial.h>          //Biblioteca da comunicação serial
#include "EmonLib.h"                //Biblioteca do sensor SCT-0013

#define Vrede 124                   // define a tensão RMS da rede(valor lido com multimetro)
#define InputSCT 0                 // define o canal analógico para o sinal do SCT = pino A0
#define I_calibration 60.606      // fator de ajuste da corrente - 2000/33 - (n/RL)

SoftwareSerial mySerial(2, 3);    // RX, TX da rede de comunicação industrial
EnergyMonitor EnerMonitor;       //Cria uma instancia da classe EnergyMonitor
double Irms = 0 ;               //Corrente IRMS
float Tempo = 0;


int pinWrRd = 4;               //Saída responsável por definir a função de leitura e escrita
bool writeOp = HIGH;          // Leitura como nível alto
bool readOp = LOW;           // Escrita como nível baixo

int pinLed = 10;

int myAdd = 0;              //Iniciando com um endereço qualquer

String buf = "";
int stRec = 0;
unsigned long lastRec=0;
unsigned long tOcioso=0;

void setup()
{
  pinMode(pinLed, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(9,LOW);

  pinMode(pinWrRd, OUTPUT);
  digitalWrite(pinWrRd, readOp );
    
  mySerial.begin( 9600 );
  Serial.begin( 9600 );
  pinMode(12 , INPUT_PULLUP );      //Pinos de endereçamento dos escravos
  pinMode(11 , INPUT_PULLUP );      //Pinos de endereçamento dos escravos
  
  EnerMonitor.current(InputSCT, I_calibration);   // configura pino SCT e fator de calibração

  int valIn1 = digitalRead( 12 );
  int valIn2 = digitalRead( 11 );
  myAdd = valIn1*2 + valIn2 ; //Calculando o endereço dos escravos

  Serial.print("ADD " );
  Serial.println( myAdd );
  
}


// Guardando o pacote
String pacote = "";

byte ordChar( char carac ) {
  byte numOrdChar = 0;
  if ( (carac >= 'A') && (carac <= 'F') ) {
    numOrdChar = carac - 65 + 10;
  } else
  if ( (carac >= 'a') && (carac <= 'f') ) {
    numOrdChar = carac - 97 + 10;
  } else
  if ( (carac >= '0') && (carac <= '9') ) { 
    numOrdChar = carac-48;    
  }
  return( numOrdChar );
}

String converteHex( String hexPac ) {
  int tam = hexPac.length();
  String convertida = "";
  for (int i=0; i < tam; i=i+2 ) {
    char cHi  = hexPac.charAt( i );
    char cLow = hexPac.charAt( i+1 );
    char cc = ordChar(cHi) * 16 + ordChar(cLow);
    convertida = convertida + (char) cc;
  }
  return convertida;
}


int confereCS(){
  String mensagem=converteHex(pacote);
  int tam = mensagem.length();
  Serial.println(" cs processing ");
  Serial.println( mensagem );
  
  byte CS = 0;
  for (int i=0; i < (tam-1); i++ ) {
    char cOne = mensagem.charAt(i) ;
    CS = CS xor cOne;
    //Serial.print( cOne , HEX );
    //Serial.println( CS, HEX );
  }
  byte originalCS = mensagem.charAt(tam-1);
  int conferiu = originalCS == CS;
  return conferiu;
}


String datagrama( byte endereco, byte comando, byte io, byte valor ) {
  String pack = "" ;
  pack = pack + endereco;
  pack = pack + comando ; 
  pack = pack + io; 
  pack = pack + valor ;
  return pack;
}

String toHex2( byte aConv ) {
  byte bHi = aConv / 16;
  byte bLow = aConv & 0x0F ;
  String sHi = "";
  if (bHi <= 9) {
    sHi = sHi + bHi ;
  } else
  {
    sHi = sHi + (55 + bHi);
  }
  String sLow = "";
  if (bLow <= 9) {
    sLow = sLow + bLow ;
  } else
  {
    sLow = sLow + (55 + bLow);
  }
  return sHi + sLow;  
}


void transmiteComCS( String mensagem ) {
  byte CS = 0;
  for (int i=0; i < mensagem.length(); i++ ) {
    char cOne = mensagem.charAt(i) ;
    CS = CS xor cOne;
    Serial.print( cOne , HEX );
    delay(1);
    mySerial.print( cOne , HEX );
    delay(1);
  }

  String toTx = toHex2( CS );
  Serial.print( toTx );
  mySerial.print( toTx );
  delay(1);
}

void runRead() {

  Irms = EnerMonitor.calcIrms(1480);         // calculo da corrente RMS
  Irms = 1.0002*Irms - 0.2856;              //Função de calibração do sensor SCT013 com base nos dados obtidos em bancada.
  int leitura=Irms*100;
  int tensao = Vrede;
  
  digitalWrite(pinWrRd, writeOp );        //Habilita a escrita
  delay(1);
  
  mySerial.print("000");   
  mySerial.print(myAdd);

  if (tensao < 1000) {                  //Transferindo o pacote sempre com 4Bytes de tamanho
    mySerial.print("0");
  }
  if (tensao < 100) {
    mySerial.print("0");
  }
  if (tensao< 10) {
    mySerial.print("0");
  }
  mySerial.print(tensao);

  if (leitura < 1000) {         //Transferindo o pacote sempre com 4Bytes de tamanho
    mySerial.print("0");
  }
  if (leitura < 100) {
    mySerial.print("0");
  }
  if (leitura < 10) {
    mySerial.print("0");
  }
  mySerial.print(leitura ); //IRMS AFERIDO VERSUS 100 (ASSEGURANDO MAIS CONFIABILIDADE)
  
  delay(1);
 
  digitalWrite(pinWrRd, readOp );
  

}

void loop()
{

  
  while (mySerial.available() > 0) { //Aguardando a comunicação na rede industrial
    char cIn = mySerial.read();
    delay(1);
    Serial.write( cIn );
    delay(1);
    lastRec = millis();
    stRec = 1;
    pacote = pacote + cIn;
  }
  
  if (stRec==1) {
    tOcioso = millis()-lastRec;
    if (tOcioso > 15) {
      stRec=0;
      if (confereCS()) {         //Conferindo a informação recebida;
        Serial.println("!"); 

        Serial.print(">");
        Serial.println(pacote);
        int endereco = ordChar(pacote.charAt(0))*16 + ordChar(pacote.charAt(1)) - 48;
        int comando = ordChar(pacote.charAt(2))*16 + ordChar(pacote.charAt(3)) - 48;
        int io = ordChar(pacote.charAt(4))*16 + ordChar(pacote.charAt(5)) - 48;
        int valor = ordChar(pacote.charAt(6))*16 + ordChar(pacote.charAt(7)) - 48;
        
        Serial.print(">> ");   //Executando os comandos solicitados
        Serial.print( endereco ); Serial.print(" ");
        Serial.print( comando ); Serial.print(" ");
        Serial.print( io ); Serial.print(" ");
        Serial.print( valor ); Serial.print(" ");
        Serial.println();
 
        if (endereco == myAdd) {
          Serial.println("SOU EU!!");
          if (comando==1) {
            digitalWrite(pinLed , valor ); 
          }
          if (comando==0) {
             runRead();           
          }
        }
      
      } else
      { Serial.println("?");
      }
      pacote = "";
      
    }
  }
}
