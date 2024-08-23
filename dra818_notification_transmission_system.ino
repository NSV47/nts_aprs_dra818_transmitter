/*
 *  Copyright (C) 2018 - Handiko Gesang - www.github.com/handiko
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <math.h>
#include <stdio.h>

#include <SoftwareSerial.h>

//------------------------------------------------
#include <LibAPRS.h>

// You must define what reference voltage the ADC
// of your device is running at. If you bought a
// MicroModem from unsigned.io, it will be running
// at 3.3v if the "hw rev" is greater than 2.0.
// This is the most common. If you build your own
// modem, you should know this value yourself :)
#define ADC_REFERENCE REF_3V3
// OR
//#define ADC_REFERENCE REF_5V

// You can also define whether your modem will be
// running with an open squelch radio:
#define OPEN_SQUELCH true

// You always need to include this function. It will
// get called by the library every time a packet is
// received, so you can process incoming packets.
//
// If you are only interested in transmitting, you
// should just leave this function empty.

//----------------------------------------------------
// Defines the Square Wave Output Pin
#define OUT_PIN 8

#define _1200   1
#define _2400   0

#define _FLAG       0x7e
#define _CTRL_ID    0x03
#define _PID        0xf0
#define _DT_EXP     ','
#define _DT_STATUS  '>'
#define _DT_POS     '!'

#define _FIXPOS         1
#define _STATUS         2
#define _FIXPOS_STATUS  3

// Defines the Dorji Control PIN
#define _PTT      2
//#define _PD       4
//#define _POW      2

#define DRJ_TXD 12
#define DRJ_RXD 11

SoftwareSerial dorji(DRJ_RXD, DRJ_TXD);

bool nada = _2400;

long timer_1 = 0;

float transmit_frequency = 433.0000;
float receive_frequency  = 433.0015;

/*
 * SQUARE WAVE SIGNAL GENERATION
 * 
 * baud_adj lets you to adjust or fine tune overall baud rate
 * by simultaneously adjust the 1200 Hz and 2400 Hz tone,
 * so that both tone would scales synchronously.
 * adj_1200 determined the 1200 hz tone adjustment.
 * tc1200 is the half of the 1200 Hz signal periods.
 * 
 *      -------------------------                           -------
 *     |                         |                         |
 *     |                         |                         |
 *     |                         |                         |
 * ----                           -------------------------
 * 
 *     |<------ tc1200 --------->|<------ tc1200 --------->|
 *     
 * adj_2400 determined the 2400 hz tone adjustment.0
 * tc2400 is the half of the 2400 Hz signal periods.
 * 
 *      ------------              ------------              -------
 *     |            |            |            |            |
 *     |            |            |            |            |            
 *     |            |            |            |            |
 * ----              ------------              ------------
 * 
 *     |<--tc2400-->|<--tc2400-->|<--tc2400-->|<--tc2400-->|
 *     
 */
const float baud_adj = 0.985; // 0.975
const float adj_1200 = 1.0 * baud_adj;
const float adj_2400 = 1.0 * baud_adj;
unsigned int tc1200 = (unsigned int)(0.5 * adj_1200 * 1000000.0 / 1200.0);
unsigned int tc2400 = (unsigned int)(0.5 * adj_2400 * 1000000.0 / 2400.0);

/*
 * This strings will be used to generate AFSK signals, over and over again.
 */
const char *mycall = "MYCALL";
char myssid = 1;

const char *dest = "APRS";

const char *digi = "WIDE2";
char digissid = 1;

//const char *mystatus = "Hello World, This is a simple Arduino APRS Transmitter !";
const char *mystatus = "TEST";

const char *lat = "0610.55S";
const char *lon = "10649.62E";
const char sym_ovl = 'H';
const char sym_tab = 'a';

