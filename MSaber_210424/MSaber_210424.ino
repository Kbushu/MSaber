/*
   Swing and strike
*/
#define DEBUG 1             // debug information in Serial (1 - allow, 0 - disallow)

// ---------------------------- SETTINGS -------------------------------
#define SWING_TIMEOUT 500   // timeout between swings
#define SWING_L_THR 150     // swing angle speed threshold 300
#define SWING_THR 200       // fast swing angle speed threshold 150
#define STRIKE_THR 150      // hit acceleration threshold
#define STRIKE_S_THR 320    // hard hit acceleration threshold
#define FLASH_DELAY 80      // flash time while hit

// -------------------------- LIBS ---------------------------
#include <Wire.h>
#include <MPU6050.h>
#include <TMRpcm.h>         // audio from SD library
TMRpcm tmrpcm;
#define IMU_GND A1

MPU6050 accelgyro;

const int ledR_Pin =  5;// the number of the LED pin
const int ledG_Pin =  6;// the number of the LED pin
const int ledB_Pin =  3;// the number of the LED pin

// ------------------------------ VARIABLES ---------------------------------
int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long ACC, GYR, COMPL;
int gyroX, gyroY, gyroZ, accelX, accelY, accelZ, freq, freq_f = 20;
float k = 0.2;
unsigned long humTimer = -9000, mpuTimer, nowTimer;
unsigned long previousMillis = 0;        // will store last time LED was updated
byte brightness;

// constants won't change:
const long interval = 30;           // 30 is good interval at which to blink (milliseconds)
int ledState = LOW;             // ledState used to set the LED
//unsigned int buzz; //the buzzer frequency
//const int buzzer = 9; //buzzer to arduino pin 9
boolean eeprom_flag, swing_flag, swing_allow, strike_flag, HUMmode = 0; //set hummode to 0 as we don't need it now
unsigned long btn_timer, PULSE_timer, swing_timer, swing_timeout, battery_timer, bzzTimer;
byte nowNumber;
boolean ls_state = 1;

// --------------------------------- SOUNDS ----------------------------------
const char strike1[] PROGMEM = "SK1.wav";
const char strike2[] PROGMEM = "SK2.wav";
const char strike3[] PROGMEM = "SK3.wav";
const char strike4[] PROGMEM = "SK4.wav";
const char strike5[] PROGMEM = "SK5.wav";
const char strike6[] PROGMEM = "SK6.wav";
const char strike7[] PROGMEM = "SK7.wav";
const char strike8[] PROGMEM = "SK8.wav";

const char* const strikes[] PROGMEM  = {
  strike1, strike2, strike3, strike4, strike5, strike6, strike7, strike8
};

int strike_time[8] = {779, 563, 687, 702, 673, 661, 666, 635};

const char strike_s1[] PROGMEM = "SKS1.wav";
const char strike_s2[] PROGMEM = "SKS2.wav";
const char strike_s3[] PROGMEM = "SKS3.wav";
const char strike_s4[] PROGMEM = "SKS4.wav";
const char strike_s5[] PROGMEM = "SKS5.wav";
const char strike_s6[] PROGMEM = "SKS6.wav";
const char strike_s7[] PROGMEM = "SKS7.wav";
const char strike_s8[] PROGMEM = "SKS8.wav";

const char* const strikes_short[] PROGMEM = {
  strike_s1, strike_s2, strike_s3, strike_s4,
  strike_s5, strike_s6, strike_s7, strike_s8
};
int strike_s_time[8] = {270, 167, 186, 250, 252, 255, 250, 238};

const char swing1[] PROGMEM = "SWS1.wav";
const char swing2[] PROGMEM = "SWS2.wav";
const char swing3[] PROGMEM = "SWS3.wav";
const char swing4[] PROGMEM = "SWS4.wav";
const char swing5[] PROGMEM = "SWS5.wav";

const char* const swings[] PROGMEM  = {
  swing1, swing2, swing3, swing4, swing5
};
int swing_time[8] = {389, 372, 360, 366, 337};

const char swingL1[] PROGMEM = "SWL1.wav";
const char swingL2[] PROGMEM = "SWL2.wav";
const char swingL3[] PROGMEM = "SWL3.wav";
const char swingL4[] PROGMEM = "SWL4.wav";

const char* const swings_L[] PROGMEM  = {
  swingL1, swingL2, swingL3, swingL4
};
int swing_time_L[8] = {636, 441, 772, 702};

