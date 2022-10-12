#include "Arduino.h"

#define RS 2 
#define E  3
#define NOPDEL 10

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00



void LcdClear();
void LcdOn();

void HomePos();
void SetPosition(uint8_t x, uint8_t y); 

void CursorOn();
void CursorOff();
void BlinkOn();
void BlinkOff();


void hd44Begin(uint8_t cols, uint8_t lines);
void hd44Init();
void hd44SetDataPins(uint8_t data);
void hd44WriteCmd(uint8_t cmd);
void hd44WriteData(uint8_t data);
void hd44Epulse();


/*void timer1_init();
 void nopDelay();
void microDelay(uint16_t t);
uint32_t getTime();*/

const uint8_t data_pins[8] = {6,7,8,9,10,11,12,13};

const uint8_t lcd_pozice[4][20]= {
  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13},
  {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53},
  {0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27},
  {0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67}
};

//uint8_t ddram[4][100];
//uint32_t cas = 0;
uint8_t _displayfunction, _displaycontrol, _displaymode;



void SetPosition(uint8_t x, uint8_t y){
  if(x >= 20 || y >= 4) return;
  if(x < 0 || y < 0) return;
  uint8_t pos = lcd_pozice[y][x];
  hd44WriteCmd(LCD_SETDDRAMADDR | pos);
}


void CursorOn(){
  _displaycontrol |= LCD_CURSORON;
  hd44WriteCmd(LCD_DISPLAYCONTROL | _displaycontrol);
}
void CursorOff(){
  _displaycontrol &= ~LCD_CURSORON;
  hd44WriteCmd(LCD_DISPLAYCONTROL | _displaycontrol);
}
void BlinkOn(){
  _displaycontrol |= LCD_BLINKON;
  hd44WriteCmd(LCD_DISPLAYCONTROL | _displaycontrol);
}
void BlinkOff(){
  _displaycontrol &= ~LCD_BLINKON;
  hd44WriteCmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void HomePos(){
  hd44WriteCmd(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void LcdOn() {
  _displaycontrol |= LCD_DISPLAYON;
  hd44WriteCmd(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LcdClear(){
  hd44WriteCmd(LCD_CLEARDISPLAY);
  delayMicroseconds(2000);
}


void hd44Init(){
  _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;

  hd44Begin(16, 1);
  hd44Begin(20, 4);
}

void hd44Begin(uint8_t cols, uint8_t lines){
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  pinMode(RS, OUTPUT);
  pinMode(E, OUTPUT);
  
  for (int i=0; i < 8; ++i) pinMode(data_pins[i], OUTPUT);
 
  delayMicroseconds(50000); 
  digitalWrite(RS, LOW);
  digitalWrite(E, LOW);
  
  hd44WriteCmd(LCD_FUNCTIONSET | _displayfunction);
  delayMicroseconds(4500);  // wait more than 4.1 ms

  hd44WriteCmd(LCD_FUNCTIONSET | _displayfunction);
  delayMicroseconds(150);

  hd44WriteCmd(LCD_FUNCTIONSET | _displayfunction);
  
  hd44WriteCmd(LCD_FUNCTIONSET | _displayfunction);  
    
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  LcdOn();

  LcdClear();

  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  
  hd44WriteCmd(LCD_ENTRYMODESET | _displaymode);
}
  
inline void hd44Epulse(){
  digitalWrite(E, LOW);
  delayMicroseconds(1);    
  digitalWrite(E, HIGH);
  delayMicroseconds(1);    // enable pulse must be >450 ns
  digitalWrite(E, LOW);
  delayMicroseconds(100);   // commands need >37 us to settle
  
}

inline void hd44WriteCmd(uint8_t cmd){
  digitalWrite(RS, LOW);
  hd44SetDataPins(cmd);
  hd44Epulse();
}

inline void hd44WriteData(uint8_t data){
  digitalWrite(RS, HIGH);
  hd44SetDataPins(data);
  hd44Epulse();
}


inline void hd44SetDataPins(uint8_t data){
  for(uint8_t i = 0; i < 8; i++)
    digitalWrite(data_pins[i], (data >> i) & 0x01);
}

/*void nopDelay(){
  for(uint8_t i = 0; i < NOPDEL; i++)
    __asm__ __volatile__("nop\n\t");
}

void microDelay(uint16_t t){
  //t nasobky 100us
  uint32_t cas = getTime();
  while(getTime() < cas + t);
}

void timer1_init(){
  noInterrupts();                   
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 1600;                       // po 100mikros
  TCCR1B |= (1 << WGM12);             // CTC mode
  TCCR1B |= (1 << CS10);              // ps 1
  TIMSK1 |= (1 << OCIE1A);            // enable timer compare interrupt
  interrupts();                       
}

ISR(TIMER1_COMPA_vect){          
  cas++;
}

uint32_t getTime(){
  return cas;
}*/
