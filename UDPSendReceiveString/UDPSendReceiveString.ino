// tftp 192.168.1.10 -c get test.txt
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <SD.h>
 
File Archivo;
int l_nombre;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 10);

unsigned int localPort = 69;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.begin(9600);
  //Se muestra por pantalla que se va a iniciar la comunicación con la SD
  Serial.print("Comenzando la comunicación con la tarjeta SD");
  
  //Se establece como salida el pin correspondiente a SS.
  pinMode(4, OUTPUT);
  
  //Se muestra por el monitor si la comunicación se ha establecido correctamente
  //o ha habido algún tipo de error.
  if (!SD.begin(4)){
    
    Serial.println("Se ha producido un fallo al iniciar la comunicación");
    return;
  }
  Serial.println("Se ha iniciado la comunicación correctamente");
  
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    l_nombre = packetSize;
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, packetSize);
    Serial.println("archivo requerido:");

    //almacena el nombre del archivo
    for(int i=2;i<l_nombre;i++){
      if(packetBuffer[i+1]==0){
        l_nombre = i-1;
        String nombre_archivo = String("");
        for(int j=2;j<l_nombre+2;j++){
          nombre_archivo = String(nombre_archivo + packetBuffer[j]);
        }
        Serial.println(nombre_archivo);
        //Se vuelve a abrir el fichero, esta vez para leer los datos escritos.
        Archivo = SD.open(nombre_archivo);
        
        //Si el archivo se ha abierto correctamente se muestran los datos.
        if (Archivo){
          
          //Se muestra por el monitor que la información que va a aparecer es la del
          //archivo datos.txt.
          Serial.println("Información contenida en archivo");
          
          //Se implementa un bucle que recorrerá el archivo hasta que no encuentre más
          //información (Archivo.available()==FALSE).
      
          while (Archivo.available()){
            
           //Se escribe la información que ha sido leída del archivo.
           Serial.write(Archivo.read());
          }
          
          
          //Si todo ha ido bien cierra el archivo para no perder datos.
          Archivo.close();
        }//fin if archivo
        
        //En caso de que haya habido problemas abriendo datos.txt, se muestra por pantalla.
        else{
       
          Serial.println("El archivo no se abrió correctamente");
        }

      }
    }
    
    Serial.println("");
    // send a reply to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  }
  delay(10);
}

