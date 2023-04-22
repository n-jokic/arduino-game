//Potrebne Biblioteke
#include <TimerOne.h>
#include <TM1637Display.h>

//Pins

//Display Pins
const int DIO = 44;
const int CLK = 45;

//HCSR-04 Pins
const int trigPin = 46;
const int echoPin = 47;

//Touch Sensor pin
const int buttonPin = 2;

//LED Pins
const int distance1 = 49;
const int distance2 = 50;
const int distance3 = 51;
const int player1LED = 52;
const int player2LED = 53;

//Variables
char serialData;
float duration, distance;
int pos, currentPlayer = 1, data, player1Wins = 0, player2Wins = 0;
bool startArduino = false;
volatile bool trigger = false;
volatile bool buttonState = false;


TM1637Display display(CLK, DIO);

void setup() {

  //Setup
  Serial.begin(9600);
  display.setBrightness(7);

  //Button Pin
  pinMode(buttonPin, INPUT);
  //LED Pins
  pinMode(distance1, OUTPUT);
  pinMode(distance2, OUTPUT);
  pinMode(distance3, OUTPUT);
  pinMode(player1LED, OUTPUT);
  pinMode(player2LED, OUTPUT);

  //HC-SR04 Pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Initial setup svih LE dioda
  digitalWrite(distance1,LOW);
  digitalWrite(distance2,LOW);
  digitalWrite(distance3,LOW);
  digitalWrite(player1LED,LOW);
  digitalWrite(player2LED,LOW);

  //Interrupts
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(timerIsr);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonFunction, RISING);

  //Podesavanja displeja i dioda
  playerLEDs(currentPlayer);
  updateDisplay();

}

void updateDisplay(){
  
  //Ova funkcija se koristi za update-ovanje displeja
  //Update je potreban na samom pocetku programa da se resetuje display, kao i svaki put
  //kada neko od igraca dobije partiju
  
  //Version 1
  //Ova verzija broji normalno do 9 poena, posle toga resetuje taj broj poena na 0
  /*
  if(player1Wins == 10) player1Wins = 0;
  if(player2Wins == 10) player2Wins = 0;
  int prikaz = 1000 + player1Wins*100 + 20 + player2Wins;
  display.showNumberDec(prikaz);
  */


  //Version 2
  //Ova verzija broji normalno do 9 poena, posle toga prikazuje rezultat iz 4 displeja u sledecnoj sekvenci:
  //1 - sto oznacava prvog igraca
  //A - sto oznacava njegov broj poena
  //2 - sto oznacava drugog igraca
  //B - sto oznacava broj poena drugog igraca
  
  if(player1Wins < 10 && player2Wins < 10){
      int prikaz = 1000 + player1Wins*100 + 20 + player2Wins;
      display.showNumberDec(prikaz);
    }
    else{
            display.showNumberDec(1);
            delay(500);
            display.showNumberDec(player1Wins);
            delay(500);
            display.showNumberDec(2);
            delay(500);
            display.showNumberDec(player2Wins);
            delay(500);
            display.showNumberDec(0, true);   
      } 
  }

void buttonFunction(){
  
  //Ovo je funkcija koja se poziva kad se aktivira interrupt povezan sa digital pinom za dugme/touch senzor
    buttonState = true;
    
  }

void timerIsr(){
  
    //Funkcija koju pali timer preko interrupta
    //Nju koristimo za akviziciju podataka na svakih 100ms
    trigger = true;
    
  }

void playerLEDs(int cPlayer){
  
    //Ova funkcija upravlja LED indikatorima za trenutnog igraca
    if(cPlayer == 1){
        digitalWrite(player1LED, HIGH);
        digitalWrite(player2LED, LOW);
      }
      else if(cPlayer == 2){
          digitalWrite(player1LED, LOW);
          digitalWrite(player2LED, HIGH);
        }
        else{
            //Ovo ovde je ostavljeno kao mogucnost za prosirenje ili handleovanje neke greske, kada nijedan igrac nije na potezu
            digitalWrite(player1LED, LOW);
            digitalWrite(player2LED, LOW);
          }
          
  }

