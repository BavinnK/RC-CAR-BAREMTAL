#include <avr/wdt.h>
#include <Arduino.h>
#include <avr/sleep.h>
#include <MyUSART.h>
#define clk_speed 16000000
#define baud 9600
#define my_ubrr (clk_speed/16/baud-1)
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
void set_PCINT(void){
  PCICR|=(1<<PCIE1); //this register sets the pinchange interrupt for the portC pins
  //now we have to mask it which pin we want to have intterrupt to accur
  PCMSK1|=(1<<PCINT8)|(1<<PCINT9)|(1<<PCINT10)|(1<<PCINT11);//the pins from PC0 TILL PC3
}
volatile long int milisecond=0;
ISR(PCINT1_vect){
  milisecond=0;
  // when we put the mcu to deep sleep only some parameters can wake the mcu up we only reset the counter in the ISR thats it
  //1-watchdog INT
  //2-external INTS 0 and 1
  //3-PINchange INTS we chose the pcints to wake the mcu up so we dont put anycode in the routine
}
ISR(TIMER2_OVF_vect){
  milisecond++;
  //every 1.024ms this ISR will accour and inrements the milisecond variable
}
void deep_sleep(void) {
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();
}
//in this function we set timer 2 INT so each 1024 us or 1.024ms it will send an intterrupt or we can say each 1ms overflow happens and sends an intterrupt


void set_timer2(void){
  TCCR2A=0;
  TCCR2B=(1<<CS22);//with prescaler of 64 then divide the clkspeed by 64 we get 250kHz and 1 divide by that is 4us 
  TIMSK2=(1<<TOIE2);
}
void setup(){
  set_PCINT();
  watchdog_init();
  set_timer2();
  USART_init(my_ubrr);
  DDRC&=~((1<<For_dir_btn)|(1<<Rev_dir_btn)|(1<<Lef_dir_btn)|(1<<Rig_dir_btn));
  PORTC|=(1<<For_dir_btn)|(1<<Rev_dir_btn)|(1<<Lef_dir_btn)|(1<<Rig_dir_btn);
  DDRB|=((1<<Enb_A)|(1<<Enb_B));
  // PORTB|=(1<<Enb_A)|(1<<Enb_B); just set them HIGH means full speed low means zero 127 means hald speed cuz it from 0 to 255
  
  DDRD|=(1<<IN1)|(1<<IN2)|(1<<IN3)|(1<<IN4);
  set_PWM();
  ENA_speed(127);
  ENB_speed(127);
  sei();
  USART_strTransmit("MCU starting .....\n1\n2\n3\nwatchdog is enabled!\n");

}


void loop() {
  bool btnFo_pressed=!(PINC&(1<<For_dir_btn));
  bool btnRe_pressed=!(PINC&(1<<Rev_dir_btn));
  bool btnLe_pressed=!(PINC&(1<<Lef_dir_btn));
  bool btnRi_pressed=!(PINC&(1<<Rig_dir_btn));
  
  if(btnFo_pressed){
    PORTD|=(1<<IN2)|(1<<IN4);
    PORTD&=~((1<<IN1)|(1<<IN3));
    
     USART_strTransmit("drone going forward\n");
    
  }
  else if(btnRe_pressed){
    PORTD|=(1<<IN1)|(1<<IN3);
    PORTD&=~((1<<IN2)|(1<<IN4));
    
    USART_strTransmit("drone going reverse\n");
  }
  else if(btnLe_pressed){
    PORTD|=(1<<IN1)|(1<<IN4);
    PORTD&=~((1<<IN2)|(1<<IN3));
    
    USART_strTransmit("drone going left\n");
  }
  else if(btnRi_pressed){
    PORTD|=(1<<IN2)|(1<<IN3);
    PORTD&=~((1<<IN1)|(1<<IN4));
    
    USART_strTransmit("drone going right\n");
  }
  else {
    PORTD&=~((1<<IN2)|(1<<IN1)|(1<<IN3)|(1<<IN4));
    
    wdt_reset();
  }
  //if after 50 second non of the buttons were pressed the mcu goes into a deep sleep whenever any of the button pressen and ISR will accour and wakes the MCU up again after 50 sec
  //if no buttons were used it will go back to sleep 
  if(milisecond>=50000){
     USART_strTransmit("MCU going into deep sleep\n");
     wdt_disable();
    deep_sleep();
    wdt_enable(WDTO_8S);
  }
}

