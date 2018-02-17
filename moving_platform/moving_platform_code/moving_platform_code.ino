#define motor_RIGHT  9//ШИМ сигнал для скорости
#define motor_LEFT  3//ШИМ сигнал для скорости
#define motor_pin_DIRA1  4//одинаковые значения вперед
#define motor_pin_DIRA2  5//разные назад
#define motor_pin_DIRB1  6
#define motor_pin_DIRB2  7

#define ENCODER_LEFT 10
#define ENCODER_RIGHT 11

#define LEFT_MOTOR 1
#define RIGHT_MOTOR 0

#define MAX 255
#define MIN 0

//-----------------------------------------------------------------------

unsigned long time_encoder_left=micros(), time_encoder_right=micros(), time_data=micros();
//float P = 0, I = 0;  //значения П и И составвляющей с предыдущей итерации
float TEST_L = 0, TEST_R = 0;

//-----------------------------------------------------------------------
void PI_controller_L (float pre_set_val, float current_val, float Kp, float Ki, float &output)
{
  float error;  // ошибка между текцщим и требуемым значением
  static float P;
  static float I;
    
  error = pre_set_val-current_val;
  if(error<10)
  {
    Ki=0.000001;
  }
  P = Kp*error;
  I = I+Ki*error; //девый двигетаель 

  //ограничение И составляющей
  if (I>MAX)
    I=MAX;
  if (I<MIN)
    I=MIN;

  TEST_L=I;
  output=P+I;//вычитсление значения напряжения на мотор
  //ограничение напряжения
  if (output>MAX) 
    output=MAX;
  if(output<MIN)
    output=MIN;
}
void PI_controller_R(float pre_set_val, float current_val, float Kp, float Ki, float &output)
{
  float error;  // ошибка между текцщим и требуемым значением
  static float P;
  static float I;
  
  error = pre_set_val-current_val;
  P = Kp*error;
  I = I+Ki*error;
  if(error<10)
  {
    Ki=0.000001;
  }

  //ограничение И составляющей
  if (I>MAX)
    I=MAX;
  if (I<MIN)
    I=MIN;

  TEST_R=I;
  output=P+I;//вычитсление значения напряжения на мотор
  //ограничение напряжения
  if (output>MAX) 
    output=MAX;
  if(output<MIN)
    output=MIN;
}
inline void get_speed_left(float &current_speed, boolean &may_i_get_speed)
{
  static boolean state_encoder = digitalRead(ENCODER_LEFT);
  static int counter_encoder = 0;
  may_i_get_speed = 0;
  
  if(digitalRead(ENCODER_LEFT) != state_encoder) 
  {
    ++counter_encoder;
    state_encoder =! state_encoder;
  }
  if (counter_encoder >= 20)
  {
    current_speed = 60*1000000/(micros()-time_encoder_left);

    time_encoder_left=micros();
    counter_encoder=0;
    may_i_get_speed = 1;
  }  
  if (micros()-time_encoder_left >= 1000000)
  {
    current_speed = 0;
//    I=0;
//    U_PI=110;
  } 
}

inline void get_speed_right(float &current_speed, boolean &may_i_get_speed)
{
  static boolean state_encoder = digitalRead(ENCODER_RIGHT);
  static int counter_encoder = 0;
  may_i_get_speed = 0;
  
  if(digitalRead(ENCODER_RIGHT) != state_encoder) 
  {
    ++counter_encoder;
    state_encoder =! state_encoder;
  }
  if (counter_encoder >= 20)
  {
    current_speed = 60*1000000/(micros()-time_encoder_right);

    time_encoder_right=micros();
    counter_encoder=0;
    may_i_get_speed = 1;
  }  
  if (micros()-time_encoder_right >= 1000000)
  {
    current_speed = 0;
//    I=0;
//    U_PI=110;
  } 
}

void data_send(float current_speed_left, float current_speed_right, float U_PI_left_motor)
{
    Serial.print(current_speed_left);
    Serial.print(F(" "));
    Serial.print(current_speed_right);
    Serial.print(F(" "));  
    Serial.print(U_PI_left_motor);
    /*Serial.print(F(" "));  
    Serial.print(TEST_L);
    Serial.print(F(" "));  
    Serial.print(TEST_R);*/
    Serial.print(F("\n")); 
}

/*void forward() 
{
   digitalWrite (motor_pin_DIRB1, HIGH);
   digitalWrite (motor_pin_DIRB2, LOW);
   digitalWrite (motor_pin_DIRA1, HIGH);
   digitalWrite (motor_pin_DIRA2, LOW);
   analogWrite(motor_LEFT,150);
   analogWrite(motor_RIGHT,150);
}*/

void setup() 
{
   pinMode(ENCODER_LEFT,INPUT);
   pinMode(ENCODER_RIGHT,INPUT);
   
   pinMode (motor_pin_DIRA1, OUTPUT);
   pinMode (motor_pin_DIRB1, OUTPUT);
   pinMode (motor_pin_DIRA2, OUTPUT);
   pinMode (motor_pin_DIRB2, OUTPUT);
   
   pinMode (motor_LEFT, OUTPUT);
   pinMode (motor_RIGHT, OUTPUT);
   Serial.begin(9600);

   digitalWrite (motor_pin_DIRB1, HIGH);
   digitalWrite (motor_pin_DIRB2, LOW);
   digitalWrite (motor_pin_DIRA1, HIGH);
   digitalWrite (motor_pin_DIRA2, LOW);
}

void loop() 
{
  float Kp_L=0.097, Ki_L=0.000032, Kp_R=0.097, Ki_R=0.000032;
  float current_speed_left = 0;
  float current_speed_right = 0;
  
  float U_PI_left_motor = 0;
  float U_PI_right_motor = 0;
  
  float set_speed_left = 400;
  float set_speed_right = 400;
  
  boolean may_i_get_speed_left = 0;
  boolean may_i_get_speed_right = 0;

  //analogWrite(motor_LEFT, (unsigned int)U_PI_left_motor);
  //analogWrite(motor_RIGHT, (unsigned int)U_PI_right_motor);

  while(1)
  {
    get_speed_left(current_speed_left, may_i_get_speed_left);
    get_speed_right(current_speed_right, may_i_get_speed_right);

    PI_controller_L(set_speed_left, current_speed_left, Kp_L, Ki_L, U_PI_left_motor);
    analogWrite(motor_LEFT, (unsigned int)U_PI_left_motor);
    PI_controller_R(set_speed_right, current_speed_right, Kp_R, Ki_R, U_PI_right_motor);    
    analogWrite(motor_RIGHT, (unsigned int)U_PI_right_motor);

    if(micros()-time_data >=80000)
    {
      data_send(current_speed_left, current_speed_right, U_PI_left_motor);
      time_data = micros();
    }
  }
  
}
