// This #include statement was automatically added by the Particle IDE.
#include <Ubidots.h>

//#include <MCP_CAN_RK.h>


#include <mcp_can.h>
#include <SPI.h>

//WEBHOOK//////////

const char *WEBHOOK_NAME = "Ubidots";
Ubidots ubidots("webhook", UBI_PARTICLE);



//BUS CAN///////////
SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler;

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string

#define CAN0_INT A1                              // Set INT to pin A1
MCP_CAN CAN0(A2);                               // Set CS to pin A2



int ledPin = D1;                    // Indicador de funcionamiento
int danger = D2;                    // Pin de alerta temperatura de motor mayor a la de trabajo 
int vent = D3;                      // Estado del Ventilador
////////////////////////LED///////////
int led = D4;                       // activación del ventilador desde ubidots
float control = 0;
int i = 0;


bool R;

void setup()
{
    Serial.begin(115200);
     // Subscribe to the integration response event
    Particle.subscribe("UbidotsWebhook", myHandler);
    pinMode(led, OUTPUT);
 
    R = 0;
  
  
    pinMode(ledPin, OUTPUT);
    pinMode(danger, OUTPUT);
    pinMode(vent, OUTPUT);

  // Wait 10 seconds for USB debug serial to be connected (plus 1 more)
  waitFor(Serial.isConnected, 10000);
  delay(1000);

  
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input
  
  Serial.println("MCP2515 Library Receive Example...");
  
}

// Control desde ubidots/////////////////////////////////////
float setControl(float value) {
    if (value == 1) {
        return 1;
    }
    return 0;
}

void myHandler(const char *event, const char *data) {
  // Handle the integration response
  i++;
  Serial.println(i);
  Serial.print(event);
  Serial.print(", data: ");
  if (data) {
    Serial.println(data);
    control = setControl(atof(data));
  } else {
    Serial.println("NULL");
  }
}


void loop()
{
  
      if (control == 1) {
        digitalWrite(led, HIGH);
    } else {
        digitalWrite(led, LOW);
    } 
    
  
 
  if(!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
   //  Serial.print(rxBuf[0]);

   
   float temp =  rxBuf[0];


  String temp1 = String(temp);
  
  Particle.publish("temp1", temp1, PRIVATE);
  
  
  if (temp >=60){
             String danger = temp1;
            Particle.publish("danger", "Peligro temperatura elevada!!!", PRIVATE);
  }
  
  
  
 // Enviar datos a Ubidots
 
 // ubidots.add("peligro", R); // Crea y signa una variable en ubidots
 
  ubidots.add("temp", temp); // Crea y signa una variable en ubidots
  ubidots.send(WEBHOOK_NAME, PUBLIC); // Envia los datos a la variable por webhooks
  
  Serial.print (temp);
  

  ///*************************** Sistema ECT ******************************************************************************/
          if (temp >= 10){
              digitalWrite(ledPin, HIGH);

             }
  
         else {
     
             digitalWrite(ledPin, LOW);

             }

        if  (temp >= 50){
             digitalWrite(danger, HIGH);
             digitalWrite(vent, HIGH);
             

             
             
             }
  
         else {
             
            

            digitalWrite(danger,LOW);
            digitalWrite(vent,LOW);
             } 
  
   
    Serial.println();
  }
}



/*********************************************************************************************************
  END FILE
***********************************************************************************************************/


