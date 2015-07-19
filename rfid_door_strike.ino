#include <SoftwareSerial.h>  // SoftwareSerial must be included because the library depends on it
#include "RFID.h"
#include <EEPROM.h>
#define doorPin 6


// Creates an RFID instance in Wiegand Mode
// DATA0 of the RFID Reader must be connected 
// to Pin 2 of your Arduino (INT0 on most boards, INT1 on Leonardo)
// DATA1 of the RFID Reader must be connected
// to Pin 3 of your Arduino (INT1 on most boards, INT0 on Leonrado)
RFID rfid(RFID_WIEGAND, W26BIT);
// Declares a struct to hold the data of the RFID tag
// Available fields:
//  * id (3 Bytes) - card code
//  * valid - validity

RFIDTag tag;
long MASTER_ADD = 9038195;
long MASTER_DEL = 9033777;
long storedCard;
boolean programMode = false;

void setup() 
{
  pinMode(doorPin, OUTPUT);
  Serial.begin(9600);  // Initializes serial port
  // Waits for serial port to connect. Needed for Leonardo only
  while ( !Serial ) ;
}

void loop()
{
  if( rfid.available() )  // Checks if there is available an RFID tag
  {
    tag = rfid.getTag();  // Retrieves the information of the tag
    Serial.print("CC = ");  // and prints that info on the serial port
    Serial.println(tag.id, DEC);
    /*        
    Serial.print("The ID is ");
    if (tag.valid) Serial.println("valid");
    else Serial.println("invalid");
    */
    if (programMode) 
    {
      if ( tag.id != MASTER_ADD)
      {
        writeID(tag.id);
        Serial.println("New key added");
        programMode = false;
      }
    } 
    else if ( tag.id == MASTER_ADD)
    {
      programMode = true;
      Serial.println("Detected MASTER ADD key");
      Serial.println("Swipe key you wish to add...");
    } 
    else if ( tag.id == MASTER_DEL)
    {
      if (EEPROMReadLong(0) > 0 ) { EEPROMWriteLong(0,0); }
      Serial.println("Detected MASTER DEL key");
      Serial.println("All guest keys disabled");
    }
    else if ( findID( tag.id ) )
    {
      Serial.println("Detected VALID key");
      Serial.println("Open Door"); 
      openDoor(2);
    } 
    else 
    { 
      Serial.println("ID not valid"); 
    }
  }
}

// Looks in the EEPROM to try to match any of the EEPROM ID's with the passed ID
boolean findID( long tagId )
{
  long count = EEPROMReadLong(0);           // Read the first Byte of EEPROM that
  //Serial.print("Count: ");                // stores the number of ID's in EEPROM
  //Serial.print(count);
  //Serial.print("\n");
  for ( long i = 1; i <= count; i++ )      // Loop once for each EEPROM entry
  {
    long readId = EEPROMReadLong(i);      // Read an ID from EEPROM, it is stored in storedCard[6]
    if( readId == tagId )    // Check to see if the storedCard read from EEPROM 
    {                                     // is the same as the find[] ID card passed
      //Serial.print("We have a matched card!!! \n");
      return true;
      break;                              // Stop looking we found it
    }
    else                                  // If not, return false
    {
      //Serial.print("No Match here.... \n");
    }
  }
  return false;
}

// Write an array to the EEPROM in the next available slot
void writeID( long tagId )
{
  if ( !findID( tagId ) )          // Before we write to the EEPROM, check to see if we have seen this card before!
  {
    long num = EEPROMReadLong(0);  // Get the numer of used spaces, position 0 stores the number of ID cards
    num++;                         // Increment the counter by one
    EEPROMWriteLong( 0, num );        // Write the new count to the counter
    EEPROMWriteLong( num, tagId );
    //successWrite();
  }
  else
  {
    //failedWrite();
  }
}

void openDoor(int setDelay)
{
  setDelay *=1000; // Sets delay in seconds
  digitalWrite(doorPin, HIGH); // Unlock door
  delay(setDelay);
  digitalWrite(doorPin,LOW); // Relock door
}

long EEPROMReadLong( long address )  // Number = position in EEPROM to get the 5 Bytes from 
{
  long start = (address * 4 );  // Figure out starting position
  long four = EEPROM.read(start);
  long three = EEPROM.read(start+1);
  long two = EEPROM.read(start+2);
  long one = EEPROM.read(start+3);
  return ((four<<0)&0xFF) + ((three<<8)&0xFFFF) + ((two<<16)&0xFFFFFF) + ((one<<24)&0xFFFFFFFF);
}

void EEPROMWriteLong(long address, long value)
{
  long start = (address * 4 );  // Figure out starting position
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  
  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(start, four);
  EEPROM.write(start + 1, three);
  EEPROM.write(start + 2, two);
  EEPROM.write(start + 3, one);
}
