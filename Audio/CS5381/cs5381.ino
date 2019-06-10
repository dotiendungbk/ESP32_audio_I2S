#include <Arduino.h>


// Master mode
int led =12;
int RST=14;
int MS=27;

int LRCK=26;
int SCLK=25;
int MCLK=33;
int SDO=32;// input data in esp32 from cs5831
uint32_t tmpDataRight=0;
uint32_t tmpDataLeft=0;
uint32_t tmp=0;
#define LIMIT_RIGHT 10000
uint32_t DataRight[LIMIT_RIGHT];
bool UpdateRight=false;
int iR=0;

#define LIMIT_LEFT 10000
uint32_t DataLeft[LIMIT_LEFT];
bool UpdateLeft=false;
int iL=0;


struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool updated;
    int iRight;
    int iLeft;
    
};

Button button1 = {25, 0, false,0,0};

bool hold=false;

void IRAM_ATTR isr(void* arg) {
    Button* s = static_cast<Button*>(arg);
    s->numberKeyPresses += 1;
   
    //if(s->updated==false){
      
    
            if(digitalRead(LRCK)==HIGH){
                s->iLeft=0;
        
                if(s->iRight<24){
                                tmp<<=1;
                                if(digitalRead(SDO)==HIGH) tmp+=1;
                    
                                s->iRight+=1;
                                if(s->iRight>23) {
                                                  tmpDataRight=tmp;
                                                  tmp=0;
                                                  if(UpdateRight==false){
                                                    
   
                                                                        DataRight[iR]=tmpDataRight;
                                                                        iR++;
                                                                        if(iR>=LIMIT_RIGHT) UpdateRight=true;
                                                  }
                                                  
                                }
                 }
              
            }
            else{
                s->iRight=0;
                if(s->iLeft<24){
                                tmp<<=1;
                                if(digitalRead(SDO)==HIGH) tmp+=1;
                                s->iLeft+=1;
                                if(s->iLeft>23){
                                                tmpDataLeft=tmp;
                                                tmp=0;
                                                if(UpdateLeft==false){
                                                    
   
                                                                        DataLeft[iL]=tmpDataLeft;
                                                                        iL++;
                                                                        if(iL>=LIMIT_LEFT) UpdateLeft=true;
                                                  }
                                                //s->updated=true;
                                                
                                }
                }
                
            }
   
}

//void IRAM_ATTR isr() {
    
//}

void setup() {
    Serial.begin(2000000);
    pinMode(led, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(MS, OUTPUT);
    pinMode(button1.PIN, INPUT_PULLUP);
    pinMode(LRCK,INPUT_PULLUP);
    pinMode(SDO,INPUT_PULLUP);
    pinMode(MCLK,OUTPUT);

    digitalWrite(MCLK,LOW);
    digitalWrite(led, HIGH); 
    digitalWrite(RST, HIGH); 
    digitalWrite(MS, HIGH); 
    attachInterruptArg(button1.PIN, isr, &button1, FALLING);
   
}

void loop() {
//    if (button1.updated)
//    {
//        Serial.print(tmpDataLeft);Serial.print(" "); Serial.println(tmpDataRight);
//        button1.updated = false;
//    }

    if((UpdateRight==true)&&(UpdateLeft==true))
    {
           digitalWrite(led,LOW);
           digitalWrite(RST,LOW);
           
          for(int i=4000;i<LIMIT_RIGHT;i++){
                                  Serial.print(DataLeft[i]);
                                  Serial.print(" ");
                                  Serial.println(DataRight[i]);delay(10);
          
          }
          UpdateRight=false;
          UpdateLeft=false;
          iR=0;
          iL=0;

          digitalWrite(RST,HIGH);
          digitalWrite(led,HIGH);
    }
  
    digitalWrite(MCLK,LOW);
    digitalWrite(MCLK,HIGH);

    
}