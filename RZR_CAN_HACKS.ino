#include <SPI.h>
#include <mcp_can.h>


//debug macros. if DEBUG is defined, then we will print things via serial. if DEBUG is not defined (i.e. the following line is commented out) we will not print things
#define DEBUG

#ifdef DEBUG
#define Serialprintln Serial.println
#define Serialprint Serial.print
#else
#define Serialprintln
#define Serialprint
#endif

//initialize CAN library (9 for v1.1 and greater, 10 for v0.9b and v1.0)
MCP_CAN CAN(9);

//GLOBALS
//CONSTANTS
//rpm scale factors
//each scale factor converts RPMs to 500*MPH, (i.e. rpm*scaleFactor = mph*500)
const float RPM_SCALE_LOW     = 1.6; //low gear
const float RPM_SCALE_HIGH    = 3.5; //high gear

//CAN ID's
const unsigned long CAN_ID_CURTIS_RPM  = 0x601;      //the curtis controller RPM CAN address (read)
const unsigned long CAN_ID_RZR_SHIFTER = 0x18F00500; //the RZR CAN bus shifter address (read)
const unsigned long CAN_ID_RZR_SPEED   = 0x18FEF100; //the RZR CAN bus speed address (write)
const unsigned long CAN_ID_RZR_RPM     = 0x18FF6600; //the RZR CAN bus RPM address (write)
const unsigned long CAN_ID_RZR_CEL     = 0x18FECA00; //the RZR CAN bus check engine light (write)

//CAN BUFFERS
//the CAN buffer to turn off the check engine light
unsigned char CAN_BUF_RZR_CEL[8] = { 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF };
//RZR speed buffer
unsigned char rzrSpeedBuf[8] {
  0xFF,
  0x00, //lsb
  0x00, //msb
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF
};
//RZR rpm buffer
unsigned char rzrRPMBuf[8] = {
  0x00, //lsb
  0x00, //msb
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00
};

//VARIABLES
//scale factor for RPM to MPH
float rpmScaleFactor = 0;
//timing variable for asynchronous CAN message sending
unsigned long timePassed = 0;

void setup() {

  //initialize serial communication
  Serial.begin(115200);

  //initialize the CAN bus shield
START_INIT:
  if (CAN_OK == CAN.begin(CAN_250KBPS))
  {
    Serialprintln("CAN BUS Shield init ok!");
  }
  else
  {
    Serialprintln("CAN BUS Shield init fail");
    delay(100);
    goto START_INIT;
  }
  //start the async timing
  timePassed = millis();
}

void loop() {
  //check the async timing and send messages if need be
  if (millis() - timePassed > 9) {
    CAN.sendMsgBuf(CAN_ID_RZR_CEL, 1, 8, CAN_BUF_RZR_CEL);
    CAN.sendMsgBuf(CAN_ID_RZR_SPEED, 1, 8, rzrSpeedBuf);
    CAN.sendMsgBuf(CAN_ID_RZR_RPM, 1, 8, rzrRPMBuf);
    timePassed = millis();
  }
  
  
  //read off the CAN bus, looking for curtis RPM signal or RZR shifter position signal

  //vairables to store data
  unsigned char len = 0;
  unsigned char buf[8];

  //check if we have a message on the bus
  int canReceive = CAN.checkReceive();
  if (CAN_MSGAVAIL == canReceive)
  {
    //read the data and get the CAN ID
    CAN.readMsgBuf(&len, buf);
    unsigned long canID = CAN.getCanId();

    //check if its the CAN ID we want
    //RPM from curtis
    if (canID == CAN_ID_CURTIS_RPM) {
      //get rpm
      unsigned long rpm = buf[0] << 8 | buf[1];
      if (rpm > 0) {
        Serialprint("RPM: ");
        Serialprintln(rpm);
      }

      //calculate speed
      unsigned long speedVal = (unsigned long)((float)rpm * rpmScaleFactor);

      //debugging
      if (rpm > 0) {
        Serialprint("speed: ");
        Serialprintln(speedVal/500);
        Serialprint("Timestamp: ");
        Serialprintln(millis());
      }
      
      //set the speed lsb
      rzrSpeedBuf[1] = speedVal & 0xFF;
      //set the speed msb
      rzrSpeedBuf[2] = speedVal >> 8;

      //set the rpm lsb
      rzrRPMBuf[0] = rpm & 0xFF;
      //set the rpm msb
      rzrRPMBuf[1] = rpm >> 8;

      //shifter position
    } else if (canID == CAN_ID_RZR_SHIFTER) {
      
      //check what gear we are in, and set the scale factor accordingly
      int pos = buf[5];
      
      //low
      if (pos == 0x4C) {
        rpmScaleFactor = RPM_SCALE_LOW;
      //high
      } else if (pos == 0x48) {
        rpmScaleFactor = RPM_SCALE_HIGH;
      //anything else (park, neutral, reverse, error)
      } else {
        rpmScaleFactor = 0;
      }
    }
  }
}
