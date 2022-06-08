//Librerías:
#include <Stepper.h>
#include <UTFT.h>
#include <SPI.h>
#include <SD.h>

//Definiciones
  //Motor de 2048 pasos (2038 reales) en pines del 8 al 11:
Stepper motor(2038, 11, 10, 9, 8);
  //Pantalla RGB 480x320 en pines del 38 al 41:
UTFT pantalla(CTE32HR,38,39,40,41);
  //Estados
#define HOME 0
#define CALIB 1
#define BARRIDO 2
#define GUARDADO 3

//Variables:
  //Estado
uint8_t estado = HOME;
  //Fuentes de texto
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
 //Botones
#define BTN_1 0
#define BTN_2 1
#define BTN_3 2
//#define BTN_4 3
  //Configuracion de botones
   //Asignacion pines de entrada botones
uint8_t boton[3] = {2, 3, 4};
    //Estado de botones
uint8_t boton_estado[4];
    //Detecta flanco de subida
uint8_t flancoSubida(int btn) {
  uint8_t valorNuevo = digitalRead(boton[btn]);
  uint8_t resulta = boton_estado[btn] != valorNuevo && valorNuevo == 1;
  boton_estado[btn] = valorNuevo;
  return resulta;
};

uint8_t boton2 = 0;
uint8_t boton3 = 0;

  //Localización de datos SD y tamaño de archivo
int posicionDato = 0;
int dimArchivo = 0;
String numDato = "";

int dato1 = 0;
int dato2 = 0;
int selector = 1;

int numArchivo = 1;
bool disponible = false;
String nombreArchivo = "datos" + (String) numArchivo + ".txt";

File datos;

void setup() {
  Serial.begin(9600);
  while (!Serial){
    ;
  }
  //Botones
  pinMode(boton[BTN_1], INPUT_PULLUP);
  pinMode(boton[BTN_2], INPUT_PULLUP);
  pinMode(boton[BTN_3], INPUT_PULLUP);
//  pinMode(boton[BTN_4], INPUT_PULLUP);
  boton_estado[0] = HIGH;
  boton_estado[1] = HIGH;
  boton_estado[2] = HIGH;
//  boton_estado[3] = HIGH;

  //Entrada fotodiodo
  pinMode(A0, INPUT);
  
  //Motor:
    //Velocidad (RPM):
  motor.setSpeed(1);

  //Pantalla:
  pantalla.InitLCD();
  pantalla.setFont(BigFont);

  //Autotest
  pantalla.clrScr();
  
  autotest();
  
  delay(1500);

  pantalla.clrScr();
  pantalla.fillScr(0, 0, 0);
}


