

/******** DEFINICIÓN DE LIBRERIA, VARIABLES Y CONSTANTES************************/
#include <math.h>                   // libreria de matemáticas 
#define coolantsensorDivider 3800   //valor de la resistencia que se usa para el divisor de tension, similar al valor de la resistencia del ECT medido a temperatura ambiente
#define coolantsensorPin A0         //Pin de la entrada analógica del divisor de tensión
#define NUMSAMPLES 5                //Valor divisor para sacar el valor promedio de valores leidos

#include <Adafruit_DHT.h>
//#include <MCP_CAN_RK.h>
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#include <mcp_can.h>
#include <SPI.h>

float h = 0;
float t = 0;

SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler;


#define CAN0_INT A1                              // Set INT to pin A1
MCP_CAN CAN0(A2);                               // Set CS to pin A2



String estado_temp;     //Cadema para el estado de la temperatura

/*********************** Hallando las constantes para la ecuación de STEINHART*****************************************/
const float steinconstA = 0.00132774106461327;        //Constante A de la ecuación de steinhart
const float steinconstB = 0.000254470874104285;       //Constante B de la ecuación de steinhart
const float steinconstC = 0.000000101216538378909;    //Constante C de la ecuación de steinhart

int samples[NUMSAMPLES];                              //Variable que almacena el número de medidas


/***************************VOID SETUP********************************************/

void setup()
{
  Serial.begin(115200);

  // Wait 10 seconds for USB debug serial to be connected (plus 1 more)
  waitFor(Serial.isConnected, 10000);
  delay(1000);
  
  dht.begin(); //start dht sensor
  
    // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");

  CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
}


/***************************VOID LOOP********************************************/
void loop()
{
  

  uint8_t i;                                          //integer for loop
  float average;                                      //decimal for average
  
  for (i=0; i<NUMSAMPLES; i++) {                      
    samples[i] = analogRead(coolantsensorPin);        //toma muestras de los valores leidos
    delay(10);
  }

  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
    average += samples[i];                            //Suma las muestras obtenidas
  }
  average /= NUMSAMPLES;                              //Divide por el número de muestras 

  Serial.print("Lectura promedio del valor analogico del sensor = ");
  Serial.println(average);                                        // Muestra el valor promedio leido
  average = (coolantsensorDivider*average)/(7050-average);        //Conversion del valor leido 
  Serial.print("Valor de la resistencia del sensor = ");
  Serial.println(average);

  float steinhart;                              //La acuación de steinhart estima la temperatura a cualquier valor de resistencia segun la curva 
  steinhart = log(average);                     //lnR
  steinhart = pow(steinhart,3);                 //(lnR)^3
  steinhart *= steinconstC;                     //C*((lnR)^3)
  steinhart += (steinconstB*(log(average)));    //B*(lnR) + C*((lnR)^3)
  steinhart += steinconstA;                     //Complete equation, 1/T=A+BlnR+C(lnR)^3
  steinhart = 1.0/steinhart;                    //Inverse to isolate for T
  steinhart -= 273.15;                          //Conversion de kelvin a celcius

  Serial.print("Temperatura = ");
  Serial.print(steinhart);                      //Muestra el valor de la temperatura
  Serial.println(" *C");
  
 // String temp = String(steinhart);                  // webhook
 // Particle.publish("tempect", temp, PRIVATE);       // webhook
/////////////////////////////////////////////////////////////////////////////////

delay(200);


  h = dht.getHumidity();
  t = dht.getTempCelcius();
  
//Serial.println(h);
Serial.println(t);
delay(200);


unsigned char buf[1] = {steinhart};


  byte sndStat = CAN0.sendMsgBuf(0x100, 0, 8, buf);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.printlnf("Error Sending Message %d", sndStat);
  }
  delay(100);   // send data per 100ms*/
}



/*********************************************************************************************************
  END FILE
*********************************************************************************************************/