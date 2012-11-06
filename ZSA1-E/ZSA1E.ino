/*****************************************************************************************
 * Purpose : Zabbix Sensor Agent - Environmental Monitoring Solution                     *
 * Author  : Evgeny Levkov                                                               *
 * Credits:                                                                              *
 *           JO3RI www.JO3RI.be/arduino - for web based ethernet config with EEPROM code *
 *           Rob Faludi http://www.faludi.com - for free memory test code                *
 *****************************************************************************************/

// ZSA1-E
//Credits: 
// JO3RI www.JO3RI.be/arduino - for web based ethernet config with EEPROM code
// Rob Faludi http://www.faludi.com - for free memory test code
// Evgeny Levkov - Zabbix Agent implementation http://www.sensor.im

//-----------------INCLUDES--------------------
#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
#include "DHT.h"         // Download at https://github.com/adafruit/DHT-sensor-library
#include <TextFinder.h>  // Download at http://www.arduino.cc/playground/Code/TextFinder
#include <avr/pgmspace.h>
//--------------------------------------------
#define MAX_CMD_LENGTH   25
#define MAX_LINE 20
//-----------------DHT SENSOR PARAMETERS------
#define DHTPIN 47     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);
//--------------------------------------------
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xE3, 0x5B };

IPAddress ip(10, 0, 0, 90);
IPAddress gateway(10, 0, 0, 138);
IPAddress subnet(255, 255, 255, 0);

EthernetServer server = EthernetServer(10050); //Zabbix Agent port //Zabbix agent port
EthernetServer webserver = EthernetServer(80); //Zabbix Agent port //Web setup port
EthernetClient client;
EthernetClient webclient;
boolean connected = false;
//--------------------------------------------
// This is our buffer through which we will "flow" our HTML code.
// It has to be as big as the longest character chain +1 including the "
char buffer[100];
//--------------------------------------------
// This is the HTML code used in web ethernet config 
prog_char htmlx0[] PROGMEM = "<html><title>ZSA1-E Setup Page</title><body marginwidth=\"0\" marginheight=\"0\" ";
prog_char htmlx1[] PROGMEM = "leftmargin=\"0\" style=\"margin: 0; padding: 0;\"><table bgcolor=\"#999999\" border";
prog_char htmlx2[] PROGMEM = "=\"0\" width=\"100%\" cellpadding=\"1\" style=\"font-family:Verdana;color:#fff";
prog_char htmlx3[] PROGMEM = "fff;font-size:12px;\"><tr><td>&nbsp Sensor.IM - Zabbix Sensor Agent - ZSA1-E - Ethernet Setup Page</td></tr></table><br>";
PROGMEM const char *string_table0[] = {htmlx0, htmlx1, htmlx2, htmlx3};

prog_char htmla0[] PROGMEM = "<script>function hex2num (s_hex) {eval(\"var n_num=0X\" + s_hex);return n_num;}";
prog_char htmla1[] PROGMEM = "</script><table><form><input type=\"hidden\" name=\"SBM\" value=\"1\"><tr><td>MAC:";
prog_char htmla2[] PROGMEM = "<input id=\"T1\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"DT1\" value=\"";
prog_char htmla3[] PROGMEM = "\">.<input id=\"T3\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"DT2\" value=\"";
prog_char htmla4[] PROGMEM = "\">.<input id=\"T5\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"DT3\" value=\"";
prog_char htmla5[] PROGMEM = "\">.<input id=\"T7\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"DT4\" value=\"";
prog_char htmla6[] PROGMEM = "\">.<input id=\"T9\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"DT5\" value=\"";
prog_char htmla7[] PROGMEM = "\">.<input id=\"T11\" type=\"text\" size=\"2\" maxlength=\"2\" name=\"DT6\" value=\"";
PROGMEM const char *string_table1[] = {htmla0, htmla1, htmla2, htmla3, htmla4, htmla5, htmla6, htmla7};