const char strings[860]={
"b0NWtAXLKj0Sn8WRsakzQS8JN25zAAf3md5ILaYty6jvZHrq1QU1CWfC6tKOMY7cFCopla9sn0b\
n26zcd9qRHFWflqMcmMwx9ZDmzxrs4cfjiMox4R0pNCB0fm26gDVcdMCZcVOnovLDWUlFHL0m2UL\
j3SVJonE4swIlemv2miVFJ3hjETh54cubpJhefhHtOGlwwtd64PigxsjzB3oXI6tJR3sCd84sheQ\
is2DrnBZPd4pYdZvv6nx01hDeQNiUYGilAHb7cdqlEIMwhHVaqIgn43JOwQzSMGOWvAbFdSxLyoU\
d8rYeyVWHxW3tyJS7wjWjsr1UV3RCkPBL4XhMpceV3z0zu6y9rQGWxBwVAbBliOo630lkdmwRkuM\
B0INNcS4CjELYzsVQnEnX5OMCryDdbFEGwCpDEiFPETlP4EeqsYI6ACIRsM9A8buf1eecrwBKgkT\
3Ty0mHlOjc4ibBiJCJB5vTzvEbQdfgsLGubfPL1Y8Vb5PAzwCGVotWxUPUPamGgBezXZ4JbOAbUf\
XGEM1ppuRtam8zk4ePExs1ccD4qumNt0pvfEWyCiIrVuLAK1TGoOG9rE0U0wCaLILlmLiTu1UtPM\
STm1sZzEAdunENMmMrHH4bO5W3dL36Njoq7fCVyFGiIurYBcmamYRWHFas3f6DCN7IpOiKo0PM1E\
If7eeVegEB4lQZ5EVSXJ4HpGodk4h903bu4KIfm2VilJUUtjiy9lMqTXGliafDss5zBGpL8S7yh1\
z2NdgD8TrRGXR4EJ9gSiJTCBiGoSe1uzoeqPNV1pMM7ld7bKbTriOlBNyTCm7lx7cM8J5IsO4ieg\
CSjG0OzwiQEhed7hvS2b78Qu"
};

unsigned int tx_delay = 5000;
unsigned int str_len = 400;

char bit_stuff = 0;
unsigned short crc=0xffff;

//----------------------------------------------
// IMPORTANT! This function is called from within an
// interrupt. That means that you should only do things
// here that are FAST. Don't print out info directly
// from this function, instead set a flag and print it
// from your main loop, like this:

boolean gotPacket = false;
AX25Msg incomingPacket;
uint8_t *packetData;
//----------------------------------------------

/*
 * 
 */
void set_nada_1200(void);
void set_nada_2400(void);
void set_nada(bool nada);

void send_char_NRZI(unsigned char in_byte, bool enBitStuff);
void send_string_len(const char *in_string, int len);

void calc_crc(bool in_bit);
void send_crc(void);

void send_packet(char packet_type);
void send_flag(unsigned char flag_len);
void send_header(void);
void send_payload(char type);

void set_io(void);
void print_code_version(void);
void print_debug(char type);

void dorji_init(SoftwareSerial &ser);
void dorji_reset(SoftwareSerial &ser);
void dorji_setvol(byte vol, SoftwareSerial &ser);
void dorji_setfreq(float txf, float rxf, SoftwareSerial &ser);
void dorji_setfilter(byte prd, byte lff, byte rff, SoftwareSerial &ser);
void dorji_readback(SoftwareSerial &ser);

//-------------------------------------------------------------------------

void aprs_msg_callback(struct AX25Msg *msg) {
  // If we already have a packet waiting to be
  // processed, we must drop the new one.
  if (!gotPacket) {
    // Set flag to indicate we got a packet
    gotPacket = true;

    // The memory referenced as *msg is volatile
    // and we need to copy all the data to a
    // local variable for later processing.
    memcpy(&incomingPacket, msg, sizeof(AX25Msg));

    // We need to allocate a new buffer for the
    // data payload of the packet. First we check
    // if there is enough free RAM.
    if (freeMemory() > msg->len) {
      packetData = (uint8_t*)malloc(msg->len);
      memcpy(packetData, msg->info, msg->len);
      incomingPacket.info = packetData;
    } else {
      // We did not have enough free RAM to receive
      // this packet, so we drop it.
      gotPacket = false;
    }
  }
}

//-------------------------------------------------------------------------

/*
 * 
 */
