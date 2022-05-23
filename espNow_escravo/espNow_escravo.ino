/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
//MASTER//

#include <esp_now.h>
#include <WiFi.h>
#include <ETH.h>
// Pino do sinal de habilitação para o oscilador de cristal externo (-1 para desabilitar para fonte APLL interna)
#define ETH_PHY_POWER 17
// Tipo de Ethernet PHY
#define ETH_TYPE ETH_PHY_LAN8720
// Endereço I2C de Ethernet PHY (0 ou 1 para LAN8720)
#define ETH_ADDR 1
#define ETH_PHY_ADDR 1
// Pino do sinal de relógio I2C para Ethernet PHY
#define ETH_MDC_PIN 23
// Pino do sinal I2C IO para Ethernet PHY
#define ETH_MDIO_PIN 18
// Clock
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN

static bool eth_connected = false;


const char *ssid = "esp32_cliente";
const char *password = "123456789";


WiFiServer server(80);

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void testClient(const char * host, uint16_t port)
{
  Serial.print("\nconnecting to ");
  Serial.println(host);

  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available());
  while (client.available()) {
    Serial.write(client.read());
    
  }

  Serial.println("closing connection\n");
  client.stop();
}




// Exemplo de estrutura para receber dados
// Deve corresponder à estrutura do remetente
typedef struct struct_message {
    char a[120];
    int b;
    float c;
    bool d;
} struct_message;

// Cria uma struct_message chamada myData
struct_message myData;
// função de retorno de chamada que será executada quando os dados forem recebidos
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(myData.a);
  Serial.print("Int: ");
  Serial.println(myData.b);
  Serial.print("Float: ");
  Serial.println(myData.c);
  Serial.print("Bool: ");
  Serial.println(myData.d);
  Serial.println();
}
 





IPAddress ip(192,168,4,8);
IPAddress gw(192,168,4,1);
IPAddress masc(255,255,255,0);
IPAddress dns(8,8,8,8);
IPAddress dns2(8,8,4,4);
 
void setup() { 
  //Inicia o Monitor serial
  Serial.begin(115200);
   // Definir dispositivo como uma estação Wi-Fi
  WiFi.mode(WIFI_STA);

  // Inicializa o  ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar o ESP-NOW");
    return;
  }
  
 // Assim que o ESPNow for iniciado com sucesso, nos registraremos para recv CB para
 // obtém informações do empacotador de recebimento
  esp_now_register_recv_cb(OnDataRecv);
  
 
  WiFi.onEvent(WiFiEvent);
  ETH.begin(PHY1, 17, 23,18,ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
  ETH.config(ip, gw, masc, dns, dns2);
  Serial.print("IP_ETH: ");
  Serial.println(ETH.localIP());


  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");

}
 
void loop() {
   if (eth_connected) {
    Serial.println(ETH.localIP());
   //testClient("google.com", 80);
    
   }
  
   WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client."); 
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.println("<h>----Cliente------</h><br>");
            client.print("Click <a href=\"/H\">here</a> to turn ON the LED.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn OFF the LED.<br>");
            client.print("<p>");
            client.print("Char: ");
            client.print(myData.a);
            client.print("</p>");
            client.print("<p>");
            client.print("b: ");
            client.print(myData.b);
            client.print("</p>");
            client.print("<p>");
            client.print("C: ");
            client.print(myData.c);
            client.print("</p>");
            client.print("<p>");
            client.print("d: ");
            client.println(myData.d);
            client.print("</p>");
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          //strcpy(myData.a, "Ligar");
        }
        if (currentLine.endsWith("GET /L")) {
           //strcpy(myData.a, "Desligar");
        }
      }
      
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  } 

//  // Define os valores para enviar
//  //strcpy(myData.a, "ISTO É UM ARRAY DE CHAR");
//  myData.b = random(1,20);
//  myData.c = 1.2;
//  myData.d = false;
  

  delay(2000);
}
