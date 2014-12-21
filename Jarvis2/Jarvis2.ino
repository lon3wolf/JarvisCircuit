#include <Ethernet.h>
#include <SPI.h>
// the media access control (ethernet hardware) address for the shield:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  
int states[] = {0,0,0,0,0};
int ports[] = {9,8,7,6,5};
String devices[] = {"light","fan","lamp","dev1","dev2'"};
const int devCount = 5;

int led = 9;
EthernetServer server(80);
String msg = "";

String GetAllDeviceStates()
{
  String json = "{ ";
  String state = "off";
  for(int i=0; i < devCount; i++)
  {
    json += '"' + devices[i] + '"' + ':';
    state = states[i] == 1 ? "ON" : "OFF";
    json += '"' + state + '"';
    if(i != devCount - 1)
    {
      json += ",\r\n";
    }
  }
  
  json = json + " }";
  return json;
}

void SetupPorts()
{
    for(int i=0; i < devCount; i++)
    {
      pinMode(ports[i],OUTPUT);
      digitalWrite(ports[i],LOW);
      states[i] = 0;
    }
}

String GetDeviceState(String device)
{
    for(int i=0; i < devCount; i++)
    {
      if(devices[i].equalsIgnoreCase(device)) 
      {
          return (states[i] == 1 ? "ON" : "OFF");
      }
    }
    return "";
}

int GetPortIndex(int port)
{
    for(int i=0; i < devCount; i++)
    {
      if(ports[i] == port) 
      {
          return i;
      }
    }
    return -1;
}

void SetPort(int portNum, String state)
{
    int i = GetPortIndex(portNum);
    Serial.print(i);
    if(i == -1)
    {
      return;
    }
    if(state.equalsIgnoreCase("high"))
    {
      digitalWrite(portNum,HIGH);
      states[i] = 1;
    }
    else if(state.equalsIgnoreCase("low"))
    {
      digitalWrite(portNum,LOW);
      states[i] = 0;
    }
}

int GetDevicePort(String device)
{
    for(int i=0; i < devCount; i++)
    {
      if(devices[i].equalsIgnoreCase(device)) 
      {
          return ports[i];
      }
    }
    return -1;
}

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
void setup() 
{
   // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {    ; }// wait for serial port to connect. Needed for Leonardo only

  SetupPorts();
  
  // start the Ethernet connection and the server:
  int result = 0;
  result = Ethernet.begin(mac); 
  //Ethernet.begin(mac,ip,sdns,gateway,subnet);
  if(result == 1)
  {
    Serial.print("server connected!\n");
  }
  else
  {
    Serial.print("server failed to connect!\n");
  }
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() 
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client)
  {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        
        msg += c;
        if (c == '\n') 
        {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') 
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) 
        {
          // send a standard http response header
          int startp = msg.indexOf("?")+1;
          int endp = msg.indexOf("!");
          int cmdEnd;
          String cmd, val, device, state;
          msg = msg.substring(startp,endp);
          Serial.println(msg);
          
          cmdEnd = msg.indexOf("=");
          cmd = msg.substring(0,cmdEnd);
          val = msg.substring(cmdEnd+1);
          
          if(cmd.equalsIgnoreCase("query"))
          {
            device = val;
            if(device.equalsIgnoreCase("all"))
            {
              msg = GetAllDeviceStates();
              client.println(msg);
              msg = "";
              break;
            }
            client.println(GetDeviceState(device));
            msg = "";
            break;
          }
          else
          {
            device = cmd;
            state = val;
            state = state.equalsIgnoreCase("on")? "high" : "low";
            int port = GetDevicePort(device);
            if(port == -1)
            {
              break;
            }
            Serial.print(state + " -> ");
            Serial.println(port);
            
            SetPort(port,state);
            client.println("OK");
            msg = ""; // re-init message so we can have a clean state :P
            break;
          }
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
} // end of loop