void set_nada_1200(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    digitalWrite(OUT_PIN, HIGH);
    delayMicroseconds(tc1200);
    digitalWrite(OUT_PIN, LOW);
    delayMicroseconds(tc1200);
  }
}

void set_nada_2400(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    digitalWrite(OUT_PIN, HIGH);
    delayMicroseconds(tc2400);
    digitalWrite(OUT_PIN, LOW);
    delayMicroseconds(tc2400);
    
    digitalWrite(OUT_PIN, HIGH);
    delayMicroseconds(tc2400);
    digitalWrite(OUT_PIN, LOW);
    delayMicroseconds(tc2400);
  }
}

void set_nada(bool nada)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  if(nada)
    set_nada_1200();
  else
    set_nada_2400();
  }
}

/*
 * This function will calculate CRC-16 CCITT for the FCS (Frame Check Sequence)
 * as required for the HDLC frame validity check.
 * 
 * Using 0x1021 as polynomial generator. The CRC registers are initialized with
 * 0xFFFF
 */
void calc_crc(bool in_bit)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  unsigned short xor_in;
  
  xor_in = crc ^ in_bit;
  crc >>= 1;

  if(xor_in & 0x01)
    crc ^= 0x8408;
  }
}

void send_crc(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  unsigned char crc_lo = crc ^ 0xff;
  unsigned char crc_hi = (crc >> 8) ^ 0xff;

  send_char_NRZI(crc_lo, HIGH);
  send_char_NRZI(crc_hi, HIGH);
  }
}

void send_header(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  char temp;

  /*
   * APRS AX.25 Header 
   * ........................................................
   * |   DEST   |  SOURCE  |   DIGI   | CTRL FLD |    PID   |
   * --------------------------------------------------------
   * |  7 bytes |  7 bytes |  7 bytes |   0x03   |   0xf0   |
   * --------------------------------------------------------
   * 
   * DEST   : 6 byte "callsign" + 1 byte ssid
   * SOURCE : 6 byte your callsign + 1 byte ssid
   * DIGI   : 6 byte "digi callsign" + 1 byte ssid
   * 
   * ALL DEST, SOURCE, & DIGI are left shifted 1 bit, ASCII format.
   * DIGI ssid is left shifted 1 bit + 1
   * 
   * CTRL FLD is 0x03 and not shifted.
   * PID is 0xf0 and not shifted.
   */

  /********* DEST ***********/
  temp = strlen(dest);
  for(int j=0; j<temp; j++)
    send_char_NRZI(dest[j] << 1, HIGH);
  if(temp < 6)
  {
    for(int j=0; j<(6 - temp); j++)
      send_char_NRZI(' ' << 1, HIGH);
  }
  send_char_NRZI('0' << 1, HIGH);

  
  /********* SOURCE *********/
  temp = strlen(mycall);
  for(int j=0; j<temp; j++)
    send_char_NRZI(mycall[j] << 1, HIGH);
  if(temp < 6)
  {
    for(int j=0; j<(6 - temp); j++)
      send_char_NRZI(' ' << 1, HIGH);
  }
  send_char_NRZI((myssid + '0') << 1, HIGH);

  
  /********* DIGI ***********/
  temp = strlen(digi);
  for(int j=0; j<temp; j++)
    send_char_NRZI(digi[j] << 1, HIGH);
  if(temp < 6)
  {
    for(int j=0; j<(6 - temp); j++)
      send_char_NRZI(' ' << 1, HIGH);
  }
  send_char_NRZI(((digissid + '0') << 1) + 1, HIGH);

  /***** CTRL FLD & PID *****/
  send_char_NRZI(_CTRL_ID, HIGH);
  send_char_NRZI(_PID, HIGH);
}
}
void send_payload(char type)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  if(type == _FIXPOS)
  {
    send_char_NRZI(_DT_POS, HIGH);
    send_string_len(lat, strlen(lat));
    send_char_NRZI(sym_ovl, HIGH);
    send_string_len(lon, strlen(lon));
    send_char_NRZI(sym_tab, HIGH);
  }
  else if(type == _STATUS)
  {
    send_char_NRZI(_DT_STATUS, HIGH);
    send_string_len(mystatus, strlen(mystatus));
  }
  else if(type == _FIXPOS_STATUS)
  {
    send_char_NRZI(_DT_POS, HIGH);
    send_string_len(lat, strlen(lat));
    send_char_NRZI(sym_ovl, HIGH);
    send_string_len(lon, strlen(lon));
    send_char_NRZI(sym_tab, HIGH);

    send_char_NRZI(' ', HIGH);
    send_string_len(mystatus, strlen(mystatus));
  }
  }

}

