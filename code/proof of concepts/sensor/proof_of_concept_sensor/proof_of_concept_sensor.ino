//int led1 = 12;
//int led2 = 8;
//int pin1 = PC0;
//int pin2 = PC1;
void setup() {
  Serial.begin(9600);
}


void loop() {
  int sensorValue1 = analogRead(PC2);
  int sensorValue2 = analogRead(PC3); 
  Serial.println(sensorValue1);
  Serial.println(sensorValue2);
  //if (sensorValue1 > 600) {
  //digitalWrite(led1, HIGH); //ZWART sensor 1
//}
  //if(sensorValue1 < 600){
 // digitalWrite(led1, LOW);//WIT sensor 1
//}
 // if (sensorValue2 > 600) {
  //digitalWrite(led2, HIGH); //ZWART sensor 2
//}
 // if(sensorValue2 < 600){
 // digitalWrite(led2, LOW);//WIT sensor 2
//}
}
