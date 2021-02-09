#include <Ethernet.h>
#include <SPI.h>

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xF6, 0xFF };
byte ip[] = { 10, 189, 113, 200 };

char server[] = "10.189.113.254";

int count = 0;

EthernetClient client;


void setup() {
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  Serial.println(Ethernet.localIP());
  delay(1000);
  delay(1000);
  Serial.println("connecting...");


}

void loop() {
  // здесь считывание с шины событий rfid (id устройства и непосредственно rfid метка)
  
  // отправка события на сервер
  send_event(((++count) & 255), "MSG");

  String response;

  delay(25);
  // вычитывание кода ответа (1 строка ответа содержит http код ответа)
  if (client.available()) {
    response = client.readStringUntil('\r');
  }

  // если 200, вычитываем ответ до конца
  // последняя строка ответа в положительносм сценарии - это id шлагбаума который нужно открыть
  if (response.charAt(9) == '2') {
      while(client.available()) {
          response = client.readStringUntil('\r');
      }

      // send signal to шлагбаум
      Serial.println(response);
  }
  else {
    Serial.println("not 200");
    Serial.println(response);
  }

 // если не 200, продолжаем слушать шину
 delay(1000);
}

/** 
 * функция для посылки POST запроса на сервер для сверки метки 
 * 
 */
void send_event(int id, char* rfid) {
  char body[30];
  sprintf(body, "\{\"device_id\":%d,\"rfid\":\"%s\"\}", id, rfid);

  char content_len[17];
  sprintf(content_len, "Content-Length:%d", strlen(body));

  if (client.connect(server, 8080)) {
    Serial.println("Sending to Server: ");
    client.println("POST /v1/events HTTP/1.1");
    client.println("Host: 10.189.113.254:8080");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println("User-Agent: Arduino/1.0");
    client.println(content_len);
    client.println();
    client.print(body);
  }

  else {
    Serial.println("Cannot connect to Server");
  }
}
