#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>
#include <SoftwareSerial.h>

const String SEPARADOR = "|";       // Una linea esta compuesta por varios datos y el Separador que se usa para reconocer donde empieza y termina una linea con datos 
const String LIMIT = ",";           // Se usa para separar los diferentes datos de una linea 

const int TRIG = 9;               // Definicion de los numeros de pines que se usaran
const int TRIG1 = 6;               // Definicion de los numeros de pines que se usaran
const int ECO = 8;
const int ECO1 = 7;
const int BOMBA = 13;
const int VALVULA1 = 12;
const int VALVULA2 = 11;
const int VALVULA3 = 10;


double altura = 110.0;
double altura1 = 100.0;

double duracion;                     
double duracion1;                     
double distancia;
double distancia1;
unsigned long tiempoTrig= 0;          // Variable para controlar los tiempos (reemplazo de los delay) -- Primer delay del programa anterior 
unsigned long tiempoTrig1= 0;          // Variable para controlar los tiempos (reemplazo de los delay) -- Tercer delay del programa anterior 
unsigned long tiempoControl= 0;       // Variable para controlar los tiempos (reemplazo de los delay) -- Segundo delay del programa anterior
unsigned long tiempoFinal= 0;       // Variable para controlar los tiempos (reemplazo de los delay) -- Cuarto delay del programa anterior
bool modoManual = false;              // Variable para controlar si las valvulas y la bomba se administraran mediante la arduino o manual, cuando es manual es true

const long INTERVALO_TRIG = 1;          // Define en milisegundos el primer delay del programa anterior
const long INTERVALO_TRIG1 = 1;          // Define en milisegundos el tercer delay del programa anterior
const long INTERVALO_CONTROL = 300;     // Define en milisegundos el segundo delay del programa anterior 
const long INTERVALO_FINAL = 1;     // Define en milisegundos el cuarto delay del programa anterior 

const long INTERVALO_ENVIAR = 100;    // Define en milisegundos cada cuanto se enviara la informacion a la raspberry
const long INTERVALO_RECIBIR = 1;     // Define en milisegundos cada cuanto se recibira la informacion a la raspberry

bool IntervaloTrigActive = false;
bool IntervaloTrig1Active = false;
bool IntervaloControlActive = false;
bool IntervaloFinalActive = false;



ThreadController controlHilos = ThreadController(); // Funciones para la creacion de hilos
Thread hiloRecibirComandos = Thread();              // Definicion del hilo que recibira los comandos
Thread hiloEnviarDatos = Thread();                  // Definicion del hilo que enviara los comandos

SoftwareSerial bluetooth(10, 11); // RX, TX



// Tarea 2: Recibir comandos de la raspberry
void recibirComandos(){                           // Funcion que usara el hilo para recibir los comandos
  if (bluetooth.available()){
    char comando = bluetooth.read();               // Lee el numero que se envia desde la raspberry y lo guarda en la variable comando     
    Serial.println(comando);
    if(comando == 'a') digitalWrite(BOMBA, LOW);
    else if(comando == 'b') digitalWrite(VALVULA1, LOW);
    else if(comando == 'c') digitalWrite(VALVULA2, LOW);
    else if(comando == 'd') digitalWrite(VALVULA3, LOW);
    else if(comando == 'e') modoManual = false;
    else if(comando == 'A') digitalWrite(BOMBA, HIGH);
    else if(comando == 'B') digitalWrite(VALVULA1, HIGH);
    else if(comando == 'C') digitalWrite(VALVULA2, HIGH);
    else if(comando == 'D') digitalWrite(VALVULA3, HIGH);                
    else if(comando == 'E') modoManual = true;
       
  }
}


// Tarea 3: Enviar datos a la raspberry
void enviarDatos(){                                                       // Funcion que usara el hilo para recibir los comandos
  //String estadoTrig = (digitalRead(TRIG) == HIGH)?("ON"):("OFF");         // Si digitalRead(TRIG) es igual a HIGH la variable estadoTrig seria igual a "ON" y de lo contrario seria igual a "OFF"
  //String estadoTrig1 = (digitalRead(TRIG1) == HIGH)?("ON"):("OFF");
  //String estadoEco = (digitalRead(ECO) == HIGH)?("ON"):("OFF");           
  //String estadoEco1 = (digitalRead(ECO1) == HIGH)?("ON"):("OFF");           
  String estadoBomba = (digitalRead(BOMBA) == HIGH)?("ON"):("OFF");
  String estadoValvula1 = (digitalRead(VALVULA1) == HIGH)?("ON"):("OFF");
  String estadoValvula2 = (digitalRead(VALVULA2) == HIGH)?("ON"):("OFF");
  String estadoValvula3 = (digitalRead(VALVULA3) == HIGH)?("ON"):("OFF");  // Se repite lo de la anterior explicacion hasta aqui
  String estadoModoManual = (modoManual == true) ? "ON" : "OFF";
  
  //String linea = LIMIT + estadoTrig + LIMIT + estadoTrig1 + LIMIT + estadoEco 
  //+ LIMIT + estadoEco1 + LIMIT + estadoBomba + LIMIT + estadoValvula1 + LIMIT 
  //+ estadoValvula2 + LIMIT + estadoValvula3 + LIMIT + estadoModoManual + SEPARADOR;                // En la variable linea queda la informacion de los estados de los pines, donde cada uno puede ser ON o OFF y estan separados por comas
  
  String linea = LIMIT + estadoBomba + LIMIT + estadoValvula1 + LIMIT 
  + estadoValvula2 + LIMIT + estadoValvula3 + LIMIT + estadoModoManual + SEPARADOR;                // En la variable linea queda la informacion de los estados de los pines, donde cada uno puede ser ON o OFF y estan separados por comas
  
  bluetooth.println("     ");
  bluetooth.println(String(altura,2)); 
  bluetooth.println(LIMIT);
  bluetooth.println(String(altura1,2)); 
  bluetooth.println(LIMIT);
  bluetooth.println(String(distancia,2));                                                 // Como la distancia y duracion son numeros no se pueden enviar junto con lo que hay en la variable Linea por eso se envia aparte
  //bluetooth.println(LIMIT);                                                     // Para separar el dato de distancia del dato duracion se adiciona entre ellos una coma
  //bluetooth.println(duracion);  
  bluetooth.println(LIMIT);          
  bluetooth.println(String(distancia1,2));                                                 // Como la distancia y duracion son numeros no se pueden enviar junto con lo que hay en la variable Linea por eso se envia aparte
  //bluetooth.println(LIMIT);                                                     // Para separar el dato de distancia del dato duracion se adiciona entre ellos una coma
  //bluetooth.println(duracion1);                                                  // Se envia el dato de duracion
  bluetooth.println(linea);                                                     // Y finalmente se envia lo que esta guardado en la variable linea
  Serial.println(String(duracion) + " " +  String(duracion1) + " " +  String(distancia) + " " + String(distancia1));
}

