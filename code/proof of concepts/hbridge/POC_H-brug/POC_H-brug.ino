
void setup() {
  pinMode (5, OUTPUT); //rechts achteruit
  pinMode (6, OUTPUT); //rechts vooruit
  pinMode (11, OUTPUT); //links achteruit
  pinMode (3, OUTPUT); //links vooruit

}

void loop() {
 
//rechts achteruit
    digitalWrite(5,HIGH);
    digitalWrite(6, LOW); 

    delay(1000);
  
//links achteruit 
    digitalWrite(11,HIGH);
    digitalWrite(3, LOW);

delay(2000);

//rechts vooruit
    digitalWrite(6,HIGH); 
    digitalWrite(5, LOW);
  
//links vooruit
    digitalWrite(3,HIGH);
    digitalWrite(11, LOW);

delay(2000);


    digitalWrite(6,LOW); 
    digitalWrite(5, LOW);
    digitalWrite(3,LOW);
    digitalWrite(11, LOW);
 
   for (int i = 50; i <= 255; i++){
    analogWrite(6,i);
   analogWrite(3,i);
    delay(50);
   }
    
