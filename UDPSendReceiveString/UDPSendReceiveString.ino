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

  
  //Se establece como salida el pin correspondiente a SS.
  pinMode(4, OUTPUT);
  
  //Se muestra por el monitor si la comunicación se ha establecido correctamente
  //o ha habido algún tipo de error.
  if (!SD.begin(4)){
    
    Serial.println("Se ha producido un fallo al iniciar la comunicación");
    return;
  }
}//fin setup

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {

    IPAddress remote = Udp.remoteIP();
 
    // read the packet into packetBufffer
    Udp.read(packetBuffer, packetSize);

    int opcode = (int)packetBuffer[1];
    switch(opcode){
    //opcode 01
      case 1:
      {
        String nom_arch=get_nombre_archivo(&packetBuffer[0],packetSize);
        Serial.println(nom_arch);
        Archivo = SD.open(nom_arch);
        int tamano = Archivo.size();
        Serial.println(tamano);
        if(tamano<512){
          //envio archivo completo;
          resto(tamano,&Archivo, &Udp);
        }
        else{
          //divido archivo y envio paquetes
          int q_paquete = (tamano/512);
          int orden = 1;
          for(int i=0;i<q_paquete;i++){
            char paquete[512];
            paquete[0]=0;
            paquete[1]=3;
            paquete[2]=0;
            paquete[3]=orden;
            int aux=4;
            while (Archivo.available() and aux<516){ 
              paquete[aux]= Archivo.read();
              aux++;
            }
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write(paquete,516);
            Udp.endPacket();  
            orden++;
          }
        }
      }
        break;
      case 2:
       //write request
        break;
     case 3:
       //data
        break;
     case 4:
       //acknowledgment
        break;
     case 5:
       //Error
     break;
    }
  }//fin if packetSize
  delay(10);
}//fin loop

void resto(int total,File *archivo,EthernetUDP *udp){
      total = total + 4;
      Serial.println(total);
      char paquete[total];
      paquete[0]=0;
      paquete[1]=3;
      paquete[2]=0;
      paquete[3]=1;
      int aux = 4;
      while (archivo->available()){ 
        paquete[aux]= archivo->read();
        aux+=1;
      }
      archivo->close();
      udp->beginPacket(udp->remoteIP(), udp->remotePort());
      udp->write(paquete,total);
      udp->endPacket();  
}


String get_nombre_archivo(char *packetBuffer, int l_nombre){
  for(int i=2;i<l_nombre;i++){
      if(packetBuffer[i+1]==0){
        l_nombre = i-1;
        String nombre_archivo = String("");
        for(int j=2;j<l_nombre+2;j++){
          nombre_archivo = String(nombre_archivo + packetBuffer[j]);
        }
        return nombre_archivo;
      }
  }
}//fin get_nombre_archivo

