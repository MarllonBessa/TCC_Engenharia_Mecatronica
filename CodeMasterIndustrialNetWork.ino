#include <SoftwareSerial.h>

SoftwareSerial mySerial(16, 17); // RX, TX

int pinWrRd = 4;
int pinLed = 13;

bool writeOp = HIGH;
bool readOp = LOW;

void pisca() {
  digitalWrite(pinLed, HIGH);
  delay(250); // Wait for 1000 millisecond(s)
  digitalWrite(pinLed, LOW);
  delay(250); // Wait for 1000 millisecond(s)
}

void setup()
{
  pinMode(pinLed, OUTPUT);
  pinMode(pinWrRd, OUTPUT);
  digitalWrite(pinWrRd, writeOp );
  Serial.begin(9600);
  mySerial.begin(9600);  // UART 
  //lcd.begin(16,2);
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
    //Serial.print( cOne , HEX );
    //delay(1);
    mySerial.print( cOne , HEX );
    delay(1);
  }

  String toTx = toHex2( CS );
  //Serial.print( toTx );
  mySerial.print( toTx );
  delay(1);
}

int aguardaRead() {
    digitalWrite(pinWrRd, readOp );
    delay(1);
    unsigned long horaEnvio = millis();
    unsigned long passou = 0;
    String chegando = "";
     while (passou < 500) {
      passou = millis() - horaEnvio;
      if (mySerial.available() > 0) {
        char cc = mySerial.read();
        chegando = chegando + cc;
      }
    }
    Serial.println(chegando);
    // volta ao modo escrita
    digitalWrite(pinWrRd, writeOp );
    delay(1);
    while (mySerial.available()>0) {
      char cc = mySerial.read();
    }
}

String pacote = "";

void leituraSlave1() {
  // comando de leitura
  pacote = datagrama( 1 , 0 , 1 , 1 );
  transmiteComCS( pacote );     
  Serial.println("IS");
  // aguarda 20 ms a resposta
  aguardaRead();
}

void leituraSlave2() {
  // comando de leitura
  pacote = datagrama( 2 , 0 , 1 , 1 );
  transmiteComCS( pacote );     
  Serial.println("IS");
  // aguarda 20 ms a resposta
  aguardaRead();
}

void loop()
{
  // Escrita, no endero 1 (slave1), 
  pacote = datagrama( 1 , 1 , 1 , 1 );
  transmiteComCS( pacote );   
  //Serial.println("+");
  pisca();

  leituraSlave1();
  
  Serial.println("F");
  pacote = datagrama( 1 , 1 , 1 , 0 );
  transmiteComCS( pacote );   
  //Serial.println("-");  
  pisca();

  // Escrita, no endero 2 (slave2), 
  pacote = datagrama( 2 , 1 , 1 , 1 );
  transmiteComCS( pacote );    
  //Serial.println(".");
  pisca();

  leituraSlave2();

  Serial.println("F");
  pacote = datagrama( 2 , 1 , 1 , 0 );
  transmiteComCS( pacote );    
  //Serial.println(".");
  pisca(); 

}
