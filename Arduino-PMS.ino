//Imported libraries Biblioteke
#include <TimerOne.h>
#include <TM1637Display.h>

//LED 
const int dist1 = 49;
const int dist2 = 50;
const int dist3 = 51;
const int LEDplayer1 = 52;
const int LEDplayer2 = 53;

//Display
const int DIO = 44;
const int CLK = 45;

//HCSR-04
const int trigPin = 46;
const int echoPin = 47;

//Touch Sensor
const int buttonPin = 2;



char SData; //Serial Data
int pos, currPlayer = 1, data, player1Win = 0, player2Win = 0;
float dur, dist; //duration & distance
bool startAr = false; //Arduino startAriong signal
volatile bool trigger = false;
volatile bool buttonState = false;




TM1637Display display(CLK, DIO);

void setup() {

  //Init
  Serial.begin(9600);
  display.setBrightness(6);

   //LED's
  pinMode(dist1, OUTPUT);
  pinMode(dist2, OUTPUT);
  pinMode(dist3, OUTPUT);
  pinMode(LEDplayer1, OUTPUT);
  pinMode(LEDplayer2, OUTPUT);

  //Default LED values
  digitalWrite(dist1,LOW);
  digitalWrite(dist2,LOW);
  digitalWrite(dist3,LOW);
  digitalWrite(LEDplayer1,LOW);
  digitalWrite(LEDplayer2,LOW);
  
  //Button
  pinMode(buttonPin, INPUT);

  //HC-SR04
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Interrupts
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(timerIsr);
  attachInterrupt(digitalPinToInterrupt(buttonPin), button, RISING);

  players(currPlayer);
  updateDisp();

}

void updateDisp(){
  
  //funkcija koja se poziva radi restartovanja, inicijalizacij ili promene stanja na ekranu
  
  if(player1Win < 10 && player2Win < 10){
      display.showNumberDec(1000 + player1Win*100 + 20 + player2Win); // ---> format 1A2B
    }
    else{ //---> partija pocinje ispocetka nakon sto neko stigne to 10 pobeda Alternativno moze da se prikazuje simbol po simbol.
        player1Win = 0;
        player2Win = 0;
      } 
  }

void button(){ //---> changes button state
    buttonState = true; 
  }

void timerIsr(){ //---> timer za potrebe akvizicije podataka
  
    trigger = true;
    
  }

void players(int Player){//---> plaig/gasi diode igraca
  
    if(Player == 1){
        digitalWrite(LEDplayer1, HIGH);
        digitalWrite(LEDplayer2, LOW);
      }
      
    if(Player == 2){
          digitalWrite(LEDplayer1, LOW);
          digitalWrite(LEDplayer2, HIGH);
        }
          
  }

void distLEDs(int positionNumber){ // ---> indikator rastojanja od senzora
  
    if(pos == 0){ //---> dist = 0-10cm
        digitalWrite(dist1, HIGH);
        digitalWrite(dist2, LOW);
        digitalWrite(dist3, LOW);
      }
      
    if(pos == 1){ //---> dist = 10-20cm
          digitalWrite(dist1, HIGH);
          digitalWrite(dist2, HIGH);
          digitalWrite(dist3, LOW);
        }
        
     if(pos == 2){ //---> dist = 20-30cm
            digitalWrite(dist1, HIGH);
            digitalWrite(dist2, HIGH);
            digitalWrite(dist3, HIGH);
          }
          
      if(pos = 3) { //---> dist >= 30cm
              digitalWrite(dist1, LOW);
              digitalWrite(dist2, LOW);
              digitalWrite(dist3, LOW);
            }
            
  }

int getdist(){ //Obrada podataka sa HC-SR04 ultrazvucnog senzora
    
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);     //---> puls na trigger pinu aktivira senzor
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    dur = pulseIn(echoPin, HIGH); //---> trajanje impulsa
    dist = (dur*.0340)/2; //---> rastojanje brzina zvuka ~ 340 m/s, rezultat se deli sa dva jer dur*0.034 predstavlja DVA rastojanja 
    
    code(dist);
    
    distLEDs(pos);
    return pos;
    
  }

void code(int distance) { //kodiranje rastojanja u jedan pos, ovako se lako moze ismeniti vrednost rastojanja za potrebe detekcije
    if (distance > 30) pos =3;
    if (distance <= 30) pos = 2;
    if (distance <= 20) pos = 1;
    if (distance <= 10) pos = 0;
}

void serialEvent(){ //--> komunikacija sa phytonom
  

    if(Serial.available() > 0){ //--> provera dostupnosti
      
      SData = Serial.read();

      if(startAr){//--> cekanje na arduino
        
      }
      if(SData == '1'){ 
          player1Win++;
          updateDisp();
        }
        
      if(SData == '2'){
          player2Win++;
          updateDisp();
          }
          
       if(SData == '3'){
           currPlayer = 1;
          }
          
       if(SData == '4'){
          currPlayer = 2;
         }
      }
      else{
          if(SData == '0'){
              startAr = true;
            }
        }
      
    }
  
  
void loop() {
  
  if(startAr){
    
    if(trigger){ //--> svakih 100ms se menja stange trigera
        data = getdist();
        trigger = false;
      }
    
    if(buttonState){
        if(pos < 3) Serial.println(pos);
        buttonState = false;
      }
    
    players(currPlayer);

  }
}