void loop() {
  switch(estado) {
    case HOME:
      if(flancoSubida(BTN_1)){
        pantalla.fillScr(0, 0, 0);
        pantalla.print("Calibracion de 0 y 100", CENTER, 20);
        pantalla.print("Quita la muestra", CENTER, 160);
        pantalla.print("y oprime el boton 2", CENTER, 180);
        estado = CALIB;
        break;
      } else{
          //Inicio
        pantalla.print("Espectrofotometro Vis", CENTER, 80);
        pantalla.print("Carlos V. ", RIGHT, 160);
        pantalla.print("Alejandra M. ", RIGHT, 175);
        pantalla.print("Jesus D. ", RIGHT, 190);
        pantalla.print("Maria M. ", RIGHT, 205);
        pantalla.print("Dariana M. ", RIGHT, 220);
        pantalla.print("Profesora:", LEFT + 10, 220);
        pantalla.print("Silvia R.", LEFT + 10, 235);
        break;
      }
      
    break;

    case CALIB:
      if(flancoSubida(BTN_1)){
        pantalla.fillScr(0, 0, 0);
        pantalla.setColor(255, 255, 255);
        pantalla.print("Barrido de muestra", CENTER, 20);
        pantalla.print("Presiona boton 2 para barrido", CENTER, 80);
        estado = BARRIDO;
        break;
      } else {
           
        if(flancoSubida(BTN_2) && boton2 == 0){
          boton2 = 1;
        }
        
        switch(boton2) {
          case 0:
          break;
          
          case 1:
            pantalla.fillScr(0, 0, 0);
            pantalla.setColor(255, 70, 0);
            pantalla.print("Calibracion de 0", CENTER, 20);
            pantalla.print("Por favor espera...", CENTER, 160);
            
            calibrarcero();
            delay(2000);
            pantalla.fillScr(0, 0, 0);
            pantalla.setColor(255, 255, 255);
            pantalla.print("Pon muestra oscura", CENTER, 160);
            pantalla.print("y oprime boton 2", CENTER, 180);
            boton2 = 2;            
          break;

          case 2:
            if(flancoSubida(BTN_2)){
              boton2 = 3;
            }
          break;
          
          case 3:
            pantalla.fillScr(0, 0, 0);
            pantalla.setColor(0, 70, 255);
            pantalla.print("Calibracion de 100", CENTER, 20);
            pantalla.print("Por favor espera...", CENTER, 160);
            
            calibrarcien();
            delay(2000);
            pantalla.fillScr(0, 0, 0);
            pantalla.setColor(255, 255, 255);
            pantalla.print("Calibracion completada :)", CENTER, 20);
            boton2 = 0;
            delay(4000);
            pantalla.fillScr(0, 0, 0);
            estado = HOME;
          break;
        }
      }
    break;

    case BARRIDO:    
      if(flancoSubida(BTN_1)){
        pantalla.fillScr(0, 0, 0);
        pantalla.print("Historial de barridos", CENTER, 20);
        estado = GUARDADO;
        break;
      } 
      if(flancoSubida(BTN_2)){
        pantalla.fillScr(0, 0, 0);
        pantalla.setColor(255, 255, 255);
        pantalla.print("Analizando", CENTER, 280);
        delay(500);

        barrido();

        pantalla.setColor(0, 0, 0);
        pantalla.fillRect(10, 270, 470, 300);
        pantalla.setColor(255, 255, 255);
        pantalla.print("Listo", CENTER, 280);
        delay(4000);
        pantalla.fillScr(0, 0, 0);
        pantalla.print("Historial de barridos", CENTER, 20);
        estado = GUARDADO;
        break;
      }
    break;

    case GUARDADO:
      if(flancoSubida(BTN_1)){
        pantalla.fillScr(0, 0, 0);
        estado = HOME;
        break;
      }
    break;
  }
}

void consultabarrido() {
  datos = SD.open("datos.txt");
  dimArchivo = datos.size();
  
  if (datos) {
      if (posicionDato >= dimArchivo) {
        posicionDato = 0;
      }
      datos.seek(posicionDato); 
      
      //Lectura de fila
      while (datos.available()) {
          
        char digito = datos.read();
        numDato = numDato + digito;
        posicionDato = datos.position();
          
        if (digito == 10)  {
          if (selector == 1) {
            dato1 = numDato.toInt();
            selector = 2;
          } else {
            dato2 = numDato.toInt();
            selector = 1;
          }
          break;
        }
        
        pantalla.setColor(255, 255, 255);
        pantalla.drawLine(215 + dato1, 40, 215 + dato2, 60);
      }
      datos.close();
  }
  delay(1000);
}





void barrido() {
  while (!disponible) {
    datos = SD.open(nombreArchivo, FILE_WRITE);
    
    if (datos && datos.size() == 0){
      disponible = true;
      for (int e = 0; e < 119; e++) {
        motor.step(1);
        int LDR = analogRead(A0);
        datos.println(LDR);
        delay(500);
      }
      datos.close();
      motor.step(-120);
    } else {
      disponible = false;
      numArchivo = numArchivo + 1;
      //numArchivoTxt = (String) numArchivo
    }
    datos.close();
  }
  disponible = false;
}