char BUFFER[10];
// --------------------------------- SOUNDS ---------------------------------

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps

  pinMode(IMU_GND, OUTPUT);
  digitalWrite(IMU_GND, 0);
  // set the digital pin as output:
  pinMode(ledR_Pin, OUTPUT);
  pinMode(ledG_Pin, OUTPUT);
  pinMode(ledB_Pin, OUTPUT);
  //  pinMode(buzzer, OUTPUT); // Set buzzer - pin 9 as an output

  // IMU initialization
  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  //  if (DEBUG) {
  if (accelgyro.testConnection()) Serial.println(F("MPU6050 OK"));
  else Serial.println(F("MPU6050 fail"));
  //  }

  // SD initialization
  tmrpcm.speakerPin = 9;
  tmrpcm.setVolume(4);
  tmrpcm.quality(1);
  if (DEBUG) {
    if (SD.begin(8)) Serial.println(F("SD OK"));
    else Serial.println(F("SD fail"));
  } else {
    SD.begin(8);
  }
  //  swing_flag = 1;
  //  strike_flag = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  getFreq();
  getEffect1();
  //  getEffect();
  //  end of loop
}

//this is working ok
void getEffect1() {
  if ((GYR >= SWING_L_THR) && (millis() - swing_timeout > 100)) {
    tmrpcm.loop(0);
    nowNumber = random(5);
    if (GYR > SWING_THR) {
      Serial.println(F("Long swing!"));
      strcpy_P(BUFFER, (char*)pgm_read_word(&(swings_L[nowNumber])));
      tmrpcm.play(BUFFER);
      while (tmrpcm.isPlaying() == 1){
              delay(100);
      }
    } else {
      Serial.println(F("Swing!"));
      strcpy_P(BUFFER, (char*)pgm_read_word(&(swings[nowNumber])));
      tmrpcm.play(BUFFER);
//      TODO if the while works put it here also
      delay(400);
    }
  } else {
    tmrpcm.play("HUM.WAV");
    tmrpcm.loop(1);
  }
}
void getFreq() {
  if (ls_state) {                                               // if GyverSaber is on
    if (millis() - mpuTimer > 500) {
      accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

      // find absolute and divide on 100
      gyroX = abs(gx / 100);
      gyroY = abs(gy / 100);
      gyroZ = abs(gz / 100);
      accelX = abs(ax / 100);
      accelY = abs(ay / 100);
      accelZ = abs(az / 100);

      // vector sum
      ACC = sq((long)accelX) + sq((long)accelY) + sq((long)accelZ);
      ACC = sqrt(ACC);
      GYR = sq((long)gyroX) + sq((long)gyroY) + sq((long)gyroZ);
      GYR = sqrt((long)GYR);
      COMPL = ACC + GYR;
      /*
         // отладка работы IMU
         Serial.print("$");
         Serial.print(gyroX);
         Serial.print(" ");
         Serial.print(gyroY);
         Serial.print(" ");
         Serial.print(gyroZ);
         Serial.println(";");
      */
//      Serial.println(GYR);
      freq = (long)COMPL * COMPL / 1500;                        // parabolic tone change
      freq = constrain(freq, 18, 300);
      freq_f = freq * k + freq_f * (1 - k);                     // smooth filter
      mpuTimer = micros();
    }
  }
}

//effect
void getEffect() {
  //  if ((ACC < STRIKE_THR) && GYR > 80 && (millis() - swing_timeout > 100)) {
  if ((ACC < STRIKE_THR) && GYR > 80 && (millis() - swing_timeout > 100)) {
    swing_timeout = millis();
    if (GYR >= SWING_THR) {
      Serial.println(F("Swing!"));
      nowNumber = random(5);
      strcpy_P(BUFFER, (char*)pgm_read_word(&(swings[nowNumber])));
      tmrpcm.play(BUFFER);
    } else if ((GYR > SWING_L_THR) && (GYR < SWING_THR)) {
      Serial.println(F("Long swing!"));
      nowNumber = random(5);
      strcpy_P(BUFFER, (char*)pgm_read_word(&(swings_L[nowNumber])));
      tmrpcm.play(BUFFER);
    }
  } else if ((ACC > STRIKE_THR) && (ACC < STRIKE_S_THR)) {
    nowNumber = random(8);
    //    hard strike
    if (ACC >= STRIKE_S_THR) {
      strcpy_P(BUFFER, (char*)pgm_read_word(&(strikes[nowNumber])));
      tmrpcm.play(BUFFER);
      Serial.println(F("hard strike!"));
    } else {
      //      soft strike
      strcpy_P(BUFFER, (char*)pgm_read_word(&(strikes_short[nowNumber])));
      tmrpcm.play(BUFFER);
      Serial.println(F("soft strike!"));
    }
    //    Serial.println(BUFFER);
  }
  //  wait for sound to finish playing
  //  delay(5000);
}
