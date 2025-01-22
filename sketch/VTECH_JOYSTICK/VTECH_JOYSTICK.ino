#include <Joystick.h>

#define CTS_PIN 2
#define RTS_PIN 3
#define JOYTSTICK_RTS (digitalRead(RTS_PIN)==LOW)
#define JOY_COM  Serial1
#define JOY_POLL 17000 // secs

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   8, 0,                  // Button Count, Hat Switch Count
                   true, true, false,     // X and Y, but no Z Axis
                   false, false, false,   // No Rx, Ry, or Rz
                   false, false,          // No rudder or throttle
                   false, false, false);

void setup()
{

  JOY_COM.begin(4800);
  Joystick.begin();
  Joystick.setXAxisRange(-5, 5);
  Joystick.setYAxisRange(-5, 5);
  pinMode(CTS_PIN, OUTPUT);
  pinMode(RTS_PIN, INPUT);

  while (!(UDADDR & _BV(ADDEN))) {} //wait till connected

}


uint8_t idle_message[3]={0xe6, 0xd6, 0x60};

uint8_t periodic_index=0;
uint8_t periodic_message[7] = {0x77,0x78,0xb2,0xb9,0xb5,0xb9,0xb4};

void loop() // run over and over
{
  static uint8_t data = 0;
  static unsigned long last_poll = millis() + JOY_POLL;

  update_joystick();

  if (JOY_COM.available()) {

    data = JOY_COM.read();
    //    Serial.print("GOT: ");
    //    Serial.print((data>0x0f)?" 0x":" 0x0");
    //    Serial.println(data,HEX);

    if (data == 0x55) {

      for(int x=0;x<3;x++){

        delay(11);
        digitalWrite(CTS_PIN, HIGH);
        delayMicroseconds(5);
  
        JOY_COM.write(idle_message[x]);
        JOY_COM.flush();
        
        delayMicroseconds(15);
        digitalWrite(CTS_PIN, LOW);
                
      }
      
      if (last_poll < millis()) { //time to poll, send keep alive
        
        delay(11);
        digitalWrite(CTS_PIN, HIGH);
        delayMicroseconds(5);
  
        JOY_COM.write(periodic_message[periodic_index]);
        JOY_COM.flush();
        
        delayMicroseconds(15);
        digitalWrite(CTS_PIN, LOW);
        
        periodic_index+=1;
        
        if(periodic_index==7)
          periodic_index=5;
        
        last_poll = millis() + JOY_POLL;
  
      }
      
    } else {


      if ((data >> 4) == 0xa) { // FUNCTION BUTTONS

        Joystick.setButton(0, (data == 0xA1));
        Joystick.setButton(1, (data == 0xA2));
        Joystick.setButton(2, (data == 0xA4));
        Joystick.setButton(3, (data == 0xA3));

      } else

        if ((data >> 4) == 0x9) {

          Joystick.setButton(4, (data & 0x01));
          Joystick.setButton(5, (data & 0x02));
          Joystick.setButton(6, (data & 0x04));
          Joystick.setButton(7, (data & 0x08));
          digitalWrite(CTS_PIN, HIGH);
          delay(5);
          JOY_COM.write(0x60 | (data & 0xf));
          delay(5);
          digitalWrite(CTS_PIN, LOW);

        } else

          if (data >> 4 == 0xc) { //first joystick movement byte

            while (JOY_COM.available() == 0) {
              digitalWrite(CTS_PIN, HIGH);
              delay(5);
              digitalWrite(CTS_PIN, LOW);
              delay(5);
            }
            uint8_t data2 = JOY_COM.read();

            if (data2 >> 4 == 0x8) { //second joystick movement byte

              if (data & 0xf) { //left right movement

                if (data & 0x8) { //LEFT

                  Joystick.setXAxis((int8_t)((0xca - data) & 0xff));

                } else {        //RIGHT

                  Joystick.setXAxis((int8_t)((data - 0xc2) & 0xff));

                }

              } else { //no left right movement

                Joystick.setXAxis(0);

              }

              if (data2 & 0xf) {

                if (data2 & 0x8) { //DOWN

                  Joystick.setYAxis((int8_t)(data2 - 0x8a));

                } else {        //UP

                  Joystick.setYAxis((int8_t)((0x82 - data2) & 0xff));

                }

              } else {

                Joystick.setYAxis(0);

              }

            }



          }

    }

  }

}



void update_joystick() {

  if (JOYTSTICK_RTS) {

    digitalWrite(CTS_PIN, HIGH);
    delay(5);

  } else {

    digitalWrite(CTS_PIN, LOW);

  }

}