void setup() {
  pinMode(TRIG,OUTPUT);
  pinMode(TRIG1,OUTPUT);
  pinMode(ECO, INPUT);
  pinMode(ECO1, INPUT);
  pinMode(BOMBA, OUTPUT);
  pinMode(VALVULA1, OUTPUT);
  pinMode(VALVULA2, OUTPUT);
  pinMode(VALVULA3, OUTPUT);
  
  IntervaloTrigActive = true;
  tiempoTrig = millis();  
  
  Serial.begin(9600); 
  bluetooth.begin(9600);
  
  hiloRecibirComandos.onRun(recibirComandos);                                       // Se inicializa el hilo de recibir comandos y se pone el nombre de la funcion que ejecutara el hilo
  hiloRecibirComandos.setInterval(INTERVALO_RECIBIR);                               // Se define cada cuanto se ejecutara la funcion en milisegundos
  
  hiloEnviarDatos.onRun(enviarDatos);
  hiloEnviarDatos.setInterval(INTERVALO_ENVIAR);  

  controlHilos.add(&hiloRecibirComandos);                                          // Esta funcion trabaja sola y se encarga de gestionar los hilos, por lo cual se ponen los nombres de los hilos
  controlHilos.add(&hiloEnviarDatos);
}

// Tarea 1: gestion de los sensores
void loop() {  
  controlHilos.run();                                                     // Se indica ala funcion que gestiona los hilos que los arranque

  digitalWrite(TRIG, HIGH); 
  unsigned long tiempoActual = millis();                                  // Obtiene el tiempo actual
  
  if(tiempoActual-tiempoTrig >= INTERVALO_TRIG && IntervaloTrigActive == true){                            // el tiempoTrig empieza en cero, el tiempoActual va aumentando progresivamente
                                        // por lo cual, tiempoActual - tiempoTrig va aumentando y cuando es mayor que INTERVALO_TRIG
    tiempoTrig = tiempoActual;                                            // entra al if, reeemplazando el delay         
    digitalWrite(TRIG, LOW);                          // tiempoTrig seria igual a tiempoActual por lo cual, mas adelante nuevamente la resta dara cero y aumentara progresivamente
    duracion = pulseIn(ECO, HIGH);
    distancia = duracion / 58.2;  
    // Serial.println(distancia);
    IntervaloTrigActive = false;
    IntervaloControlActive = true;
    tiempoControl = tiempoActual;
  }
  
  if(tiempoActual-tiempoControl >= INTERVALO_CONTROL && IntervaloControlActive == true){                // Nuevamente un reemplazo del delay      
    tiempoControl = tiempoActual;
    if(modoManual == false){     
      
      if(distancia > (altura*.49) && distancia1 < (altura*1.0)) digitalWrite(BOMBA, HIGH);
      else if(distancia <= (altura*.1) || distancia1 >= (altura*1.0)) digitalWrite(BOMBA, LOW);
      else if(distancia > (altura*.1) && distancia1 < (altura*.1))
      
      if(distancia >= (altura*.5)) digitalWrite(VALVULA2, LOW); 
      else if(distancia < (altura*.5)) digitalWrite(VALVULA2, HIGH); 
      
      if(distancia >= (altura*.3)) digitalWrite(VALVULA3, LOW); 
      else if(distancia < (altura*.3)) digitalWrite(VALVULA3, HIGH); 
      
    }
    IntervaloControlActive = false;
    IntervaloTrig1Active = true;
    tiempoTrig1 = tiempoActual;
  }
  
    
  digitalWrite(TRIG1, HIGH);  
  if(tiempoActual-tiempoTrig1 >= INTERVALO_TRIG && IntervaloTrig1Active == true){                // Otro un reemplazo del delay      
    tiempoTrig1 = tiempoActual;
    digitalWrite(TRIG1, LOW);   
    duracion1 = pulseIn(ECO1, HIGH);
    distancia1 = duracion1 / 58.2;
    IntervaloTrig1Active = false;
    IntervaloFinalActive = true;
    tiempoFinal = tiempoActual;   
  }
  
  if(tiempoActual-tiempoFinal >= INTERVALO_CONTROL && IntervaloFinalActive == true){                // Otro un reemplazo del delay      
    IntervaloFinalActive = false;
    IntervaloTrigActive = true;
    tiempoTrig = tiempoActual;            
  }
 
  
}
