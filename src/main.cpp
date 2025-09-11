#include <avr/wdt.h>
#include <Arduino.h>
enum L298N_pins{
  For_dir_btn= PC0,
  Rev_dir_btn= PC1,
  Lef_dir_btn= PC2,
  Rig_dir_btn=PC3,
  Enb_A=PB1,//goes to OC1A timer1 on the 328p
  Enb_B=PB2,//goes to OC1B timer 1 too bc TIMER1 is 16 bit 
  IN1=PD4,
  IN2=PD5,
  IN3=PD6,
  IN4=PD7
};
void set_PWM(){
  TCCR1A=(1<<WGM10)|(1<<COM1A1)|(1<<COM1B1);
  TCCR1B=(1<<WGM12)|(1<<CS11)|(1<<CS10);

}
void ENA_speed(uint8_t duty){
  OCR1A=duty;
}
void ENB_speed(uint8_t duty){
  OCR1B=duty;
}
void watchdog_init(){
  cli();
  WDTCSR=(1<<WDE)|(1<<WDCE);//first we need to send this special sequence and wait for 4 cycles exactly then send new commands and configure the watchdog
  WDTCSR=(1<<WDE)|(1<<WDP3)|(1<<WDP0);//i configured it to 8 second the prescaler
  sei();
}
void setup() {
  watchdog_init();
  DDRC&=~((1<<For_dir_btn)|(1<<Rev_dir_btn)|(1<<Lef_dir_btn)|(1<<Rig_dir_btn));
  PORTC|=(1<<For_dir_btn)|(1<<Rev_dir_btn)|(1<<Lef_dir_btn)|(1<<Rig_dir_btn);
  DDRB|=((1<<Enb_A)|(1<<Enb_B));
 // PORTB|=(1<<Enb_A)|(1<<Enb_B); just set them HIGH means full speed low means zero 127 means hald speed cuz it from 0 to 255
  DDRD|=(1<<IN1)|(1<<IN2)|(1<<IN3)|(1<<IN4);
  set_PWM();
  ENA_speed(127);
  ENB_speed(127);

}


void loop() {
  bool btnFo_pressed=!(PINC&(1<<For_dir_btn));
  bool btnRe_pressed=!(PINC&(1<<Rev_dir_btn));
  bool btnLe_pressed=!(PINC&(1<<Lef_dir_btn));
  bool btnRi_pressed=!(PINC&(1<<Rig_dir_btn));
  if(btnFo_pressed){
    PORTD|=(1<<IN2)|(1<<IN4);
    PORTD&=~((1<<IN1)|(1<<IN3));
    wdt_reset();
    
  }
  else if(btnRe_pressed){
    PORTD|=(1<<IN1)|(1<<IN3);
    PORTD&=~((1<<IN2)|(1<<IN4));
    wdt_reset();
  }
  else if(btnLe_pressed){
    PORTD|=(1<<IN1)|(1<<IN4);
    PORTD&=~((1<<IN2)|(1<<IN3));
    wdt_reset();
  }
  else if(btnRi_pressed){
    PORTD|=(1<<IN2)|(1<<IN3);
    PORTD&=~((1<<IN1)|(1<<IN4));
    wdt_reset();
  }
  else {
    PORTD&=~((1<<IN2)|(1<<IN1)|(1<<IN3)|(1<<IN4));
    wdt_reset();
  }
}
