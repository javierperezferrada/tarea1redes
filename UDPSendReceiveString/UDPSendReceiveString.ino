// tftp 192.168.1.10 -c get test.txt
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <SD.h>
 
File Archivo;

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
}//fin setup

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
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

//opcode 01
    int largo = get_largo_nombre(&packetBuffer[2],packetSize);
    Serial.println(largo);
    char nom_arch[largo-1];
    get_nombre_a(&nom_arch[0],&packetBuffer[2],largo);
    String nom=(String)nom_arch;
    Archivo = SD.open(nom);
    int tamano = Archivo.size();
    Serial.println(tamano);
    if(tamano<512){
      //envio archivo completo;
      int total = tamano + 4;
      char paquete[total];
      paquete[0]=0;
      paquete[1]=3;
      paquete[2]=0;
      paquete[3]=0;
      int aux = 4;
      while (Archivo.available()){ 
        paquete[aux]= Archivo.read();
        aux+=1;
      }
      Archivo.close();
      for(int i=0;i<total;i++){
        Serial.print(paquete[i]);
      }
      // send a reply to the IP address and port that sent us the packet we received
      Serial.print("Tamaño de la respuesta ");
    Serial.println(total);
    Serial.print("hacia ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());
    
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(paquete);
      Serial.println("paquete enviado");
      Udp.endPacket();
    }
    else{
      //divido archivo y envio paquete 1
    }
      
  }
  delay(10);
}//fin loop

void readRequest(){
  
}//fin readRequest
int get_largo_nombre(char *packetBuffer, int l_nombre){
  for(int i=0;i<l_nombre;i++){
      if(packetBuffer[i+1]==0){
        int largo_nombre = i+1;
        return largo_nombre;
      }
  }
}
  
void get_nombre_a(char *nombre,char *packetBuffer, int l_nombre){
  for(int i=0;i<l_nombre;i++){
        for(int j=0;j<l_nombre;j++){
          nombre[j] = packetBuffer[j];
        }
  }
}

String get_nombre_archivo(char *packetBuffer, int l_nombre){
  for(int i=2;i<l_nombre;i++){
      if(packetBuffer[i+1]==0){
        l_nombre = i-1;
        String nombre_archivo = String("");
        for(int j=2;j<l_nombre+2;j++){
          nombre_archivo = String(nombre_archivo + packetBuffer[j]);
        }
        Serial.println(nombre_archivo);
        return nombre_archivo;
      }
  }
}//fin get_nombre_archivo

