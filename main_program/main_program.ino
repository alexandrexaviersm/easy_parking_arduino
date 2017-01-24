//https://github.com/bblanchon/ArduinoJson/wiki/Decoding%20JSON
//https://github.com/bblanchon/ArduinoJson/wiki/API-Reference
//

#include <ArduinoJson.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h> //LCD
#include <LiquidCrystal_I2C.h> //LCD

// Inicializa o display no endereco 0x3F
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFA, 0xDE}; //Define o Mac para EthernetShield
IPAddress ip(192, 168, 0, 10); //Ip Fixo, reservado no Pool do DHCP do roteador

EthernetServer server(8080); //Porta do Webserver

//Declação de porta das vagas (Arduino Mega)
int vaga1 = 31;
int vaga2 = 33;
int vaga3 = 35;
int vaga4 = 37;

int ledvaga1 = 30;
int ledvaga2 = 32;
int ledvaga3 = 34;
int ledvaga4 = 36;

//Declação do LED das vagas

//Vetor com o texto das vagas
int vagas[6] = {0, vaga1, vaga2, vaga3, vaga4};
int ledvagas[6] = {0, ledvaga1, ledvaga2, ledvaga3, ledvaga4};
char* texto[] = {"0", "vaga1", "vaga2", "vaga3", "vaga4"};

int vagaslivres_setorA;
int vagaslivres_setorB;

StaticJsonBuffer<200> jsonBuffer;
JsonObject& vagajson = jsonBuffer.createObject();

void setup() {

  Ethernet.begin(mac, ip); //Inicializa o MAC e IP
  server.begin();
  Serial.begin(9600); //inicia a comunicação serial com baud rate 9600
  //Serial.print("server is at ");
  // start the Ethernet connection and the server:
  //Serial.println(Ethernet.localIP());

  pinMode(vaga1, INPUT);
  pinMode(vaga2, INPUT);
  pinMode(vaga3, INPUT);
  pinMode(vaga4, INPUT);

  pinMode(ledvaga1, OUTPUT);
  pinMode(ledvaga2, OUTPUT);
  pinMode(ledvaga3, OUTPUT);
  pinMode(ledvaga4, OUTPUT);

  pinMode(39,INPUT);

  lcd.begin (16,2);

}


void loop() {

//o loop se encarrega de zerar todas as vagas, então o loop irá contar quantas livres
  vagaslivres_setorA=0;
  vagaslivres_setorB=0;

//for para contar vagas livres
  for (int i = 1; i <= 4; i++) {
    if (digitalRead(vagas[i]) == LOW)  {
      // Estado da vaga (Ocupada)
      vagajson[texto[i]] = false;
      digitalWrite(ledvagas[i],LOW);
    }
    else  {
      // Estado da vaga (Livre)
      vagajson[texto[i]] = true;
      digitalWrite(ledvagas[i],HIGH);
      if((i==1)||(i==2)) //Checa a qual vaga pertence o sensor
        vagaslivres_setorA++;
      else
        vagaslivres_setorB++;
    }//fim se
    delay(50);
  }//fim for
  //Serial.println();



  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("<--- Setor A:");
  lcd.print(vagaslivres_setorA);
  lcd.setCursor(0,1);
  lcd.print("---> Setor B:");
  lcd.print(vagaslivres_setorB);
  lcd.print("");

  // Aguarda a conexão do cliente
  EthernetClient client = server.available();
  if (client) {
//    Serial.println("new client");
    // Uma requisição HTTP termina com linha em branco
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
//        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Access-Control-Allow-Origin: *"); //Permitir conexão de outros locais
          client.println("Content-Type: application/json");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 10");  //REFRESH
          client.println();

          vagajson.printTo(client); //Escreve o retorno JSON

          break;
        }//fim se c == '\n'

        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }//fim c == '\n'
      }//fim se client.available
    }//fim while
    // give the web browser time to receive the data
    delay(500);
    // close the connection:
    client.stop();
//    Serial.println("client disconnected");
  }//fim se (client)
  delay(50);
}//fim loop