void calibrarcero() {
  datos = SD.open("cero.txt", FILE_WRITE);
  
  if (datos){
    for (int e = 0; e < 119; e++) {
      motor.step(1);
      int LDR = analogRead(A0);
      Serial.println(LDR);
      datos.println(LDR);
      delay(500);
    }
    datos.close();
    motor.step(-120);
  }
  datos.close();
}

void calibrarcien() {
  
  datos = SD.open("cien.txt", FILE_WRITE);
  
  if (datos){
    for (int e = 0; e < 119; e++) {
      motor.step(1);
      int LDR = analogRead(A0);
      Serial.println(LDR);
      datos.println(LDR);
      delay(500);
    }
    datos.close();
    motor.step(-120);
  }
  datos.close();
}
  
void autotest(){
  pantalla.fillScr(0, 0, 0);
  pantalla.setColor(255, 255, 255);
  pantalla.print("Autotest", CENTER, 80);
  pantalla.print("Espera por favor...", CENTER, 160);
  
  if (!SD.begin(53)){
    pantalla.fillScr(199, 0, 57);
    pantalla.setColor(255, 255, 255);
    pantalla.print("Error acceso memoria", CENTER, 80);
    while(1);
  }
  delay(100);
  
  datos = SD.open("datos.txt", FILE_WRITE);
  
  if (!datos){
    pantalla.fillScr(199, 0, 57);
    pantalla.setColor(255, 255, 255);
    pantalla.print("Error acceso datos", CENTER, 80);
    while(1);
  } else {
    datos.close();
  }
  datos.close();

  delay(100);
  datos = SD.open("cien.txt", FILE_WRITE);
  
  if (!datos){
    pantalla.fillScr(199, 0, 57);
    pantalla.setColor(255, 255, 255);
    pantalla.print("Error acceso datos cien", CENTER, 80);
    while(1);
  } else {
    datos.close();
  }
  datos.close();

  delay(100);
  datos = SD.open("cero.txt", FILE_WRITE);
  
  if (!datos){
    pantalla.fillScr(199, 0, 57);
    pantalla.setColor(255, 255, 255);
    pantalla.print("Error acceso datos cero", CENTER, 80);
    while(1);
  } else {
    datos.close();
  }
  datos.close();
}

void arcoiris() {
  
  for (int b = 70; b < 84; b++)  {
    pantalla.setColor(255, 0, 255-(b*3));
    pantalla.drawLine(b-30, 20, b-30, 200);
  }
  
  for (int r = 0; r < 84; r++)  {
    pantalla.setColor(255, r*3, 0);
    pantalla.drawLine(r+53, 20, r+53, 200);
  }
  
  for (int g = 1; g < 84; g++)  {
    pantalla.setColor(255-(g*3), 255, 0);
    pantalla.drawLine(g+136,20, g+136, 200);
  }

  for (int b = 1; b < 84; b++)  {
    pantalla.setColor(0, 255, b*3);
    pantalla.drawLine(b+219, 20, b+219, 200);
  }

  for (int r = 1; r < 84; r++)  {
    pantalla.setColor(0, 255-(r*3), 255);
    pantalla.drawLine(r+302, 20, r+302, 200);
  }

  for (int g = 1; g < 30; g++)  {
    pantalla.setColor(g*3, 0, 255);
    pantalla.drawLine(g+385,20, g+385, 200);
  }

    pantalla.setColor(255, 255, 255);

    //Texto estándar
  for (int n = 0; n < 5; n++) {
    pantalla.drawLine(40+(60*n), 201, 40+(60*n), 215);
  }
    pantalla.print("300", 20, 216);
    pantalla.print("367", 80, 216);
    pantalla.print("433", 140, 216);
    pantalla.print("500", 200, 216);
    pantalla.print("567", 260, 216);
    pantalla.print("633", 320, 216);
    pantalla.drawLine(345, 201, 345, 215);
    pantalla.print("700", 380, 216);
    pantalla.drawLine(415, 201, 415, 215);

}

void longitud() {

}
