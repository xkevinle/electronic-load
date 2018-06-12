#include <LiquidCrystal.h>
#include <Keypad.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const byte numRows= 4;
const byte numCols= 4;

char keymap[numRows][numCols]= 
{
{'1', '2', '3', 'A'}, 
{'4', '5', '6', 'B'}, 
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};

byte rowPins[numRows] = {A2,A3,A4,A5};
byte colPins[numCols]= {6,7,8,9};

Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

int mode = 0;

float input_voltage = 0.0;
float temp=0.0;
float r1=100000.0;
float r2=10000.0;

float count = 0.0;
float current = 0.0;
float resistance = 0.0;
float power = 0.0;
float control = 0.0;
float output = 0.0;
//char key = 'Z';
int temp1 = 0;
int temp2 = 0;

unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {
 
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(10, OUTPUT);
  pinMode(13, OUTPUT);
}
 
void loop() {
  
  // Current Sensor code
  for(int i = 0; i < 200; i++)
  {
    current = current + (((0.0049*analogRead(A0))-2.5)/0.066); //30A
    //current = current + (((0.0049*analogRead(A0))-2.5)/0.185); //5A
  }
  current = current/200;
  
  // Current Sensor calibration
  //current = 0.9478*current - 0.0584;
  current = 0.9478*current - 0.0484;
  //current = 0.9816*current + 0.1429;
  //current = 0.9889*current + 0.1314;

  if( current < 0)
  {
    current = 0.001;
  }

  // Voltage Sensor code
  for(int i = 0; i < 100; i++)
  {
      temp = temp + ((analogRead(A1) * 5.0) / 1024.0);
  }
  temp = temp/100;
  input_voltage = temp / (r2/(r1+r2));
  input_voltage = 0.9907*input_voltage + 0.0726; // Calibrated equation from Excel
  
  if(input_voltage == 0.0726)
  {
    input_voltage = 0;
  }

  
  if(input_voltage<4)
  {
    mode=0;
    output=0;
    lcd.setCursor(0,0);
    lcd.print("Voltage too low ");
    lcd.setCursor(0,1);
    lcd.print("                ");
  }
  else if(input_voltage>31)
  {
    mode=0;
    output=0;
    lcd.setCursor(0,0);
    lcd.print("Voltage too high");
    lcd.setCursor(0,1);
    lcd.print("                ");
  }
  else if(mode==0)
  {
    lcd.setCursor(0,0);
    lcd.print("Select mode     ");
    lcd.setCursor(0,1);
    lcd.print("A:CP B:CR C:CC  ");
  }
  
  char key = myKeypad.getKey();
  
  lcd.setCursor(0,0);
  
  // display and set mode
  if(key=='A')
  {
    count=1;
    mode=1;
  }
  else if(key=='B')
  {
    count=100;
    mode=2;
  }
  else if(key=='C')
  {
    count=0.5;
    mode=3;
  }
  else if(key=='D')
  {
    mode=0;
  }

  //increment or decrement user input by 1s or 10s
  if(mode==1)
  {
    if (count > 0 && count < 101)
    {
      if(key=='2')
      {
        count++;
      }
      else if(key=='1')
      {
        count--;
      }
      else if(key=='4')
      {
        count = count - 10;
      }
      else if(key=='5')
      {
        count = count + 10;
      }
    }
    if (count<1)
    {
      count = 1;
    }
    if (count>100)
    {
      count = 100;
    }
  }
  
  if(mode==2)
  {
    if (count > 0 && count < 301)
    {
      if(key=='2')
      {
        count++;
      }
      else if(key=='1')
      {
        count--;
      }
      else if(key=='4')
      {
        count = count - 10;
      }
      else if(key=='5')
      {
        count = count + 10;
      }
    }
    if (count<1)
    {
      count = 1;
    }
    if (count>300)
    {
      count = 300;
    }
  }
  
  if(mode==3)
  {
    if (count > 0 && count < 21)
    {
      if(key=='2')
      {
        count = count + 0.1;
      }
      else if(key=='1')
      {
        count = count - 0.1;
      }
      else if(key=='4')
      {
        count--;
      }
      else if(key=='5')
      {
        count++;
      }
    }
    if (count<0.1)
    {
      count = 0.1;
    }
    if (count>20)
    {
      count = 20;
    }
  }

  //lcd.setCursor(4,0);
  if(mode==1)
  {
    lcd.print("CP: ");
    lcd.print(count,0);
    lcd.print("W ");
  }
  if(mode==2)
  {
    lcd.print("CR: ");
    lcd.print(count,0);
    lcd.print((char)244);
    lcd.print(" ");
  }
  if(mode==3)
  {
    lcd.print("CC: ");
    lcd.print(count,1);
    lcd.print("A ");
  }

  if(input_voltage>4.9&&input_voltage<30.1)
  {
    //control calculations
    if(mode==1)
    {
      // constant power calculation
      control = count/input_voltage;
      if(current<control&&output<50)
      {
        
        output = output + 0.05;
      }
      else if(current>control&&output>0)
      {
        output = output - 0.05;
      }
    }
    if(mode==2)
    {
      // constant resistance calculation
      control = input_voltage/count;
      if(current<control&&output<50)
      {
        
        output = output + 0.05;
      }
      else if(current>control&&output>0)
      {
        output = output - 0.05;
      }
    }
    if(mode==3)
    {
      // constant current calculation
      control = count;
      if(current<control&&output<50)
      {
        
        output = output + 0.05;
      }
      else if(current>control&&output>0)
      {
        output = output - 0.05;
      }
    }
  }
  
  resistance = input_voltage/current;
  power = input_voltage*current;
 
  
  // update LCD display every second
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    if(mode==1)
    {
      lcd.print(input_voltage,1);
      lcd.print("V      ");
      lcd.setCursor(0,1);
      lcd.print(current,2);
      lcd.print("A "); 
      lcd.print(resistance,1);
      lcd.print((char)244);
      lcd.print("      ");
    }
    if(mode==2)
    {
      lcd.print(input_voltage,1);
      lcd.print("V      ");
      lcd.setCursor(0,1);
      lcd.print(current,2);
      lcd.print("A ");
      lcd.print(power,1);
      lcd.print("W      ");
    }
    if(mode==3)
    {
      lcd.print(input_voltage,1);
      lcd.print("V      ");
      lcd.setCursor(0,1);
      lcd.print(resistance,1);
      lcd.print((char)244);
      lcd.print(" ");
      lcd.print(power,1);
      lcd.print("W      ");
    }
  }

  //turn fan on when 30W or higher
  if(power>29)
  {
    digitalWrite(13, HIGH);
  }
  else
  {
    digitalWrite(13, LOW);
  }

  //analogWrite(10, 255); //control testing
  analogWrite(10, output);
  
}
