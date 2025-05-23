/*
  mtconnect_adapter

 MTConnect adapter for iVAC Tool Plus, which detects current on any electrical powered device.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13 if not using OPTA 
 * Optocoupler attached to digital pin 8.

 created 16 May 2025
 */
 #include <Ethernet.h>

const int HEARTBEAT_TIMEOUT = 10000;
const int OPTO_PIN = 8;

String incoming = "";
boolean alreadyConnected = false;
int currState = 0;

// Static IP configuration for the Opta device.
IPAddress ip(10, 130, 22, 84); 

EthernetServer server(7878);
EthernetClient client;

void setup() {
  Serial.begin(9600);
  
  while(!Serial);

  //pinMode(OPTO_PIN, INPUT);

  //Try starting Ethernet connection via DHCP
  if (Ethernet.begin() == 0) {
    Serial.println("- Failed to configure Ethernet using DHCP!");

    // Try to configure Ethernet with the predefined static IP address.
    Ethernet.begin(ip);
  }

   // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  server.begin();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());

}

void loop() {
  // listen for incoming mtconnect agent
  loopTime = millis();
  client = server.accept();

  if(client) 
  {
    Serial.println("New client");
    boolean currentLineIsBlank = true;
    while(client.connected())
    {
      if (!alreadyConnected)
      {
        //send initial SHDR data on first connection
        if(Ethernet.linkStatus() == LinkON)
        {
          client.println("|avail|AVAILABLE\n");
          Serial.println("Sent: |avail|AVAILABLE\n");
        } else
        {
          client.println("|avail|UNAVAILABLE\n");
          Serial.println("Sent: |avail|UNAVAILABLE\n");
        }
        client.println("* shdrVersion: 2.0\n");
        Serial.println("Sent: * shdrVersion: 2.0\n");

        client.println("* adapterVersion: 2.0\n");
        Serial.println("Sent: * adapterVersion: 2.0\n");

        currState = digitalRead(OPTO_PIN);

        client.println("|ToolPlus_A1|" + String(currState) + "\n");
        Serial.println("Sent: |ToolPlus_A1|" + String(currState) +"\n");
        alreadyConnected = true;
      } 
      //send data when state changes
      else 
      {
        int newState = digitalRead(OPTO_PIN);
        if(newState != currState)
        {
          client.println("|ToolPlus_A1|" + String(newState) +"\n");
          Serial.println("Sent: |ToolPlus_A1|" + String(newState) +"\n");

          currState = newState;
        }  
      }

      if (client.available()) 
      {
        char c = client.read();
        incoming += c;

        if(incoming.indexOf("* PING") >= 0)
        {
          lastPing = loopTime;
          client.println("* PONG " + String(HEARTBEAT_TIMEOUT) + "\n");
          Serial.println("Sent: * PONG " + String(HEARTBEAT_TIMEOUT) + "\n");
          incoming = "";
        }
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }
}