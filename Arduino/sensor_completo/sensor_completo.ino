#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>
#include <SoftwareSerial.h>

const String SEPARADOR = "|";       // Una linea esta compuesta por varios datos y el Separador que se usa para reconocer donde empieza y termina una linea con datos 
const String LIMIT = ",";           // Se usa para separar los diferentes datos de una linea 

const int TRIG = 9;               // Definicion de los numeros de pines que se usaran
const int ECO = 8;
const int BOMBA = 13;
const int VALVULA1 = 12;
const int VALVULA2 = 11;
const int VALVULA3 = 10;

int duracion;                     
int distancia;
unsigned long tiempoTrig= 0;          // Variable para controlar los tiempos (reemplazo de los delay) -- Primer delay del programa anterior 
unsigned long tiempoControl= 0;       // Variable para controlar los tiempos (reemplazo de los delay) -- Segundo delay del programa anterior

const long INTERVALO_TRIG = 1;          // Define en milisegundos el primer delay del programa anterior
const long INTERVALO_CONTROL = 300;     // Define en milisegundos el segundo delay del programa anterior 
const long INTERVALO_ENVIAR = 500;    // Define en milisegundos cada cuanto se enviara la informacion a la raspberry
const long INTERVALO_RECIBIR = 100;     // Define en milisegundos cada cuanto se recibira la informacion a la raspberry

ThreadController controlHilos = ThreadController(); // Funciones para la creacion de hilos
Thread hiloRecibirComandos = Thread();              // Definicion del hilo que recibira los comandos
Thread hiloEnviarDatos = Thread();                  // Definicion del hilo que enviara los comandos

SoftwareSerial bluetooth(10, 11); // RX, TX



// Tarea 2: Recibir comandos de la raspberry
void recibirComandos(){                           // Funcion que usara el hilo para recibir los comandos
  if (bluetooth.available()){
    int comando = bluetooth.read();               // Lee el numero que se envia desde la raspberry y lo guarda en la variable comando
    switch(comando){                              // Si la variable comando es igual a 1, se ejecuta la parte que dice case 1: hasta donde dice break
                                                   // Si la variable comando es igual a 2, se ejecuta la parte que dice case 2: hasta donde dice break y asi sucesivamente
      case 1:
        if(digitalRead(BOMBA) == HIGH){
          digitalWrite(BOMBA, LOW);        
        }else{
          digitalWrite(BOMBA, HIGH);  
        }              
        break;
      case 2:
        if(digitalRead(VALVULA1) == HIGH){
          digitalWrite(VALVULA1, LOW);        
        }else{
          digitalWrite(VALVULA1, HIGH);  
        }                  
        break;
      case 3:
        if(digitalRead(VALVULA2) == HIGH){
          digitalWrite(VALVULA2, LOW);        
        }else{
          digitalWrite(VALVULA2, HIGH);  
        }                        
        break;
      case 4:
        if(digitalRead(VALVULA3) == HIGH){
          digitalWrite(VALVULA3, LOW);        
        }else{
          digitalWrite(VALVULA3, HIGH);  
        }                              
        break;
      default:                                // En caso de que el comando recibido no sea ningun numero del 1 al trece se ejecuta esta parte (No ejecutaria nada)
        break;
    }

  }
}

// Tarea 3: Enviar datos a la raspberry
void enviarDatos(){                                                       // Funcion que usara el hilo para recibir los comandos
  String estadoTrig = (digitalRead(TRIG) == HIGH)?("ON"):("OFF");         // Si digitalRead(TRIG) es igual a HIGH la variable estadoTrig seria igual a "ON" y de lo contrario seria igual a "OFF"
  String estadoEco = (digitalRead(ECO) == HIGH)?("ON"):("OFF");           
  String estadoBomba = (digitalRead(BOMBA) == HIGH)?("ON"):("OFF");
  String estadoValvula1 = (digitalRead(VALVULA1) == HIGH)?("ON"):("OFF");
  String estadoValvula2 = (digitalRead(VALVULA2) == HIGH)?("ON"):("OFF");
  String estadoValvula3 = (digitalRead(VALVULA3) == HIGH)?("ON"):("OFF");  // Se repite lo de la anterior explicacion hasta aqui
  
  String linea = LIMIT + estadoTrig + LIMIT + estadoEco + LIMIT + estadoBomba + LIMIT + estadoValvula1 +
    LIMIT + estadoValvula2 + LIMIT + estadoValvula3 + SEPARADOR;                // En la variable linea queda la informacion de los estados de los pines, donde cada uno puede ser ON o OFF y estan separados por comas
  bluetooth.println("     ");
  bluetooth.println(distancia);                                                 // Como la distancia y duracion son numeros no se pueden enviar junto con lo que hay en la variable Linea por eso se envia aparte
  bluetooth.println(LIMIT);                                                     // Para separar el dato de distancia del dato duracion se adiciona entre ellos una coma
  bluetooth.println(duracion);                                                  // Se envia el dato de duracion
  bluetooth.println(linea);                                                     // Y finalmente se envia lo que esta guardado en la variable linea

}

void setup() {
  pinMode(TRIG,OUTPUT);
  pinMode(ECO, INPUT);
  pinMode(BOMBA, OUTPUT);
  pinMode(VALVULA1, OUTPUT);
  pinMode(VALVULA2, OUTPUT);
  pinMode(VALVULA3, OUTPUT);
  
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
  
  if(tiempoActual-tiempoTrig >= INTERVALO_TRIG){                          // el tiempoTrig empieza en cero, el tiempoActual va aumentando progresivamente
                                                                          // por lo cual, tiempoActual - tiempoTrig va aumentando y cuando es mayor que INTERVALO_TRIG
    tiempoTrig = tiempoActual;                                            // entra al if, reeemplazando el delay     
    tiempoTrig = tiempoActual;                                            // tiempoTrig seria igual a tiempoActual por lo cual, mas adelante nuevamente la resta dara cero y aumentara progresivamente
    digitalWrite(TRIG, LOW);    
    duracion = pulseIn(ECO, HIGH);
    distancia = duracion / 58.2;
   // Serial.println(distancia);  
    
    if(tiempoActual-tiempoControl >= INTERVALO_CONTROL){                // Nuevamente un reemplazo del delay
      tiempoControl = tiempoActual;
      if(distancia <=50  && distancia >=10){
        digitalWrite(VALVULA1, HIGH); 
      }else{
        digitalWrite(VALVULA1, LOW);
      }
      if(distancia <=40 && distancia >=10){
        digitalWrite(VALVULA2, HIGH); 
      }
      else{
        digitalWrite(VALVULA2, LOW);  
      }
      if(distancia <=30 && distancia >=10){
        digitalWrite(VALVULA3, HIGH); 
      }
      else{
        digitalWrite(VALVULA3, LOW);  
      }
      if(distancia >= 50){
        digitalWrite(BOMBA, HIGH);    
      }
      if(distancia <= 10){
        digitalWrite(BOMBA, LOW);
      }
    } 
  }
 
  
}