/*
 * This function will send one byte input and convert it
 * into AFSK signal one bit at a time LSB first.
 * 
 * The encode which used is NRZI (Non Return to Zero, Inverted)
 * bit 1 : transmitted as no change in tone
 * bit 0 : transmitted as change in tone
 */
void send_char_NRZI(unsigned char in_byte, bool enBitStuff)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  bool bits;
  
  for(int i = 0; i < 8; i++)
  {
    bits = in_byte & 0x01;

    calc_crc(bits);

    if(bits)
    {
      set_nada(nada);
      bit_stuff++;

      if((enBitStuff) && (bit_stuff == 5))
      {
        nada ^= 1;
        set_nada(nada);
        
        bit_stuff = 0;
      }
    }
    else
    {
      nada ^= 1;
      set_nada(nada);

      bit_stuff = 0;
    }

    in_byte >>= 1;
  }
  }
}

void send_string_len(const char *in_string, int len)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  for(int j=0; j<len; j++)
    send_char_NRZI(in_string[j], HIGH);
  }
}

void send_flag(unsigned char flag_len)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  for(int j=0; j<flag_len; j++)
    send_char_NRZI(_FLAG, LOW); 
  }
}

/*
 * In this preliminary test, a packet is consists of FLAG(s) and PAYLOAD(s).
 * Standard APRS FLAG is 0x7e character sent over and over again as a packet
 * delimiter. In this example, 100 flags is used the preamble and 3 flags as
 * the postamble.
 */
void send_packet(char packet_type)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  print_debug(packet_type);

  //digitalWrite(LED_BUILTIN, 1);
  digitalWrite(_PTT, LOW);
  
  /*
   * AX25 FRAME
   * 
   * ........................................................
   * |  FLAG(s) |  HEADER  | PAYLOAD  | FCS(CRC) |  FLAG(s) |
   * --------------------------------------------------------
   * |  N bytes | 22 bytes |  N bytes | 2 bytes  |  N bytes |
   * --------------------------------------------------------
   * 
   * FLAG(s)  : 0x7e
   * HEADER   : see header
   * PAYLOAD  : 1 byte data type + N byte info
   * FCS      : 2 bytes calculated from HEADER + PAYLOAD
   */
  
  send_flag(100);
  crc = 0xffff;
  send_header();
  send_payload(packet_type);
  send_crc();
  send_flag(3);

  digitalWrite(_PTT, HIGH);
  //digitalWrite(LED_BUILTIN, 0);
  }
}

/*
 * Function to randomized the value of a variable with defined low and hi limit value.
 * Used to create random AFSK pulse length.
 */
void randomize(unsigned int &var, unsigned int low, unsigned int high)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  var = random(low, high);
  }
}

/*
 * 
 */
void set_io(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT);

  pinMode(DRJ_RXD, INPUT);
  pinMode(DRJ_TXD, OUTPUT);
  pinMode(_PTT, OUTPUT);
  //pinMode(_PD, OUTPUT);
  //pinMode(_POW, OUTPUT);

  digitalWrite(_PTT, LOW);
  //digitalWrite(_PD, HIGH);
  //digitalWrite(_POW, LOW);
  
  Serial.begin(115200);
  dorji.begin(9600);
  }
}

void print_code_version(void)
{
  Serial.println(" ");
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");
  
  Serial.println("Random String Pulsed AFSK Generator - Started \n");
}