prog_char htmlb0[] PROGMEM = "\"><input id=\"T2\" type=\"hidden\" name=\"DT1\"><input id=\"T4\" type=\"hidden\" name=\"DT2";
prog_char htmlb1[] PROGMEM = "\"><input id=\"T6\" type=\"hidden\" name=\"DT3\"><input id=\"T8\" type=\"hidden\" name=\"DT4";
prog_char htmlb2[] PROGMEM = "\"><input id=\"T10\" type=\"hidden\" name=\"DT5\"><input id=\"T12\" type=\"hidden\" name=\"D";
prog_char htmlb3[] PROGMEM = "T6\"></td></tr><tr><td>IP: <input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT7\" value=\"";
prog_char htmlb4[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT8\" value=\"";
prog_char htmlb5[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT9\" value=\"";
prog_char htmlb6[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT10\" value=\"";
PROGMEM const char *string_table2[] = {htmlb0, htmlb1, htmlb2, htmlb3, htmlb4, htmlb5, htmlb6};

prog_char htmlc0[] PROGMEM = "\"></td></tr><tr><td>MASK: <input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT11\" value=\"";
prog_char htmlc1[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT12\" value=\"";
prog_char htmlc2[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT13\" value=\"";
prog_char htmlc3[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT14\" value=\"";
PROGMEM const char *string_table3[] = {htmlc0, htmlc1, htmlc2, htmlc3};

prog_char htmld0[] PROGMEM = "\"></td></tr><tr><td>GW: <input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT15\" value=\"";
prog_char htmld1[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT16\" value=\"";
prog_char htmld2[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT17\" value=\"";
prog_char htmld3[] PROGMEM = "\">.<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"DT18\" value=\"";
prog_char htmld4[] PROGMEM = "\"></td></tr><tr><td><br></td></tr><tr><td><input id=\"button1\"type=\"submit\" value=\"SUBMIT\" ";
prog_char htmld5[] PROGMEM = "></td></tr></form></table></body></html>";
PROGMEM const char *string_table4[] = {htmld0, htmld1, htmld2, htmld3, htmld4, htmld5};

prog_char htmle0[] PROGMEM = "Onclick=\"document.getElementById('T2').value ";
prog_char htmle1[] PROGMEM = "= hex2num(document.getElementById('T1').value);";
prog_char htmle2[] PROGMEM = "document.getElementById('T4').value = hex2num(document.getElementById('T3').value);";
prog_char htmle3[] PROGMEM = "document.getElementById('T6').value = hex2num(document.getElementById('T5').value);";
prog_char htmle4[] PROGMEM = "document.getElementById('T8').value = hex2num(document.getElementById('T7').value);";
prog_char htmle5[] PROGMEM = "document.getElementById('T10').value = hex2num(document.getElementById('T9').value);";
prog_char htmle6[] PROGMEM = "document.getElementById('T12').value = hex2num(document.getElementById('T11').value);\"";
PROGMEM const char *string_table5[] = {htmle0, htmle1, htmle2, htmle3, htmle4, htmle5, htmle6};
//--------------------------------------------
String cmd; //FOR ZABBIX COMMAND
int led = 33; //LED PORT TO BLINK AFTER RECIEVING ZABBIX COMMAND
//--------------------------------------------
const byte ID = 0x92; //used to identify if valid data in EEPROM the "know" bit, 
// if this is written in EEPROM the sketch has ran before
// We use this, so that the very first time you'll run this sketch it will use
// the values written above. 
// defining which EEPROM address we are using for what data
//--------------------------------------------

void setup()
{
     ShieldSetup (); //Setup the Ethernet shield
  
     server.begin();
     webserver.begin();
     pinMode(2, OUTPUT); //???
     pinMode(7, INPUT_PULLUP); //???
     pinMode(led, OUTPUT);    
     digitalWrite(led, LOW); 
 
}

void loop()
{
//--------------------------------------------
  client = server.available();
  if (client == true) {
    if (!connected) {
      client.flush();
      connected = true;
    }

    if (client.available() > 0) {
      readTelnetCommand(client.read());
          }

      }
  webclient = webserver.available();
//WEB CLIENT CODE-----------------------------
   if (webclient) {
    TextFinder  finder(webclient );
    while (webclient.connected()) {      
      if (webclient.available()) {
        //This part does all the text searching
        if( finder.find("GET /") ) {
          // if you find the word "setup" continue looking for more
          // if you don't find that word, stop looking and go further
          // This way you can put your own webpage later on in the sketch
          if (finder.findUntil("setup", "\n\r")){
            // if you find the word "SBM" continue looking for more
            // if you don't find that word, stop looking and go further
            // it means the SUBMIT button hasn't been pressed an nothing has
            // been submitted. Just go to the place where the setup page is
            // been build and show it in the client's browser.
          if (finder.findUntil("SBM", "\n\r")){
            byte SET = finder.getValue();
            // Now while you are looking for the letters "DT", you'll have to remember
            // every number behind "DT" and put them in "val" and while doing so, check
            // for the according values and put those in mac, ip, subnet and gateway.
              while(finder.findUntil("DT", "\n\r")){
                int val = finder.getValue();
                // if val from "DT" is between 1 and 6 the according value must be a MAC value.
                if(val >= 1 && val <= 6) {
                  mac[val - 1] = finder.getValue();
                }
                // if val from "DT" is between 7 and 10 the according value must be a IP value.
                if(val >= 7 && val <= 10) {
                  ip[val - 7] = finder.getValue();
                }
                // if val from "DT" is between 11 and 14 the according value must be a MASK value.
                if(val >= 11 && val <= 14) {
                  subnet[val - 11] = finder.getValue();
                }
                // if val from "DT" is between 15 and 18 the according value must be a GW value.
                if(val >= 15 && val <= 18) {
                  gateway[val - 15] = finder.getValue();
                }
              }
            // Now that we got all the data, we can save it to EEPROM
            for (int i = 0 ; i < 6; i++){
              EEPROM.write(i + 1,mac[i]);
            }
            for (int i = 0 ; i < 4; i++){
              EEPROM.write(i + 7, ip[i]);
            }
            for (int i = 0 ; i < 4; i++){
              EEPROM.write(i + 11, subnet[i]);
            }
            for (int i = 0 ; i < 4; i++){
              EEPROM.write(i + 15, gateway[i]);
            }
            // set ID to the known bit, so when you reset the Arduino is will use the EEPROM values
            EEPROM.write(0, 0x92); 
            // if al the data has been written to EEPROM we should reset the arduino
            // for now you'll have to use the hardware reset button
          }
          // and from this point on, we can start building our setup page
          // and show it in the client's browser.
          webclient.println("HTTP/1.1 200 OK");
          webclient.println("Content-Type: text/html");
          webclient.println();
          for (int i = 0; i < 4; i++)
            {
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table0[i])));
            webclient.print( buffer );
            }
          for (int i = 0; i < 3; i++)
            {
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[i])));
            webclient.print( buffer );
            }
          webclient.print(mac[0],HEX);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[3])));
          webclient.print( buffer );
          webclient.print(mac[1],HEX);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[4])));
          webclient.print( buffer );
          webclient.print(mac[2],HEX);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[5])));
          webclient.print( buffer );
          webclient.print(mac[3],HEX);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[6])));
          webclient.print( buffer );
          webclient.print(mac[4],HEX);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[7])));
          webclient.print( buffer );
          webclient.print(mac[5],HEX);
          for (int i = 0; i < 4; i++)
            {
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[i])));
            webclient.print( buffer );
            }
          webclient.print(ip[0],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[4])));
          webclient.print( buffer );
          webclient.print(ip[1],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[5])));
          webclient.print( buffer );
          webclient.print(ip[2],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[6])));
          webclient.print( buffer );
          webclient.print(ip[3],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table3[0])));
          webclient.print( buffer );
          webclient.print(subnet[0],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table3[1])));
          webclient.print( buffer );
          webclient.print(subnet[1],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table3[2])));
          webclient.print( buffer );
          webclient.print(subnet[2],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table3[3])));
          webclient.print( buffer );
          webclient.print(subnet[3],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[0])));
          webclient.print( buffer );
          webclient.print(gateway[0],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[1])));
          webclient.print( buffer );
          webclient.print(gateway[1],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[2])));
          webclient.print( buffer );
          webclient.print(gateway[2],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[3])));
          webclient.print( buffer );
          webclient.print(gateway[3],DEC);
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[4])));
          webclient.print( buffer );
          for (int i = 0; i < 7; i++)
            {
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table5[i])));
            webclient.print( buffer );
            }
          strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[5])));
          webclient.print( buffer );
          break;
      }}
        webclient.println("HTTP/1.1 200 OK");
        webclient.println("Content-Type: text/html");
        webclient.println();
        // put your own html from here on
        webclient.print("IT WORKS: go to ");
        webclient.print(ip[0],DEC);
        for (int i= 1; i < 4; i++){
          webclient.print(".");
          webclient.print(ip[i],DEC);
          }
        webclient.print("/setup");
        webclient.print("<BR><BR>");
		webclient.print("<font color=\"blue\"><b>Temperature:</font><font color=\"red\">");
		webclient.print(dht.readTemperature());
		webclient.print("</font><font color=\"blue\">c");
		webclient.print("     Humidity:</font><font color=\"red\">");
		webclient.print(dht.readHumidity());
		webclient.print("&nbsp;</font><font color=\"blue\"> %</b></font>");
		webclient.print("<BR><BR>");
		webclient.print("<form><button>Refresh</button></form>");
        break;  
        }
      }
    delay(2000);
    webclient.stop();
  }
}
//--------------------------------------------
void readTelnetCommand(char c) {

  if(cmd.length() == MAX_CMD_LENGTH) {
    cmd = "";
  }

  cmd += c;

  if(c == '\n') {
    if(cmd.length() > 2) {
      // remove \r and \n from the string
      cmd = cmd.substring(0,cmd.length() - 1);
      parseCommand();
    }
  }
}
//--------------------------------------------
void parseCommand() {  //Commands recieved by agent on port 10050 parsing

  // AGENT.PING      
  if(cmd.equals("agent.ping")) {
      server.println("ZBXD1");
      client.stop();
//      connected = false;

 // AGENT.VERSION      
   } else if(cmd.equals("agent.version")) {
      server.println("Sensor.IM 1.0");
      client.stop();
//      connected = false;

// MEMORY.FREE  - for diagnostic purposes
   } else if(cmd.equals("memory.free")) {
      server.println(memoryTest());
      client.stop();
      
// HUMIDITY.READ
} else if(cmd.equals("humidity.read")) {
      float h = dht.readHumidity();
       server.println(h );
     digitalWrite(led, HIGH);
      client.stop();
      delay(2000);
      digitalWrite(led, LOW);

// TEMPERATURE.READ
} else if(cmd.equals("temperature.read")) {
      float t = dht.readTemperature();
       server.println(t );
     digitalWrite(led, HIGH);
      client.stop();
//      connected = false;
      delay(2000);
      digitalWrite(led, LOW);
      
// NOT SUPPORTED      
  } else {
    //  server.println("ZBXDZBX_NOTSUPPORTED");
    server.println(cmd);
      client.stop();
//      connected = false;
  }
  cmd = "";
}
//--------------------------------------------
void ShieldSetup() //Ethernet setup - read settings from EEPROM or use default
{
  int idcheck = EEPROM.read(0);
  if (idcheck != ID){
    //ifcheck id is not the value as const byte ID,
    //it means this sketch has NOT been used to setup the shield before
    //just use the values written in the beginning of the sketch
  }
  if (idcheck == ID){
    //if id is the same value as const byte ID,
    //it means this sketch has been used to setup the shield.
    //So we will read the values out of EERPOM ans use them
    //to setup the shield.
    for (int i = 0; i < 6; i++){
      mac[i] = EEPROM.read(i+1);
    }
    for (int i = 0; i < 4; i++){
      ip[i] = EEPROM.read(i+7);
    }
    for (int i = 0; i < 4; i++){
      subnet[i] = EEPROM.read(i+11);
    }
    for (int i = 0; i < 4; i++){
      gateway[i] = EEPROM.read(i+15);
    }
  }
  Ethernet.begin(mac, ip, gateway, subnet);
}
//--------------------------------------------
// this function will return the number of bytes currently free in RAM - for diagnostics only
int memoryTest() 
{
  int byteCounter = 0; // initialize a counter
  byte *byteArray; // create a pointer to a byte array
  // More on pointers here: http://en.wikipedia.org/wiki/Pointer#C_pointers

  // use the malloc function to repeatedly attempt
  // allocating a certain number of bytes to memory
  // More on malloc here: http://en.wikipedia.org/wiki/Malloc
  while ( (byteArray = (byte*) malloc (byteCounter * sizeof(byte))) != NULL ) 
  {
    byteCounter++; // if allocation was successful, then up the count for the next try
    free(byteArray); // free memory after allocating it
  }

  free(byteArray); // also free memory after the function finishes
  return byteCounter; // send back the highest number of bytes successfully allocated
}
