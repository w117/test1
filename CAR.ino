#include <SoftwareSerial.h>
#include <nokia5110.h>
#include <MsTimer2.h>

#define IN1 12
#define IN2 13
#define IN3 8
#define IN4 11   //电机驱动脚
#define MS  A4  //模式选择
#define ENA 10
#define ENB 9  //PWM调速脚��

static const int pin5110_CLK = 4;
static const int pin5110_DAT = 5;
static const int pin5110_DC  = 6;
static const int pin5110_SCE = 7;
static const int pin5110_RST = 19;//5110连线

unsigned int motor_1 = 0; //计左电机码盘脉冲值
unsigned int motor_2 = 0; //计右电机码盘脉冲值
unsigned int motor_L = 0; //计左电机码盘总脉冲值
unsigned int motor_R = 0; //计右电机码盘总脉冲值
float speed_L = 0; //计左电机速度值cm/s
float speed_R = 0; //计右电机速度值
float T = 0;      //总计时
float Distance = 0;//计距离cm
float Speed = 0; //计电机平均速度值cm/s��
int Direction;
int j = 0;//状态标记

static const byte hanzi12x12[][24] =
{ /* Nokia5110显示汉字的设置方法：列行式+逆向 */
  {0x04, 0xCB, 0x12, 0xE6, 0xAA, 0xAC, 0xAB, 0xEA, 0x0E, 0xFA, 0x02, 0x00, 0x00, 0x0F, 0x00, 0x03, 0x02, 0x02, 0x02, 0x0B, 0x08, 0x0F, 0x00, 0x00}, /*"简",0*/
  {0x00, 0x9F, 0x75, 0x55, 0xD5, 0x55, 0x55, 0xD5, 0x55, 0x5F, 0xC0, 0x00, 0x01, 0x08, 0x04, 0x02, 0x09, 0x04, 0x02, 0x09, 0x08, 0x08, 0x07, 0x00}, /*"易",1*/
  {0x24, 0xF2, 0x09, 0x00, 0xFE, 0x0A, 0xEA, 0xAA, 0xBE, 0xA9, 0xE9, 0x00, 0x00, 0x0F, 0x00, 0x08, 0x07, 0x00, 0x0F, 0x0A, 0x0A, 0x0A, 0x0F, 0x00}, /*"循",2*/
  {0x11, 0xF2, 0x80, 0x64, 0x04, 0xFC, 0x05, 0x06, 0xFC, 0x24, 0xC4, 0x00, 0x08, 0x07, 0x08, 0x0A, 0x09, 0x08, 0x08, 0x0A, 0x0B, 0x08, 0x08, 0x00}, /*"迹",3*/
  {0x04, 0x64, 0x54, 0x4C, 0x47, 0xF4, 0x44, 0x44, 0x44, 0x44, 0x04, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0F, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00}, /*"车",4*/
};

//声明实例对象
static nokia5110 lcd(pin5110_CLK, pin5110_DAT, pin5110_DC, pin5110_RST, pin5110_SCE);

void flash() {
  speed_L = motor_1 * 22 / 20.0 * 2; //20齿22cm
  speed_R = motor_2 * 22 / 20.0 * 2;
  motor_1 = 0;  //重新定义motor1的值
  motor_2 = 0;  //重新定义motor1的值
  if (Distance > 0.5)
    T += 0.5;        //0.5s
}

void Motor(char Action, int speed1, int speed2)
{
  if (Action == 1)    //Front
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speed1);
    analogWrite(ENB, speed2);
  }
  if (Action == 2)    //Back
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speed1);
    analogWrite(ENB, speed2);
  }
  if (Action == 3)    //Left
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speed1);
    analogWrite(ENB, speed2);
  }
  if (Action == 4)   //Right
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speed1);
    analogWrite(ENB, speed2);
  }
  if (Action == 5)    //Stop
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
}

void setup() {
  Serial.begin(38400);
  for (int i = 4; i <= 13; i++) {
    pinMode(i, OUTPUT);
  }
  pinMode(19, OUTPUT);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(MS, INPUT_PULLUP);

  //初始化
  //第三个参数是对比度，有些屏幕全黑的要改成0xB5才行
  lcd.begin(84, 48, 200); //width, height,

  attachInterrupt(0, left_motor, RISING);
  attachInterrupt(1, right_motor, RISING);

  lcd.setCursor(0, 0); lcd.print("//**********//");
  lcd.setCursor(12, 1); for (int i = 0; i < 5; i++) lcd.drawBitmap(hanzi12x12[i], 12, 2);
  lcd.setCursor(21, 3); lcd.print("Arduino");
  lcd.setCursor(0, 5); lcd.print("//*14621122*//");
  delay(1000); lcd.clear();

  attachInterrupt(0, right_motor, RISING);
  attachInterrupt(1, left_motor, RISING);
  MsTimer2::set(500, flash); // 中断设置函数，每 250ms 进入一次中断
  MsTimer2::start();    //开始计时

}
void loop()
{
  int a = digitalRead(A0);
  int b = digitalRead(A1);
  int c = digitalRead(A2);
//  int d = digitalRead(A3);

  if (digitalRead(MS) == LOW)
  {
    Direction = Serial.read();
    switch (Direction) {
      case 'F': Motor(1, 100, 100); break;
      case 'B': Motor(2, 100, 100); break;
      case 'L': Motor(3, 100, 100); break;
      case 'R': Motor(4, 100, 100); break;
      case 'Z': Motor(5, 0, 0); break;
    }
    lcd.setCursor(46, 2); lcd.print(T);
    lcd.setCursor(0, 4); lcd.print(Speed);
  }
  else if (j == 0)
  {
    if (a == 0 && b == 1 && c == 0) //前进
      Motor(1, 50, 48);
   // else if (a == 0 && b == 0 && c == 0) //停止
   // Motor(5, 0, 0);
    else if (a == 1 && b == 1 && c == 1) //黑线停
    { Motor(5, 0, 0);
      j = 1;
    }
    else if (a == 0 && b == 0 && c == 1 ) //左转大
      Motor(1, 50, 17);
    else if (a == 0 && b == 1 && c == 1 ) //左转小
      Motor(1, 50, 35);
    else if (a == 1 && b == 0 && c == 0 ) //右转大
      Motor(1, 17, 48);
    else if (a == 1 && b == 1&& c == 0) //右转小
      Motor(1, 35, 48);
    lcd.setCursor(46, 2); lcd.print(T);
    lcd.setCursor(0, 4); lcd.print(Speed);
  }
  Distance = (motor_L + motor_R) * 22 / 20.0 / 2.0;
  Speed = Distance / T;
  lcd.setCursor(0, 0); lcd.print(speed_L);
  lcd.setCursor(42, 0); lcd.print(speed_R);
  lcd.setCursor(0, 2); lcd.print(Distance);
}

void left_motor() //触发函数
{ motor_1++;
  motor_L++;
}

void right_motor()  //触发函数
{ motor_2++;
  motor_R++;
}