void print_debug(char type)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  int temp;
  
  /*
   * PROTOCOL DEBUG
   * 
   * MYCALL-N>APRS,DIGIn-N:<PAYLOAD STRING> <CR><LF>
   */
   
  temp = strlen(mycall);
  for(int j=0; j<temp; j++)
    Serial.print(mycall[j]);
  Serial.print('-');
  Serial.print(myssid, DEC);
  Serial.print('>');

  temp = strlen(dest);
  for(int j=0; j<temp; j++)
    Serial.print(dest[j]);
  Serial.print(',');

  temp = strlen(digi);
  for(int j=0; j<temp; j++)
    Serial.print(digi[j]);
  Serial.print('-');
  Serial.print(digissid, DEC);
  Serial.print(':');

  
  if(type == _FIXPOS)
  {
    Serial.print(_DT_POS);
    Serial.print(lat);
    Serial.print(sym_ovl);
    Serial.print(lon);
    Serial.print(sym_tab);
  }
  else if(type == _STATUS)
  {
    Serial.print(_DT_STATUS);
    Serial.print(mystatus);
  }
  else if(type == _FIXPOS_STATUS)
  {
    Serial.print(_DT_POS);
    Serial.print(lat);
    Serial.print(sym_ovl);
    Serial.print(lon);
    Serial.print(sym_tab);

    Serial.print(' ');
    Serial.print(mystatus);
  }
  
  Serial.println(' ');
  }
}

/*
 * 
 */
void dorji_init(SoftwareSerial &ser)
{
  ser.println("AT+DMOCONNECT");
}

void dorji_reset(SoftwareSerial &ser)
{
  for(char i=0;i<3;i++)
    ser.println("AT+DMOCONNECT");
}

void dorji_setvol(byte vol, SoftwareSerial &ser)
{
  ser.print("AT+DMOSETVOLUME=");
  ser.println(vol, 1);
}

void dorji_setfreq(byte bnd, float txf, float rxf, SoftwareSerial &ser)
{
  ser.print("AT+DMOSETGROUP="); // bandwidth 25 KHz!
  ser.print(bnd);
  ser.print(',');
  ser.print(txf, 4);
  ser.print(',');
  ser.print(rxf, 4);
  ser.println(",0000,0,0000");
}

void dorji_setfilter(byte prd, byte lff, byte rff, SoftwareSerial &ser) // pre-order, low frequency filter, rf filter
{
  ser.print("AT+SETFILTER=");
  ser.print(prd, 1);
  ser.print(',');
  ser.print(lff, 1);
  ser.print(',');
  ser.println(rff, 1);
}

void dorji_readback(SoftwareSerial &ser)
{
  String d;
  
  while(ser.available() < 1);
  if(ser.available() > 0)
  {
    d = ser.readString();
    Serial.print(d);
  }
}

/*
 * 
 */
void setup()
{
 
  set_io();
  print_code_version();

  delay(250);
  
  dorji_reset(dorji);
  dorji_readback(dorji);
  delay(1000);
  dorji_setfreq(1, transmit_frequency, receive_frequency, dorji);
  dorji_readback(dorji);
  dorji_setvol(8, dorji);
  dorji_readback(dorji);
  dorji_setfilter(1, 0, 0, dorji);
  dorji_readback(dorji);

  Serial.println(' ');

  delay(1000);

  //randomSeed(analogRead(A0));

  //-------------------------------------------------
  // Initialise APRS library - This starts the modem
  APRS_init(ADC_REFERENCE, OPEN_SQUELCH);

  // You must at a minimum configure your callsign and SSID
  APRS_setCallsign("NOCALL", 1);
  
  // You don't need to set the destination identifier, but
  // if you want to, this is how you do it:
  // APRS_setDestination("APZMDM", 0);
  
  // Path parameters are set to sensible values by
  // default, but this is how you can configure them:
  // APRS_setPath1("WIDE1", 1);
  // APRS_setPath2("WIDE2", 2);
  
  // You can define preamble and tail like this:
  // APRS_setPreamble(350);
  // APRS_setTail(50);
  
  // You can use the normal or alternate symbol table:
  // APRS_useAlternateSymbolTable(false);
  
  // And set what symbol you want to use:
  // APRS_setSymbol('n');
  
  // We can print out all the settings
  APRS_printSettings();
  Serial.print(F("Free RAM:     ")); Serial.println(freeMemory());
  //-------------------------------------------------
}

