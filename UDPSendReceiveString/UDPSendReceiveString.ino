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
char packetBuffer1[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
EthernetUDP Udp;
EthernetUDP Udp1;

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Udp1.begin(70);
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  if (!SD.begin(4)){    
    Serial.println("Se ha producido un fallo al iniciar la comunicación");
    return;
  }
}//fin setup

void loop() {
  int packetSize = Udp.parsePacket();
  int packetSize1 = Udp1.parsePacket();
  if(packetSize1){
    Serial.println("llego algo al puerto 70");
    Udp1.read(packetBuffer1, packetSize1);
    int opcode = (int)packetBuffer1[1];
    switch (opcode){
      case 3:
       {
        int paquete = packetBuffer1[3];
        Serial.println(paquete);
        envia_ack(&Udp,&Udp1,paquete);
       }
        break;
     case 4:
       {
        int orden = packetBuffer1[3];
        envia_archivo(&Archivo,&Udp,&Udp1, orden+1);
       }
        break;
     case 5:
       //Error
     break;
    }
  }
  if (packetSize) {
    Serial.println("llego algo al puerto 69");
    Udp.read(packetBuffer, packetSize);
    int opcode = (int)packetBuffer[1];
    switch (opcode){
      case 1:
      {
        String nom_arch=get_nombre_archivo(&packetBuffer[0],packetSize);
        Serial.println(nom_arch);
        Archivo = SD.open(nom_arch);
        if(Archivo.available()<512){
          envia_resto(Archivo.available(),&Archivo, &Udp,&Udp1,1);
        }
        else{
          envia_bloque(&Archivo,&Udp,&Udp1,1);
        }
      }
        break;
        case 2:
        {
          //String nombre=get_nombre_archivo(&packetBuffer[0],packetSize);
          //Serial.println(nombre);
          //Contenido = SD.open(nombre);
          envia_ack(&Udp,&Udp1,0);
        }
        break;
      
    }//fin switch
  }//fin if packetSize
  delay(10);
}//fin loop

void envia_ack(EthernetUDP *udp,EthernetUDP *udp1,int orden){
  char ack[4];
  ack[0]=0;
  ack[1]=4;
  ack[2]=0;
  ack[3]=orden;
  if(orden==0){
    udp1->beginPacket(udp->remoteIP(), udp->remotePort());
    udp1->write(ack,4);
    udp1->endPacket();  
  }else{
    udp1->beginPacket(udp1->remoteIP(), udp1->remotePort());
    udp1->write(ack,4);
    udp1->endPacket(); 
  }
  
}

void envia_archivo(File *archivo,EthernetUDP *udp,EthernetUDP *udp1, int aux){
  if(archivo->available()<512){
          envia_resto(archivo->available(),archivo, udp,udp1,aux);
        }
        else{
          envia_bloque(archivo,udp,udp1,aux);
        }
}

void envia_bloque(File *archivo,EthernetUDP *udp,EthernetUDP *udp1, int aux){
  char bloq[516];
  bloq[0]=0;
  bloq[1]=3;
  bloq[2]=0;
  bloq[3]=aux;
  for(int i=4;i<516;i++){
    bloq[i] = archivo->read();   
  }
  if(aux==1){
    udp1->beginPacket(udp->remoteIP(), udp->remotePort());
    udp1->write(bloq,516);
    udp1->endPacket();  
  }else{
    udp1->beginPacket(udp1->remoteIP(), udp1->remotePort());
    udp1->write(bloq,516);
    udp1->endPacket(); 
  }
}

void envia_resto(int total,File *archivo,EthernetUDP *udp,EthernetUDP *udp1,int num_block){
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
      if(num_block==1){
        udp1->beginPacket(udp->remoteIP(), udp->remotePort());
        udp1->write(paquete,total);
        udp1->endPacket();  
      }else{
        udp1->beginPacket(udp1->remoteIP(), udp1->remotePort());
        udp1->write(paquete,total);
        udp1->endPacket(); 
      }  
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
