#include <LiquidCrystal.h>
#include <avr/io.h>
#include <avr/interrupt.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

byte grad[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

volatile uint8_t ore     = 18;
volatile uint8_t minute  = 1;
volatile uint8_t secunde = 50;

volatile bool secundaNoua = true;

void adc_init() {
  ADMUX = (1 << REFS0);
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t canal) {
  ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  return ADC;
}

float citesteTemperatura() {
  uint32_t suma = 0;
  for (uint8_t i = 0; i < 8; i++) {
    suma += adc_read(0);
  }
  float valoareADC = suma / 8.0;
  float mV = valoareADC * 5000.0 / 1024.0;
  return (mV - 500.0) / 10.0;
}

void timer1_init() {
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 15624;                         
  TCCR1B |= (1 << WGM12);                
  TCCR1B |= (1 << CS12) | (1 << CS10);   
  TIMSK1 |= (1 << OCIE1A);               
  sei();
}

ISR(TIMER1_COMPA_vect) {
  secunde++;
  if (secunde >= 60) {
    secunde = 0;
    minute++;
    if (minute >= 60) {
      minute = 0;
      ore++;
      if (ore >= 24) {
        ore = 0;
      }
    }
  }
  secundaNoua = true;
}

void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0, grad);
  adc_init();
  timer1_init();
}

void loop() {
  if (secundaNoua) {
    cli();
    uint8_t h = ore;
    uint8_t m = minute;
    uint8_t s = secunde;
    secundaNoua = false;
    sei();

    float temp = citesteTemperatura();

    lcd.setCursor(0, 0);
    lcd.print("Temp = ");
    lcd.print((int)temp);
    lcd.write(byte(0));
    lcd.print("C      ");

    char buffer[17];
    sprintf(buffer, "Ora %02d:%02d:%02d   ", h, m, s);
    lcd.setCursor(0, 1);
    lcd.print(buffer);
  }
}