//----------------------------------------------------------------------
void locationUpdateExample() {
  // Let's first set our latitude and longtitude.
  // These should be in NMEA format!
  APRS_setLat("5530.80N");
  APRS_setLon("01143.89E");
  
  // We can optionally set power/height/gain/directivity
  // information. These functions accept ranges
  // from 0 to 10, directivity 0 to 9.
  // See this site for a calculator:
  // http://www.aprsfl.net/phgr.php
  // LibAPRS will only add PHG info if all four variables
  // are defined!
  APRS_setPower(2);
  APRS_setHeight(4);
  APRS_setGain(7);
  APRS_setDirectivity(0);
  
  // We'll define a comment string
  char *comment = "LibAPRS location update";
    
  // And send the update
  APRS_sendLoc(comment, strlen(comment));
  
}

void messageExample() {
  //------------------------------------------

  //dorji_setfreq(1, 433.000, 433.001, dorji); // tx rx
  //dorji_readback(dorji);

  //digitalWrite(3, LOW);
  //delay(100);
  //------------------------------------------
  // We first need to set the message recipient
  APRS_setMessageDestination("AA3BBB", 0);
  
  // And define a string to send
  char *message = "Hi there! This is a message.";
  //char *message = "GET MILLIS TIME";
  APRS_sendMsg(message, strlen(message));

  //------------------------------------------

  //dorji_setfreq(0, 433.000, 433.001, dorji); // tx rx
  //dorji_readback(dorji);
  
  //------------------------------------------
}

// Here's a function to process incoming packets
// Remember to call this function often, so you
// won't miss any packets due to one already
// waiting to be processed
void processPacket() {
  if (gotPacket) {
    //char data[100];
    gotPacket = false;
    
    Serial.print(F("Received APRS packet. SRC: "));
    Serial.print(incomingPacket.src.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.src.ssid);
    Serial.print(F(". DST: "));
    Serial.print(incomingPacket.dst.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.dst.ssid);
    Serial.print(F(". Data: "));
    
    for (int i = 0; i < incomingPacket.len; i++) {
      Serial.write(incomingPacket.info[i]);
      //data[i]=incomingPacket.info[i];
    }
    Serial.println("");
    //Serial.println(data);
    // Remeber to free memory for our buffer!
    free(packetData);

    // You can print out the amount of free
    // RAM to check you don't have any memory
    // leaks
    // Serial.print(F("Free RAM: ")); Serial.println(freeMemory());
  }
}

boolean whichExample = false;
//-----------------------------------------------------------------------
/*
ADCSRA =    _BV(ADEN) |
            _BV(ADSC) |
            _BV(ADATE)|
            _BV(ADIE) |
            _BV(ADPS2);
*/

bool flag;

void loop()
{
  //randomize(tx_delay, 100, 10000);
  //if(millis()-timer_1>=10000){ 
  //  timer_1=millis();
  //  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  //      send_packet(1);
  //  }
  //}
  //ADCSRA |= (1<<ADEN);
  //send_packet(random(1,4));
  
  //send_packet(1);
  
  //delay(tx_delay);
  //randomize(tx_delay, 10, 5000);
  //randomize(str_len, 10, 420);

  //---------------------------------------------

  //delay(10000);
  //flag=true;
  if(millis()-timer_1>=10000&&true){ 
    timer_1=millis();
    //if(flag){
      flag=false;
      messageExample();
      //delay(500);
      messageExample();
      //delay(500);
    //}
    //messageExample();
    //delay(500);
#if 0
    if (whichExample&&false) {
      //dorji_setfreq(1, transmit_frequency, receive_frequency, dorji);
      //delay(500);
      locationUpdateExample();
      locationUpdateExample();
      delay(500);
      //dorji_setfreq(0, transmit_frequency, receive_frequency, dorji);
      //delay(500);
    } else {
      //dorji_setfreq(1, transmit_frequency, receive_frequency, dorji);
      //delay(500);
      messageExample();
      messageExample();
      delay(500);
      //dorji_setfreq(0, transmit_frequency, receive_frequency, dorji);
      //delay(500);
    }
    whichExample ^= true;
#endif
  }
  //delay(500);
  processPacket();
  
  //---------------------------------------------
}
