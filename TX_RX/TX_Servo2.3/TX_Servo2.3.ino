/*   Данный скетч делает следующее: передатчик (TX) отправляет массив
 *   данных, который генерируется согласно показаниям с джойтиков и кнопок. Приёмник (RX) получает массив, и записывает
 *   на управляющие устройства.
*/

#include <SPI.h>          // библиотека для работы с шиной SPI
#include "nRF24L01.h"     // библиотека радиомодуля
#include "RF24.h"         // ещё библиотека радиомодуля
#define ypin_1 A0         // ось Х джойстика1
#define xpin_1 A1         // ось Y джойстика1
#define ypin_2 A2         // ось Х джойстика2
#define xpin_2 A3         // ось Y джойстика2
#define joy_but_1 4       //кнопка джойстика1
#define joy_but_2 2       //кнопка джойстика2

RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

byte transmit_data[8]; // массив, хранящий передаваемые данные, [n] занчит от [0] до [n-1]
byte latest_data[8]; // для режима энергосбережения
boolean flag_joy_but_1=0, oldflag_joy_but_1=1, flag_joy_but_2=0, oldflag_joy_but_2=1; //flag_button;  //флаги для работы с кнопками
boolean flag;
byte id = 123; // идентификатор
long previousMillis_1 = 0, previousMillis_2 = 0; //предыдущие значения времени для защиты от дребезга

void setup() {
  Serial.begin(9600); //открываем порт для связи с ПК
  pinMode(xpin_1, INPUT);
  pinMode(ypin_1, INPUT);
  pinMode(xpin_2, INPUT);
  pinMode(ypin_2, INPUT);
  pinMode(joy_but_1, INPUT_PULLUP);
  pinMode(joy_but_2, INPUT_PULLUP);
  //pinMode(button, INPUT_PULLUP);*/

  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); //скоростсь обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); //начать работу
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик
}

void loop() {
  transmit_data[0] = id;
  transmit_data[1] = map(analogRead(xpin_1), 0, 1023, 0, 180); //значение оси Х джойстика1 преобразованные от 0 до 180 для серво
  transmit_data[2] = map(analogRead(ypin_1), 0, 1023, 0, 180); //значение оси Y джойстика1 преобразованные от 0 до 180 для серво
  transmit_data[3] = map(analogRead(xpin_2), 0, 1023, 0, 180); //значение оси Х джойстика2 преобразованные от 0 до 180 для серво
  transmit_data[4] = map(analogRead(ypin_2), 0, 1023, 0, 180); //значение оси Y джойстика2 преобразованные от 0 до 180 для серво
  
  //кнопка на джойстике1
  
  if (digitalRead(joy_but_1) == HIGH ) {
    if (millis() - previousMillis_1 > 100) { //защита от дребезга, чтобы с первого раза заходила, а потом только по 100мс
      previousMillis_1 = millis();
      if (flag_joy_but_1 == oldflag_joy_but_1) {
        flag_joy_but_1 = !flag_joy_but_1;
      }
    }
  } else {
    oldflag_joy_but_1 = flag_joy_but_1;
  }
  transmit_data[5]=flag_joy_but_1;

  //кнопка на джойстике2
  
  if (digitalRead(joy_but_2) == HIGH ) {
    if (millis() - previousMillis_2 > 100) { //защита от дребезга, чтобы с первого раза заходила, а потом только по 100мс
      previousMillis_2 = millis();
      if (flag_joy_but_2 == oldflag_joy_but_2) {
        flag_joy_but_2 = !flag_joy_but_2;
      }
    }
  } else {
    oldflag_joy_but_2 = flag_joy_but_2;
  }
  transmit_data[6]=flag_joy_but_2;
  
// доп. кнопка
  transmit_data[7] = 0;

  for (int i = 0; i < 7; i++) { // в цикле от 0 до числа каналов
    if (transmit_data[i] != latest_data[i]) { // если есть изменения в transmit_data
      flag = 1; // поднять флаг отправки по радио
      latest_data[i] = transmit_data[i]; // запомнить последнее изменение
    }
  }
  Serial.print("Sent: "); Serial.print(transmit_data[0]);Serial.print("\t");Serial.print(transmit_data[1]);Serial.print("\t");Serial.print(transmit_data[2]);Serial.print("\t");Serial.print(transmit_data[3]);Serial.print("\t");Serial.print(transmit_data[4]);Serial.print("\t");Serial.print(transmit_data[5]);Serial.print("\t");Serial.print(transmit_data[6]);Serial.print("\t");Serial.println(transmit_data[7]);
  
  if (flag == 1) {
    radio.powerUp(); // включить передатчик
    radio.write(&transmit_data, sizeof(transmit_data)); // отправить по радио
    flag = 0; //опустить флаг
    radio.powerDown(); // выключить передатчик
  }
  
  delay(10);
}
