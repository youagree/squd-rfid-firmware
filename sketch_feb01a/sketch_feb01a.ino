#include <Ethernet.h>
#include <SPI.h>

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xF6, 0xFF };
byte ip[] = { 10, 189, 113, 200 };

char server[] = "10.189.113.254";

EthernetClient client;


void setup() {
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  delay(1000);
} 


unsigned long byte_array_to_decimal(uint8_t* arr) {
    unsigned long result = 0;

    // от старшего к младшему: a[0] - старший, a[N] - младший
    //  uint8_t a[] = {104, 253, 254};
  
    //   {01101000, 11111111, 11111111}
    //a0  01101000  00000000  00000000 
    //a1  00000000  11111101  00000000
    //a2  00000000  00000000  11111110
  
    //res 01101000  11111101  11111110
  
    //Serial.println(byte_array_to_decimal(a));

    // 1
    // 00000000 00000000 00000000 00000000
                               // 01101000
                      // 01101000 00000000

    // 2
    // 00000000 00000000 01101000 00000000
    //                            11111111
    // 00000000 01101000 11111111 00000000

    for (int i = 0; i < 2; i++) {
      result = result | arr[i];
      result = result << 8;
    }

    result = result | arr[2];

    return result;
}


void loop() {
  // здесь считывание с шины событий rfid (id устройства и непосредственно rfid метка)

  uint8_t address = 0;
  uint8_t rfid[3];

  if (Serial.available() > 0) {
    char head = Serial.read();

    if (head == ':') {
      delay(10);
      int addr_tmp = Serial.read();
      address = (uint8_t) addr_tmp;
      for (int i = 0; i < 3; i++) {
        delay(10);
        int byte_tmp = Serial.read();
        rfid[i] = (uint8_t) byte_tmp;
      }

      // отправка события на сервер
      send_event(address, byte_array_to_decimal(rfid));

      String response;
    
      delay(500);
      // вычитывание кода ответа (1 строка ответа содержит http код ответа)
      if (client.available()) {
        response = client.readStringUntil('\r');
      }
    
      // если 200, вычитываем ответ до конца
      // последняя строка ответа в положительносм сценарии - это id шлагбаума который нужно открыть
      if (response.charAt(9) == '2') {
          while(client.available()) {
            delay(5);
            response = client.readStringUntil('\r');
          }
    
          // send signal to шлагбаум
          Serial.print(response);
      }
    }
  }
}

/** 
 * функция для посылки POST запроса на сервер для сверки метки 
 * 
 */
void send_event(uint8_t id, unsigned long rfid) {
  char body[38];
  sprintf(body, "\{\"device_id\":%d,\"rfid\":%lu\}", id, rfid);

  char content_len[17];
  sprintf(content_len, "Content-Length:%d", strlen(body));

  if (client.connect(server, 8080)) {
    client.println("POST /v1/event HTTP/1.1");
    client.println("Host: 10.189.113.254:8080");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println("User-Agent: Arduino/1.0");
    client.println(content_len);
    client.println();
    client.print(body);
  }

  else {
    // todo: придумать световую индикацию, что нет коннекта к серверу
  }
}