void distanceLEDs(int positionNumber){
  
    //Ova funkcija upravlja distance LE diodama koje nam sluze kao indikator za lakse koriscenje igre
    //0-10cm upaljena samo prva
    //10-20cm upaljene prve dve
    //20-30cm upaljene sve tri
    //>30cm sve ugasene
    
    if(pos == 0){
        digitalWrite(distance1, HIGH);
        digitalWrite(distance2, LOW);
        digitalWrite(distance3, LOW);
      }
      else if(pos == 1){
          digitalWrite(distance1, HIGH);
          digitalWrite(distance2, HIGH);
          digitalWrite(distance3, LOW);
        }
        else if(pos == 2){
            digitalWrite(distance1, HIGH);
            digitalWrite(distance2, HIGH);
            digitalWrite(distance3, HIGH);
          }
          else{
              digitalWrite(distance1, LOW);
              digitalWrite(distance2, LOW);
              digitalWrite(distance3, LOW);
            }
            
  }

int getDistance(){
  
    //Ovaj deo koda se odnosi na prikupljanje i obradu podataka sa HC-SR04 ultrazvucnog senzora
    //Saljemo pulse na trigger pin koji "aktivira" senzor
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    //Kao povratnu informaciju dobijamo vreme koje je potrebno da ultrazvucni signal ode do mete i da se vrati
    //Znajuci brzinu zvuka mozemo da izracunamo udaljenost objekta
    duration = pulseIn(echoPin, HIGH);
    distance = (duration*.0343)/2;
    
    //Udaljenost konvertujemo u indikator koji lakse mozemo da koristimo u ostatku programa
    if(distance <= 10) pos = 0;
    if(distance > 10 && distance <= 20) pos = 1;
    if(distance > 20 && distance <= 30) pos = 2;
    if(distance >= 30) pos = 3;
    
    //Serial.println(distance);
    distanceLEDs(pos);
    return pos;
    
  }

void serialEvent(){
  
    //Ova funkcija handle-uje svu komunikaciju sa Python-om
    //Protokol je osmisljen ovako:
    //Pre pocetka akvizicije sa seznora i pocetkom ostale komunikacije, ceka se da se pritisne start dugme koje detektujemo kao '0'
    //Ako na port stigne '1' to znaci da je pobedio prvi igrac
    //Ako na port stigne '2' to znaci da je pobedio drugi igrac
    //Ako na port stigne '3' to znaci da je na potezu igrac 1
    //Ako na port stigne '4' to znaci da je na potezu igrac 2

    if(Serial.available() > 0){
      
      serialData = Serial.read();
      //Serial.print(serialData);

      if(startArduino){
      if(serialData == '1'){
          player1Wins++;
          updateDisplay();
        }
        else if(serialData == '2'){
            player2Wins++;
            updateDisplay();
          }
          else if(serialData == '3'){
              currentPlayer = 1;
            }
            else if(serialData == '4'){
                currentPlayer = 2;
              }
      }
      else{
          if(serialData == '0'){
              startArduino = true;
            }
        }
      
    }
  }
  
void loop() {
  
  //Izvrsavanje pocinje kad se pritisne start dugme na GUI-u
  if(startArduino){
    //Trigger se prebaci na true svakih 100ms koriscenjem timer1 interrupt-a, kada vrsimo akviziciju
    if(trigger){
        data = getDistance();
        trigger = false;
      }
    //Ako je dugme pritisnuto, proveravamo da li je prepreka udaljena manje od 30cm i ako jeste, saljemo tu informaciju python-u
    if(buttonState){
        if(pos < 3) Serial.println(pos);
        buttonState = false;
      }
    //Radimo update LE dioda za trenutnog igraca
    playerLEDs(currentPlayer);

  }
}
