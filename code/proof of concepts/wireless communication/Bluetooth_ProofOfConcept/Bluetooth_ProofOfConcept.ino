int LED = 8;
String command;
String command1;
String command2;
String command3;

void setup()
{
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  Serial1.begin(9600);
  while(!Serial);
  Serial.println("Ready");
  pinMode(A0, INPUT);
}

void loop()
{
  delay(1000);
  int waarde = analogRead(A0);
  
  
  if(Serial.available())
  {
     command = Serial.readStringUntil('\r\n');             //lezen van de seriele monitor van de pc
     
        Serial1.println(command);
        if (command.startsWith("waarde?"))
    {
       Serial.println(waarde);
       Serial1.println(command); 
    }
    else
    {
      Serial.println("foute aanvraag");
      Serial1.println(command);
      digitalWrite(LED, HIGH);
    }
   } 
    
   if(Serial1.available()) 
   {
      command1 = Serial1.readStringUntil('\r\n');           //lezen van de seriele monitor van de gsm

      Serial.println(command1);

      if (command1.startsWith("waarde?"))
    {
       Serial1.println(waarde);
       Serial.println(command); 
    }
    else
    {
      Serial1.println("foute aanvraag");
      Serial.println(command);
      digitalWrite(LED, LOW);
    }
   }

   
   
  
}
