//invoegen bibliotheken
#include "SerialCommand.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"

//instellen basiswaarden
#define SerialPort Serial
#define BluetoothPort Serial
#define Baudrate 9600
#define Ain1 3
#define Ain2 11
#define Bin1 5
#define Bin2 6
#define ButtonStart PD2
SerialCommand sCmd(BluetoothPort);

//gebruikte globale parameters
unsigned long previous;
unsigned long calculationTime;
int sensor[] = {22, 19, PC0, PC1, PC2, PC3, PC4, PC5};
long normalised[8];
float position;
float error;
float Iregeling;
float lastError;
float output;
int powerLeft;
int powerRight;
bool start;
int ledcyclus;
bool buttonStart;
bool lastButtonStart;

//parameters opgeslagen in EEPROM
struct param_t
{
  unsigned long cycleTime;
  int calibrateBlack[8];
  int calibrateWhite[8];
  float kp;
  float ki;
  float kd;
  float diff;
  int power;
} params;

void setup()
{
  //clock juist zetten
  CLKPR = _BV(CLKPCE);
  CLKPR = 0;

  //instellen setup
  SerialPort.begin(Baudrate);
  BluetoothPort.begin(Baudrate);
  start = false;
  pinMode(Ain1, OUTPUT);
  pinMode(Ain2, OUTPUT);
  pinMode(Bin1, OUTPUT);
  pinMode(Bin2, OUTPUT);
  pinMode(ButtonStart, INPUT_PULLUP);
  Iregeling = 0;

  //De verschillende commandos
  sCmd.addCommand("set", onSet);
  sCmd.addCommand("cal", onCalibrate);
  sCmd.addCommand("debug", onDebug);
  sCmd.addCommand("start", onStart);
  sCmd.addCommand("reset", onReset);
  sCmd.setDefaultHandler(onUnknownCommand);

  EEPROM_readAnything(0, params);

  SerialPort.println("ready");
}

void loop()
{
  //bekijken van de startdrukknop
  buttonStart = digitalRead(ButtonStart);
  if (buttonStart != lastButtonStart) {
    if (buttonStart == LOW and start == false)
    {
      start = true;
    }
    else if (buttonStart == LOW and start == true) {
      start = false;
      analogWrite(Ain1, 0);
      analogWrite(Ain2, 0);
      analogWrite(Bin1, 0);
      analogWrite(Bin2, 0);
    }
    lastButtonStart = buttonStart;
  }

  //bekijken of er commando's gestuurd worden
  sCmd.readSerial();

  //cyclisch programma
  unsigned long current = micros();
  if (current - previous >= params.cycleTime)
  {
    previous = current;

    //uitlezen en normaliseren sensoren
    for (int i = 0; i < 8; i++)
    {
      long value = analogRead(sensor[i]);
      normalised[i] = map(value, params.calibrateBlack[i], params.calibrateWhite[i], 1000, 0);
    }

    //hoogste sensor bepalen
    int index = 0;
    for (int i = 1; i < 8; i++) {
      if (normalised[i] > normalised[index]) {
        index = i;
      }
    }

    //positie bepalen
    if (index == 0) {
      position = -35;
    }
    else if (index == 5) {
      position = 35;
    }
    else
    {
      float top = 0;
      float bottem = 0;
      float sensorPosition[] = { -35, -25, -15, -5, 5, 15, 25, 35};
      for (int i = 0; i < 8; i++)
      {
        top += normalised[i] * sensorPosition[i];
      }
      for (int i = 0; i < 8; i++)
      {
        bottem += normalised [i];
      }
      position = top / bottem;
    }

    //motorsturing bepalen
    if (start == true) {

      if (ledcyclus >= 1000) {
        digitalWrite(11, !digitalRead(11));
        ledcyclus = 0;
      }

      ledcyclus++;

      //berekenen bijsturing kp
      error = -position;
      output = error * params.kp;

      //berekenen bijsturing ki
      Iregeling += params.ki * error;
      Iregeling = constrain(Iregeling, -510, 510);
      output +=  Iregeling;

      //berekenen bijsturing kd
      output += params.kd * (error - lastError);
      lastError = error;

      //begrenzen output
      output = constrain(output, -510, 510);

      //omzetten bijsturing
      if (output >= 0)
      {
        powerLeft = constrain(params.power + params.diff * output, -255, 255); //berekeningen uit constraint halen?
        powerRight = constrain(powerLeft - output, -255, 255);
        powerLeft = powerRight + output;
      }
      else
      {
        powerRight = constrain(params.power - params.diff * output, -255, 255);
        powerLeft = constrain(powerRight + output, -255, 255);
        powerRight = powerLeft - output;
      }

      //wegschrijven bijsturing
      if (powerRight > 0)
      {
        analogWrite(Ain1, powerRight);
        analogWrite(Ain2, 0);
      }
      else
      {
        powerRight = 255 - powerRight; //abs() gebruiken?
        analogWrite(Ain1, 0);
        analogWrite(Ain2, powerRight);
      }

      if (powerLeft > 0)
      {
        analogWrite(Bin1, powerLeft);
        analogWrite(Bin2, 0);
      }
      else
      {
        powerLeft = 255 - powerLeft; //abs() gebruiken?
        analogWrite(Bin1, 0);
        analogWrite(Bin2, powerLeft);
      }
    }
  }

  //berekenen hoelang het duurd om cyclus te doorlopen
  unsigned long difference = micros() - current;
  if (difference > calculationTime) calculationTime = difference;

}

