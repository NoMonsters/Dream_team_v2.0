/*   Данный скетч делает следующее: передатчик (TX) отправляет массив
 *   данных, который генерируется согласно показаниям с кнопки и с 
 *   двух потенциомтеров. Приёмник (RX) получает массив, и производит
 *   заданные действия.
*/

#include <SPI.h>              // библиотека для работы с шиной SPI
#include "nRF24L01.h"         // библиотека радиомодуля
#include "RF24.h"             // ещё библиотека радиомодуля
#include <Servo.h>

RF24 radio(9,10); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  //возможные номера труб
byte recieved_data[8]; //массив принимаемых данных [n] значит от [0] до [n-1]
byte servo1 = 6; // сервопривод на 6 цифровом
byte servo2 = 5; // сервопривод на 5 цифровом
byte id = 123; //идентификатор

Servo myservo1; 
Servo myservo2;

void setup(){
  Serial.begin(9600); //открываем порт для связи с ПК
  
  myservo1.attach(servo1);
  myservo2.attach(servo2);
  
  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0,15);     //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openReadingPipe(1,address[0]);      //хотим слушать трубу 0
  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  
  radio.powerUp(); //начать работу
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
}

void loop() {
   byte pipeNo;                          
    while( radio.available(&pipeNo)){    // слушаем эфир со всех труб
      radio.read( &recieved_data, sizeof(recieved_data) );         // чиатем входящий сигнал
      Serial.print("Recieved: "); 
      Serial.print(recieved_data[1]); 
      Serial.print("\t"); 
      Serial.println(recieved_data[3]); 

      // при питаниии через USB нехвататет мощности для вывода всех значений и управления серво
/*      int i;
      Serial.print("Recieved: "); 
      for (i = 0; i < 7; i = i + 1) {
        Serial.print(recieved_data[i]);
        Serial.print ("\t");
      }
      Serial.println(" "); */
      
      }
      if (recieved_data[0] == id) { //идентификация
        myservo1.write(recieved_data[1]); //сигнал на серво
        myservo2.write(recieved_data[3]); } //сигнал на серво
        //кнопки
}


