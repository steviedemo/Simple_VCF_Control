
#define FREQ_IN A0
#define ENGAGE  A3
#define SS    7
#define MOSI  6
#define SCK   4
#define CTL_A 8
#define CTL_B 9
#define CTL_C 10
#include <KnobSections.h>
uint16_t last_pot_reading(0);
uint8_t pot_reading(0), temp_reading(0);
uint8_t output_number(0);
bool engaged(false);

void setup() {
  delay(500);
  // put your setup code here, to run once:
  DDRB |= _BV(PB0) | _BV(PB1) | _BV(PB2); // 4051 controls as outputs
  // Unused pins & 595 comm pins as outputs:
  DDRA |= _BV(PA1) | _BV(PA2) | _BV(PA4) | _BV(PA5) | _BV(PA6) | _BV(PA7);
  // Eng Latch & Pot pin as inputs
  DDRA &= ~(_BV(PA0) | _BV(PA3));
  engaged = (digitalRead(ENGAGE) == LOW);
  pot_reading = analogRead(A0) >> 2;
  output_number = splitInto8(pot_reading);
  uint8_t encoding = encode(output_number);
  if (engaged){
    encoding = ~encoding;
  }
  writeWord(encoding);
  

}

void loop() {
  // put your main code here, to run repeatedly:
  uint16_t curr = millis();
  if (curr - last_pot_reading >= 2){
    bool active = (digitalRead(ENGAGE) == LOW);
    if (active != engaged){
      engaged = active;
      uint8_t encoding = encode(output_number);
      if (engaged){
         encoding = ~encoding;
      }
      writeWord(encoding);
    }
      pot_reading = analogRead(A0) >> 2;
      if (eightWayPot(pot_reading, &output_number)){
          uint8_t encoding = encode(output_number);     
          if (engaged){
            encoding = ~encoding;   
          }
          writeWord(encoding);
          setMux(output_number);
      }
      last_pot_reading = curr;
  }
}




void setOutput(volatile uint8_t *port, uint8_t pin, bool state){
  if (state){
    *port |= (1<<pin);
  } else { 
    *port &= ~(1<<pin);
  }
}

void setMux(uint8_t idx){
  uint8_t c = (idx >> 2) & 1;
  uint8_t b = (idx >> 1) & 1;
  uint8_t a = (idx & 1);
  setOutput(&PORTB, PB2, (a == 1));
  setOutput(&PORTB, PB1, (b == 1));
  setOutput(&PORTB, PB0, (c == 1));
}


void displaySetting(bool show, bool invert_display){
  if (show){
    uint8_t encoding = encode(output_number);
    if (invert_display){
      writeWord(~encoding);
    } else {
      writeWord(encoding);
    }
  } else {
    clearDisplay();
  }
}


uint8_t encode(uint8_t val){
  uint8_t san_val = constrain(val, 0, 7);
  return (1 << san_val);
}

void writeWord(uint8_t b){
  PORTA &= ~_BV(PA7);
  for (int i = 7; i >= 0; i--){
    uint8_t v = (b >> i) & 1;
    writeBit(v);
  }
  PORTA |= _BV(PA7);
}

void clearDisplay(){
  PORTA &= ~_BV(PA7);
  for (int i = 0; i < 8; i++){
    writeBit(0);
  }
  PORTA |= _BV(PA7);
}

void writeBit(uint8_t val){
  val &= 1;
  PORTA &= ~_BV(PA4);
  if (val == 0){
    PORTA &= ~_BV(PA6);
  } else {
    PORTA |= _BV(PA6);
  }
  PORTA |= _BV(PA4);
}