//bij ongekend commando via bluetooth
void onUnknownCommand(char *command)
{
  BluetoothPort.print("unknown command: \"");
  BluetoothPort.print(command);
  BluetoothPort.println("\"");
}

//bij een set commando
void onSet()
{
  char* param = sCmd.next();
  char* value = sCmd.next();

  //set cycle
  if (strcmp(param, "cycle") == 0)
  {
    long newCycleTime = atol(value);
    float ratio = ((float) newCycleTime) / ((float) params.cycleTime);

    params.ki *= ratio;
    params.kd /= ratio;

    params.cycleTime = newCycleTime;
  }
  //set kp
  else if (strcmp(param, "kp") == 0)
  {
    params.kp = atof(value);
  }
  //set ki
  else if (strcmp(param, "ki") == 0)
  {
    float cycleTimeInSec = ((float) params.cycleTime) / 1000000;
    params.ki = atof(value) * cycleTimeInSec;
  }
  //set kd
  else if (strcmp(param, "kd") == 0)
  {
    float cycleTimeInSec = ((float) params.cycleTime) / 1000000;
    params.kd = atof(value) / cycleTimeInSec;
  }
  //set diff
  else if (strcmp(param, "diff") == 0)
  {
    params.diff = atof(value);
  }
  //set power
  else if (strcmp(param, "power") == 0)
  {
    params.power = atol(value);
  }
  EEPROM_writeAnything(0, params);
}

//bij een cal commando
void onCalibrate()
{
  char* param = sCmd.next();

  //cal black
  if (strcmp(param, "black") == 0)
  {
    BluetoothPort.print("start calibrating black... ");
    for (int i = 0; i < 8; i++) params.calibrateBlack[i] = analogRead(sensor[i]);
    BluetoothPort.println("done");
  }
  //cal white
  else if (strcmp(param, "white") == 0)
  {
    BluetoothPort.print("start calibrating white... ");
    for (int i = 0; i < 8; i++) params.calibrateWhite[i] = analogRead(sensor[i]);
    BluetoothPort.println("done");
  }
  EEPROM_writeAnything(0, params);
}

//bij een debug commando
void onDebug()
{
  BluetoothPort.print("cycle time: ");
  BluetoothPort.println(params.cycleTime);

  BluetoothPort.print("kp: ");
  BluetoothPort.println(params.kp);

  float cycleTimeInSec = ((float) params.cycleTime) / 1000000;
  float ki = params.ki / cycleTimeInSec;
  BluetoothPort.print("ki: ");
  BluetoothPort.println(ki);

  float kd = params.kd * cycleTimeInSec;
  BluetoothPort.print("kd: ");
  BluetoothPort.println(kd);

  BluetoothPort.print("diff: ");
  BluetoothPort.println(params.diff);

  BluetoothPort.print("power: ");
  BluetoothPort.println(params.power);

  BluetoothPort.print("calculation time: ");
  BluetoothPort.println(calculationTime);
  calculationTime = 0;

  BluetoothPort.print("calibration white: ");
  for (int i = 0; i < 8; i++) {
    BluetoothPort.print(params.calibrateWhite[i]);
    BluetoothPort.print(". ");
  }
  BluetoothPort.println("");

  BluetoothPort.print("calibration black: ");
  for (int i = 0; i < 8; i++) {
    BluetoothPort.print(params.calibrateBlack[i]);
    BluetoothPort.print(". ");
  }
  BluetoothPort.println("");

  BluetoothPort.print("sensors: ");
  for (int i = 0; i < 8; i++) {
    BluetoothPort.print(normalised[i]);
    BluetoothPort.print(". ");
  }
  BluetoothPort.println("");

  BluetoothPort.print("position: ");
  BluetoothPort.println(position);
}

//bij een start commando
void onStart()
{
  if (start == true) {
    start = false;
    analogWrite(Ain1, 0);
    analogWrite(Ain2, 0);
    analogWrite(Bin1, 0);
    analogWrite(Bin2, 0);
    BluetoothPort.println("Stopped");
  }
  else
  {
    start = true;
    lastError = 0;
    Iregeling = 0;
    BluetoothPort.println("Started");
  }
}

//bij een reset commando
void onReset()
{
  params.kp = 0;
  params.ki = 0;
  Iregeling = 0;
  params.kd = 0;
  lastError = 0;
  params.diff = 0.5;
  params.power = 150;
  params.cycleTime = 1500;

  EEPROM_writeAnything(0, params);

  BluetoothPort.println("Reset done");

}
