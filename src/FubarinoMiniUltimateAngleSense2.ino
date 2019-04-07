/*
  AS5047D AMS magnetic position sensor demonstration sketch
  Hardware is : Fubarino Mini, default SPI port, AS5047D in 3.3V power mode

*/

#include <SPI.h>

/* Each possible address in the chip */
#define NOP_ADDR          0x0000
#define ERRFL_ADDR        0x0001
#define PROG_ADDR         0x0003
#define DIAAGC_ADDR       0x3FFC
#define MAG_ADDR          0x3FFD
#define ANGLEUNC_ADDR     0x3FFE
#define ANGLECOM_ADDR     0x3FFF

/* Bits in the ERRFL register */
#define ERRFL_PARERR_BIT  (1 << 2)
#define ERRFL_INVCOMM_BIT (1 << 1)
#define ERRFL_FRERR_BIT   (1 << 0)

/* Bits in the PROG register */
#define PROG_PROGVER_BIT  (1 << 6)
#define PROG_PROGOTP_BIT  (1 << 3)
#define PROG_OTPREF_BIT   (1 << 2)
#define PROG_PROGEN       (1 << 0)

/* Bits in the DIAACG reigster */
#define DIAAGC_MAGL_BIT   (1 << 11)
#define DIAAGC_MAGH_BIT   (1 << 10)
#define DIAAGC_COF_BIT    (1 << 9)
#define DIAAGC_LF_BIT     (1 << 8)
#define DIAAGC_AGC_BIT    (0x00FF)


#define slaveSelectPin    17

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(slaveSelectPin, OUTPUT);
  digitalWrite(slaveSelectPin, HIGH);
  SPI.begin();
  delay(250);
  writeReg(0x0019, 0xE000);
}

void loop() {
 uint16_t angle = readReg(ANGLEUNC_ADDR);
  Serial.printf("Angle:  0x%04X\n", angle);
  delay(500);
}

/* Read out all registers and print their values to the serial port */
void readAll(void)
{
  char buf[100];
  sprintf(buf, " ERRFL   PROG DIAAGC    MAG ANGLEUNC ANGLECOM SETTINGS1 SETTINGS2 \n");
  Serial.print(buf);
  sprintf(buf, "0x%04X 0x%04X 0x%04X 0x%04X   0x%04X   0x%04X  0x%04X   0x%04X\n",
          readReg(ERRFL_ADDR),
          readReg(PROG_ADDR),
          readReg(DIAAGC_ADDR),
          readReg(MAG_ADDR),
          readReg(ANGLEUNC_ADDR),
          readReg(ANGLECOM_ADDR),
          readReg(0x0018),
          readReg(0x0019)
         );
  Serial.print(buf);
}

void writeReg(uint16_t address, uint16_t data)
{
  /* Mask off parity and read/write bit. read/write must stay 0 for write */
  uint16_t command = address & 0x3FFF;
  /* Now put parity bit in */
  command = (parity(command) << 15) | command;
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE1));
  delay(1);
  digitalWrite(slaveSelectPin, LOW);
  SPI.transfer(command >> 8);
  SPI.transfer(command & 0xFF);
  digitalWrite(slaveSelectPin, HIGH);
  command = data & 0x3FFF;
  /* Now put parity bit in */
  command = (parity(command) << 15) | command;
  delay(1);
  digitalWrite(slaveSelectPin, LOW);
  SPI.transfer(command >> 8);
  SPI.transfer(command & 0xFF);
  digitalWrite(slaveSelectPin, HIGH);
  SPI.endTransaction();
  delay(1);
}

uint16_t readReg(uint16_t address)
{
  uint16_t result = 0;
  /* Mask off parity and read/write bit. read/write must be 1 for read */
  uint16_t command = (address & 0x3FFF) | 0x4000;
  /* Now put parity bit in */
  command = (parity(command) << 15) | command;
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE1));
  delay(1);
  digitalWrite(slaveSelectPin, LOW);
  SPI.transfer(command >> 8);
  SPI.transfer(command & 0xFF);
  digitalWrite(slaveSelectPin, HIGH);
  delay(1);
  digitalWrite(slaveSelectPin, LOW);
  result = SPI.transfer(0x00);
  result = (result << 8) | SPI.transfer(0x00);
  digitalWrite(slaveSelectPin, HIGH);
  SPI.endTransaction();
  delay(1);
  if(!(!parity(result&0x3FFF)^(result>>15))){ //idk why this works
    return (result & 0x3FFF); 
  }else{
    return -1;
  }
}

bool parity(uint16_t x)
{
  uint16_t y;
  y = x ^ (x >> 1);
  y = y ^ (y >> 2);
  y = y ^ (y >> 4);
  y = y ^ (y >> 8);
  return y & 1;
}


