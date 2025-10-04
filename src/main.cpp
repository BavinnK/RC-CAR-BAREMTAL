#include <avr/wdt.h>
#include <Arduino.h>
#include <avr/sleep.h>
#include <MyUSART.h>
#define clk_speed 16000000
#define baud 9600
#define my_ubrr (clk_speed/16/baud-1)
enum L298N_pins{
  Enb_A=PD6,//goes to OC0A timer1 on the 328p
  Enb_B=PD5,//goes to OC0B timer 1 too bc TIMER1 is 8 bit
  pan_servo=PB1,
  tilt_servo=PB2,
  IN1=PB0,
  IN2=PD2,
  IN3=PB5,
  IN4=PD7
};
 uint16_t min_pos=2000,max_pos=4000,cen_posP=3000,cen_posT=3000;
 

  void set_timer1(void){
  //the timer setup
  TCCR1A=(1<<WGM11)|(1<<COM1A1)|(1<<COM1B1);//we gonna use a non-inverting PWM with 8 as the prescaler why? 
  // why the OCR1 is 39999 we want the top value as 20 ms with clk speed of 16Mhz of the 328p and 50hz pwm by this formula (TOP=(clk_SPEED)/(8*50)-1) we will get 39999
  //and u may think why 39999 is bc first we used 16bit timer of the 328p and the time for the counter to count from 0 till 39999 it takes 20ms or 20000us
  //bc the servo expects this time with 50hz 
  //1ms=0 degree so the counter needs to count till 2000 this means 1ms till 3000 is 1.5 and 4000 is 2ms

  //1.5=90
  //2=180
  //and the rest is counte3]4
  TCCR1B=( 1<<WGM12 )|(1<<WGM13)|(1<<CS11);
  ICR1=39999;//the top value that we want to count till this number
  //and also the pin that sends the   PWM should be a specific pin of the MCU look at the pinoutt of the arduino u see ~ symbol beside some pins means this pin can send a PWM
  //and the pin should be PB1 why when looking at the pinout of the uno or nano u see a arrow pointing to PB1 and it says TIMER OC1A
  DDRB|=(1<<PB1)|(1<<PB2);
}


//TCCR1A=(1<<WGM10)|(1<<COM1A1)|(1<<COM1B1);
//TCCR1B=(1<<WGM12)|(1<<CS11)|(1<<CS10);
void ENA_speed(uint8_t duty){
  OCR0A=duty;
}
void ENB_speed(uint8_t duty){
  OCR0B=duty;
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

//THIS timer2 is for the counting watchdog
void set_timer2(void){
  TCCR2A=0;
  TCCR2B=(1<<CS22);//with prescaler of 64 then divide the clkspeed by 64 we get 250kHz and 1 divide by that is 4us 
  TIMSK2=(1<<TOIE2);
}
//this one is for ENB AND ENA for the L298N motor driver
void set_timer0(void){
  TCCR0A=(1<<COM0A1)|(1<<COM0B1)|(1<<WGM01)|(1<<WGM00);//FASTPWM 
  TCCR0B=(1<<CS01)|(1<<CS00);//64 PRESCALER
};

void setup(){
  set_PCINT();
  watchdog_init();
  set_timer2();
  USART_init(my_ubrr);
  set_timer0();
  set_timer1();
  
  DDRB|=(1<<IN1)|(1<<IN3)|(1<<pan_servo)|(1<<tilt_servo);
  // PORTB|=(1<<Enb_A)|(1<<Enb_B); just set them HIGH means full speed low means zero 127 means hald speed cuz it from 0 to 255
  
  DDRD|=(1<<IN2)|(1<<IN4)|(1<<Enb_A)|(1<<Enb_B);

  ENA_speed(127);//means half speed
  ENB_speed(127);//same
  sei();
  USART_strTransmit("MCU starting .....\n1\n2\n3\nwatchdog is enabled!\n");

}
void loop() {
  
  if(USART_available()){
    char btn_pressed=USART_RX();
    PORTD&=~((1<<IN2)|(1<<IN4));
    PORTB&=~((1<<IN1)|(1<<IN3));
    if(btn_pressed<0) return;//safety 
    if(btn_pressed=='F'){
      PORTD|=(1<<IN2)|(1<<IN4);
      
      
      USART_strTransmit("drone going forward\n");
      
    }
    else if(btn_pressed=='B'){
      PORTB|=(1<<IN1)|(1<<IN3);
      
      
      USART_strTransmit("drone going reverse\n");
    }
    else if(btn_pressed=='L'){
      PORTB|=(1<<IN1);
      PORTD|=((1<<IN4));
      
      
      USART_strTransmit("drone going left\n");
    }
    else if(btn_pressed=='R'){
      PORTD|=(1<<IN2);
      PORTB|=((1<<IN3));
      
      USART_strTransmit("drone going right\n");
    }
    else if(btn_pressed=='X'){
      cen_posT-=100;
      OCR1B=cen_posT;
    }
    else if(btn_pressed=='T'){
      cen_posT+=100;
      OCR1B=cen_posT;
    }
    else if(btn_pressed=='C'){
      cen_posP-=100;
      OCR1A=cen_posP;

    }
    else if(btn_pressed=='S'){
      cen_posP+=100;
      OCR1A=cen_posP;
    }
    else {
      
      
      wdt_reset();
    }
    }
  else{
    //if after 50 second non of the buttons were pressed the mcu goes into a deep sleep whenever any of the button pressen and ISR will accour and wakes the MCU up again after 50 sec
    //if no buttons were used it will go back to sleep 
    if(milisecond>=50000){
      USART_strTransmit("MCU going into deep sleep\n");
      wdt_disable();
      deep_sleep();
      wdt_enable(WDTO_8S);
    }
    }
    if(cen_posT<min_pos){
      cen_posT=min_pos;
    }
    else if(cen_posT>max_pos){
      cen_posT=max_pos;
    }
    if(cen_posP<min_pos){
      cen_posP=min_pos;
    }
    else if(cen_posP>max_pos){
      cen_posP=max_pos;
    }
  
}