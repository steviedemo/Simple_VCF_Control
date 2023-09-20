#include <IlluminatedPushButton.h>
#include <Footswitch.h>
#include <EEPROMWearLevel.h>
#include <LED.h>
#include <stdint.h>
#define PB_1 9
#define PB_2 10
#define PB_3 8
#define PB_4 7
#define LED1 3
#define LED2 2
#define LED3 1
#define LED4 0
#define CTL_A 6
#define CTL_B 5
#define WAIT_BEFORE_SAVE 7000
void pb1CB();
void pb2CB();
void pb3CB();
void pb4CB();
void releaseCB();
void setMux(uint8_t);

#define IDX_COUNT 1
#define PB_IDX    0 

bool led_on(false);

bool setting_saved(false);
uint8_t pb_idx(0), curr_output(0);
uint32_t last_pb_press(0), last_led_blink(0), last_read(0);
//IlluminatedPushButton pb1(PB_1, LED1), pb2(PB_2, LED2), pb3(PB_3, LED3), pb4(PB_4, LED4);
Footswitch fs1(PB_1), fs2(PB_2), fs3(PB_3), fs4(PB_4);
LED led1(LED1), led2(LED2), led3(LED3), led4(LED4);
LED status_led(4);
Footswitch *buttons[4] = { &fs1, &fs2, &fs3, &fs4 };
LED *leds[4] = { &led1, &led2, &led3, &led4 };


void setup() {
  TCCR1A = _BV(COM1A1) | _BV(WGM10);
  TCCR1B = _BV(CS10) | _BV(WGM12);
  delay(500);
  for (int i = 0; i < 7; i++){
    pinMode(i, OUTPUT);
  }
  for (int i = 7; i < 11; i++){
    pinMode(i, INPUT);
  }
  delay(500);
  EEPROMwl.begin(IDX_COUNT, 600);
  uint8_t reading = EEPROMwl.read(PB_IDX);
  if (reading >= 4){
    curr_output = 0;
    EEPROMwl.write(PB_IDX, 0);
  } else {
    curr_output = reading;
  }
  curr_output &= 3;
  setMux(curr_output);
  leds[curr_output]->turnOn();
  
  fs1.attachPressCallback(pb1CB);
  fs2.attachPressCallback(pb2CB);
  fs3.attachPressCallback(pb3CB);
  fs4.attachPressCallback(pb4CB);
}

void loop() {
  uint32_t curr = millis();
  pb_idx = (pb_idx + 1) % 4;
  buttons[pb_idx]->check();
  
  if (!setting_saved && (curr - last_pb_press >= WAIT_BEFORE_SAVE)){
    EEPROMwl.write(PB_IDX, curr_output);
    setting_saved = true;
    for(int i = 0; i < 6; i++){
      status_led.toggle();
      delay(100);
    }
  }
  
  if (curr - last_led_blink >= 500){
    last_led_blink = curr;
    status_led.toggle();
  }
  
}


void setMux(uint8_t num){
  num = 3 - num;
  uint8_t n = (num & 3);
  uint8_t b = (n >> 1) & 1;
  uint8_t a = n & 1;
  digitalWrite(CTL_A, a);
  digitalWrite(CTL_B, b);
}

void setOutput(uint8_t i){
  uint8_t out = i & 0b11;
  leds[curr_output]->turnOff();
  leds[out]->turnOn();
  curr_output = out;
  setMux(curr_output);
  last_pb_press = millis();   
  setting_saved = false;
}

void pb1CB(){
  setOutput(0);
}

void pb2CB(){
  setOutput(1);
}

void pb3CB(){
  setOutput(2);
}

void pb4CB(){
  setOutput(3);
}

void saveOutput(){
  EEPROMwl.write(PB_IDX, curr_output);
  setting_saved = true;
}
