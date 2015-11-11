//  
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
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
EthernetUDP Udp;

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  if (!SD.begin(4)){    
    Serial.println("Se ha producido un fallo al iniciar la comunicación");
    return;
  }
}//fin setup

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Udp.read(packetBuffer, packetSize);
    int opcode = (int)packetBuffer[1];
    switch(opcode){
      //receptor
      case 1:
      {
        String nom_arch=get_nombre_archivo(&packetBuffer[0],packetSize);
        Serial.println(nom_arch);
        Archivo = SD.open(nom_arch);
        if(Archivo.available()<512){
          envia_resto(Archivo.available(),&Archivo, &Udp,1);
        }
        else{
          envia_bloque(&Archivo,&Udp,1);
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
       {
        int orden = packetBuffer[3];
        Serial.println(orden); 
        envia_archivo(&Archivo,&Udp, orden+1);
       }
        break;
     case 5:
       //Error
     break;
    }//fin switch
  }//fin if packetSize
  delay(10);
}//fin loop

void envia_archivo(File *archivo,EthernetUDP *udp, int aux){
  if(archivo->available()<512){
          envia_resto(archivo->available(),archivo, udp,aux);
        }
        else{
          envia_bloque(archivo,udp,aux);
        }
}

void envia_bloque(File *archivo,EthernetUDP *udp, int aux){
  char bloq[516];
  bloq[0]=0;
  bloq[1]=3;
  bloq[2]=0;
  bloq[3]=aux;
  for(int i=4;i<516;i++){
    bloq[i] = archivo->read();   
  }
  udp->beginPacket(udp->remoteIP(), udp->remotePort());
  udp->write(bloq,516);
  udp->endPacket();  
}

void envia_resto(int total,File *archivo,EthernetUDP *udp,int num_block){
  //funcion que envia el resto del archivo y cierra Archivo.
      total = total + 4;
      char paquete[total];
      paquete[0]=0;
      paquete[1]=3;
      paquete[2]=0;
      paquete[3]=num_block;
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
  //Funcion que devuelve el nombre del archivo en RRQ y WRQ.
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

