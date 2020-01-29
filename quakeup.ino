//Sample using LiquidCrystal library
#include <LiquidCrystal.h>

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int sec = 0;
int minute = 0;
int hour = 12;
int timeIndicator = 0;
bool displayPM = false;

int selectedUser = -1;
bool turnMotorsOn = false;
bool inInterval = false;

int counter;

int raw= 0;
int Vin= 3.3;
float Vout= 0;
float R1= 1000;
float R2= 0;
float prev_R2 = 0;
float buffer= 0;

int i = 0, j = 0;
int depth = 0;
int menu[4][5] =
{
  {
    12, 0, 0, 1, 100         // hour, minutes, AM/PM, interval, weight
  },
  {
    12, 0, 0, 1, 100
  },
  {
    12, 0, 0, 1, 100
  },
  {
    12, 0, 0, 1, 100         // hour, minutes, AM/PM, --, --
  }
};

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define RELAY_PIN A1
#define VELO_PIN A5

// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 195)  return btnUP;
  if (adc_key_in < 380)  return btnDOWN;
  if (adc_key_in < 555)  return btnLEFT;
  if (adc_key_in < 790)  return btnSELECT;
  return btnNONE;  // when all others fail, return this...
}

void setup()
{
  lcd.begin(16, 2);              // start the library
  lcd.setCursor(0, 0);
  displaySetTime();
  pinMode(RELAY_PIN, OUTPUT);
}

void loop()
{
  counter += 1;
  timeUpdate();
  delay(200);          // move to the begining of the second line
  lcd.clear();
  menuDisplay();
  if (depth == 0) j = 0;
  lcd_key = read_LCD_buttons();  // read the buttons
  buttonPress();
  verifyAlarm();
  alarmOn();
}

void buttonPress() {
  switch (lcd_key)               // depending on which button was pushed, we perform an action
  {
    case btnRIGHT:
      {
        if (depth < 2) depth += 1;
        break;
      }
    case btnLEFT:
      {
        if (depth > 0) depth -= 1;
        break;
      }
    case btnUP:
      {
        if (depth == 0 && i > 0) i -= 1;
        if (depth == 1 && j > 0) j -= 1;
        break;
      }
    case btnDOWN:
      {
        if (depth == 0 && i < 3) i += 1;
        if (depth == 1 && j < 2) j += 1;
        break;
      }
    case btnSELECT:
      {
        if (depth == 0)
        {
          selectUser();
        }
      }
    case btnNONE:
      {
        break;
      }
  }
}


struct displaySetTimeValues {
  int arrow = 0;
  int myTime[5] = {1, 2, 0, 0, 0}; //h1, h2, collon,  m1, m2
};

void displayAlarmTime() {
  bool selected = false;
  
  displaySetTimeValues x;
  int timeAM = 0;
  String timeSuffix = "AM";

  switch(i){
    case 0:
      x.myTime[0] = menu[0][0]/10;
      x.myTime[1] = menu[0][0]%10;
      x.myTime[3] = menu[0][1]/10;
      x.myTime[4] = menu[0][1]%10;
      if (menu[0][2] == 0) timeSuffix = "AM";
      else timeSuffix = "PM";
      break;
    case 1:
      x.myTime[0] = menu[1][0]/10;
      x.myTime[1] = menu[1][0]%10;
      x.myTime[3] = menu[1][1]/10;
      x.myTime[4] = menu[1][1]%10;
      if (menu[1][2] == 0) timeSuffix = "AM";
      else timeSuffix = "PM";
      break;
    case 2:
      x.myTime[0] = menu[2][0]/10;
      x.myTime[1] = menu[2][0]%10;
      x.myTime[3] = menu[2][1]/10;
      x.myTime[4] = menu[2][1]%10;
      if (menu[2][2] == 0) timeSuffix = "AM";
      else timeSuffix = "PM";
      break;  
  }
  
  String defaultTime = String(x.myTime[0]) + String(x.myTime[1]) + ":" + String(x.myTime[3]) + String(x.myTime[4]) + timeSuffix;
  
  while (not selected) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(defaultTime);
    lcd.setCursor(x.arrow, 1);
    lcd.print("^");
    delay(200);
    counter++;
    
    lcd_key = read_LCD_buttons();
    defaultTime = String(x.myTime[0]) + String(x.myTime[1]) + ":" + String(x.myTime[3]) + String(x.myTime[4]) + timeSuffix;

    
    switch(lcd_key){
      case btnUP:
        x.myTime[x.arrow] += 1;

        if (x.arrow == 0 && x.myTime[x.arrow] > 1)
          x.myTime[x.arrow] = 0;
        else if (x.arrow == 0 && x.myTime[x.arrow + 1] > 2 && x.myTime[x.arrow] == 1) 
          x.myTime[x.arrow + 1] = 0;
        else if (x.arrow == 1 && x.myTime[0] == 1 && x.myTime[x.arrow] > 2)
          x.myTime[x.arrow] = 0;
        else if (x.arrow == 1 && x.myTime[0] == 0 && x.myTime[x.arrow] > 9)
          x.myTime[x.arrow] = 1;
        else if (x.arrow == 3 && x.myTime[x.arrow] > 5)
          x.myTime[x.arrow] = 0;
        else if (x.arrow == 4 && x.myTime[x.arrow] > 9)
          x.myTime[x.arrow] = 0;
        else if (x.arrow == 0 && x.myTime[x.arrow] == 0 && x.myTime[x.arrow + 1] == 0)
          x.myTime[x.arrow + 1] = 1;
        else if (x.arrow == 5){
          if (timeSuffix == "AM"){
            timeSuffix = "PM";
            timeAM = 1;
          }
          else{
            timeSuffix = "AM";
            timeAM = 0;
          }
          }  

        if (x.arrow == 0 && x.myTime[x.arrow] == 0 && x.myTime[x.arrow + 1] == 0)
          x.myTime[x.arrow + 1] = 1;
        break;
      case btnDOWN:
        x.myTime[x.arrow] -= 1;

        if (x.arrow == 0 && x.myTime[x.arrow] < 0)
          x.myTime[x.arrow] = 1;
        else if (x.arrow == 1 && x.myTime[x.arrow - 1] == 1 && x.myTime[x.arrow] < 0)
          x.myTime[x.arrow] = 2;
        else if (x.arrow == 1 && x.myTime[x.arrow - 1] == 0 && x.myTime[x.arrow] < 1)
          x.myTime[x.arrow] = 9;
        else if (x.arrow == 3 && x.myTime[x.arrow] < 0)
          x.myTime[x.arrow] = 5;
        else if (x.arrow == 4 && x.myTime[x.arrow] < 0)
          x.myTime[x.arrow] = 9;
        else if (x.arrow == 0 && x.myTime[x.arrow] == 0 && x.myTime[x.arrow + 1] == 0)
          x.myTime[x.arrow + 1] = 1;
        else if (x.arrow == 5){
          if (timeSuffix == "AM"){
            timeSuffix = "PM";
            timeAM = 1;
          }
          else{
            timeSuffix = "AM";
            timeAM = 0;
          }
        }

        if (x.arrow == 0 && x.myTime[x.arrow] == 1 && x.myTime[x.arrow + 1] > 2) 
          x.myTime[x.arrow + 1] = 0;  
        break;
      case btnRIGHT:
        x.arrow +=1;
        if (x.arrow == 2) 
          x.arrow = 3;
        else if (x.arrow > 5)
          x.arrow = 5;
        lcd.clear();
        break;
      case btnLEFT:
        if (x.arrow == 0){
          selected = true;
          break;
        }
        x.arrow -=1;
        if (x.arrow == 2) 
          x.arrow = 1;
        else if (x.arrow < 0)
          x.arrow = 0;
        lcd.clear();
        break;
      case btnSELECT:
        selected = true;
        break;       
    }
    switch(i){
      case 0:
        menu[0][0] = x.myTime[0]*10 + x.myTime[1];
        menu[0][1] = x.myTime[3]*10 + x.myTime[4];
        menu[0][2] = timeAM;
        break;
      case 1:
        menu[1][0] = x.myTime[0]*10 + x.myTime[1];
        menu[1][1] = x.myTime[3]*10 + x.myTime[4];
        menu[1][2] = timeAM;
        break;
      case 2:
        menu[2][0] = x.myTime[0]*10 + x.myTime[1];
        menu[2][1] = x.myTime[3]*10 + x.myTime[4];
        menu[2][2] = timeAM;
        break;
    }
  }
  depth = 1;
  menuDisplay();
}

void displaySetInterval() {
  bool selected = false;
  int intervalTime = 1;

  while (not selected){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("How many minutes?");
    lcd.setCursor(0,1);
    switch(i){
      case 0:
        intervalTime = menu[0][3];
        break;
      case 1:
        intervalTime = menu[1][3];
        break;
      case 2:
        intervalTime = menu[2][3];
        break;
    }
    lcd.print(intervalTime);
    delay(200);
    counter ++;

    lcd_key = read_LCD_buttons();

  
    switch(lcd_key){
      case btnUP:
        if (intervalTime == 59) intervalTime = 59;
        else intervalTime +=1;
        break;
      case btnDOWN:
        if (intervalTime == 1){
          intervalTime = 1;
        }
        else{
          intervalTime -= 1;
        }
        break;
      case btnLEFT:
        selected = true;
        break;
      case btnSELECT:
        selected = true;
        break;
    }
    switch(i){
      case 0:
        menu[0][3] = intervalTime;
        break;
      case 1:
        menu[1][3] = intervalTime;
        break;
      case 2:
        menu[2][3] = intervalTime;
        break;
    }
  }
  depth = 1;
  menuDisplay();
}

void displaySetWeight() {
  bool selected = false;
  int weight = 100;

  while (not selected){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Insert weight:");
    lcd.setCursor(0,1);
    switch(i){
      case 0:
        weight = menu[0][4];
        break;
      case 1:
        weight = menu[1][4];
        break;
      case 2:
        weight = menu[2][4];
        break;
    }
    lcd.print(weight);
    delay(200);
    counter ++;

    lcd_key = read_LCD_buttons();

  
    switch(lcd_key){
      case btnUP:
        weight +=1;
        break;
      case btnDOWN:
        if (weight == 1){
          weight = 1;
        }
        else{
          weight -= 1;
        }
        break;
      case btnLEFT:
        selected = true;
        break;
      case btnSELECT:
        selected = true;
        break;
    }
    switch(i){
      case 0:
        menu[0][4] = weight;
        break;
      case 1:
        menu[1][4] = weight;
        break;
      case 2:
        menu[2][4] = weight;
        break;
    }
  }
  depth = 1;
  menuDisplay();
}

void menuDisplay() {
  String timeSuffix = "AM";
  if (timeIndicator == 0) timeSuffix = "AM";
  else timeSuffix = "PM";
  String currentTime = String(hour/10) + String(hour%10) + ":" + String(minute/10) + String(minute%10) + timeSuffix;
  switch (depth)
  {
    case 0:
      {
        switch (i)
        {
          case 0:
            {
              lcd.setCursor(0, 0);
              lcd.print(">User 1");
              if (selectedUser == 0){
                lcd.setCursor(7,0);
                lcd.print("*");
              }
              lcd.setCursor(9,0);
              lcd.print(currentTime);
              lcd.setCursor(0, 1);
              lcd.print(" User 2");
              if (selectedUser == 1){
                lcd.setCursor(7,1);
                lcd.print("*");
              }
              break;
            }
          case 1:
            {
              lcd.setCursor(0, 0);
              lcd.print(">User 2");
              if (selectedUser == 1){
                lcd.setCursor(7,0);
                lcd.print("*");
              }
              lcd.setCursor(9,0);
              lcd.print(currentTime);
              lcd.setCursor(0, 1);
              lcd.print(" User 3");
              if (selectedUser == 2){
                lcd.setCursor(7,1);
                lcd.print("*");
              }
              break;
            }
          case 2:
            {
              lcd.setCursor(0, 0);
              lcd.print(">User 3");
              if (selectedUser == 2){
                lcd.setCursor(7,0);
                lcd.print("*");
              }
              lcd.setCursor(9,0);
              lcd.print(currentTime);
              lcd.setCursor(0, 1);
              lcd.print(" General");
              break;
            }
          case 3:
            {
              lcd.setCursor(0, 0);
              lcd.print(">General");
              lcd.setCursor(9,0);
              lcd.print(currentTime);
              lcd.setCursor(0, 1);
              break;
            }
        }
        break;
      }


    case 1:
      {
        if (i == 3)
        {
          switch (j)
          {
            case 0:
              {
                lcd.setCursor(0, 0);
                lcd.print(">Set Clock");
                lcd.setCursor(0, 1);
                lcd.print(" Reset All Values");
                break;
              }
            case 1:
              {
                lcd.setCursor(0, 0);
                lcd.print(">Reset All Values");
                lcd.setCursor(0, 1);
                break;
              }
          }
        }
        else
        {
          switch (j)
          {
            case 0:
              {
                lcd.setCursor(0, 0);
                lcd.print(">Alarm Time");
                lcd.setCursor(0, 1);
                lcd.print(" Interval Time");
                break;
              }
            case 1:
              {
                lcd.setCursor(0, 0);
                lcd.print(">Interval Time");
                lcd.setCursor(0, 1);
                lcd.print(" Set Weight");
                break;
              }
            case 2:
              {
                lcd.setCursor(0, 0);
                lcd.print(">Set Weight");
                break;
              }
          }
        }
        break;
      }


    case 2:
      {
        if (i == 3)
        {
          switch (j)
          {
            case 0:
              {
                displaySetTime();
              }
            case 1:
              {
                resetValues();
              }
            case 2:
              {
                //redelares all values
              }
          }
        }
        else
        {
          switch (j)
          {
            case 0:
              {
                displayAlarmTime();
                break;
              }
            case 1:
              {
                displaySetInterval();
                break;
              }
            case 2:
              {
                displaySetWeight();
                break;
              }
          }
        }
        break;
      }
  }
}

void timeUpdate() {
   
  if (counter % 5 == 0) {
    sec += 1;
    counter = 0;
  }
  if (sec > 59) {
    sec = 0;
    minute += 1;
  }

  if (minute > 59) {
    minute = 0;
    hour += 1;
    if ((hour == 12) && (sec == 0)){
      timeIndicator += 1;
    if (timeIndicator == 2) timeIndicator = 0;
    }
  }

  

  if (timeIndicator == 0) displayPM = false;
  if (timeIndicator == 1) displayPM = true;

}



void displaySetTime() {
  bool selected = false;
  
  displaySetTimeValues x;
  int timeAM = 0;
  String timeSuffix = "AM";

  // hour, minute, timeIndicator
  x.myTime[0] = hour/10;
  x.myTime[1] = hour%10;
  x.myTime[3] = minute/10;
  x.myTime[4] = minute%10;
  if (timeIndicator == 0) timeSuffix = "AM";
  else timeSuffix = "PM";
  
  
  String defaultTime = String(x.myTime[0]) + String(x.myTime[1]) + ":" + String(x.myTime[3]) + String(x.myTime[4]) + timeSuffix;
  
  while (not selected) {
    lcd.setCursor(0,0);
    lcd.print(defaultTime);
    lcd.setCursor(x.arrow, 1);
    lcd.print("^");
    delay(200);
    counter++;
    
    lcd_key = read_LCD_buttons();
    defaultTime = String(x.myTime[0]) + String(x.myTime[1]) + ":" + String(x.myTime[3]) + String(x.myTime[4]) + timeSuffix;

    
    switch(lcd_key){
      case btnUP:
        x.myTime[x.arrow] += 1;

        if (x.arrow == 0 && x.myTime[x.arrow] > 1)
          x.myTime[x.arrow] = 0;
        else if (x.arrow == 0 && x.myTime[x.arrow + 1] > 2 && x.myTime[x.arrow] == 1) 
          x.myTime[x.arrow + 1] = 0;
        else if (x.arrow == 1 && x.myTime[0] == 1 && x.myTime[x.arrow] > 2)
          x.myTime[x.arrow] = 0;
        else if (x.arrow == 1 && x.myTime[0] == 0 && x.myTime[x.arrow] > 9)
          x.myTime[x.arrow] = 1;
        else if (x.arrow == 3 && x.myTime[x.arrow] > 5)
          x.myTime[x.arrow] = 0;
        else if (x.arrow == 4 && x.myTime[x.arrow] > 9)
          x.myTime[x.arrow] = 0;
        else if (x.arrow == 0 && x.myTime[x.arrow] == 0 && x.myTime[x.arrow + 1] == 0)
          x.myTime[x.arrow + 1] = 1;
        else if (x.arrow == 5){
          if (timeSuffix == "AM"){
            timeSuffix = "PM";
            timeAM = 1;
          }
          else{
            timeSuffix = "AM";
            timeAM = 0;
          }
          }  

        if (x.arrow == 0 && x.myTime[x.arrow] == 0 && x.myTime[x.arrow + 1] == 0)
          x.myTime[x.arrow + 1] = 1;
        break;
      case btnDOWN:
        x.myTime[x.arrow] -= 1;

        if (x.arrow == 0 && x.myTime[x.arrow] < 0)
          x.myTime[x.arrow] = 1;
        else if (x.arrow == 1 && x.myTime[x.arrow - 1] == 1 && x.myTime[x.arrow] < 0)
          x.myTime[x.arrow] = 2;
        else if (x.arrow == 1 && x.myTime[x.arrow - 1] == 0 && x.myTime[x.arrow] < 1)
          x.myTime[x.arrow] = 9;
        else if (x.arrow == 3 && x.myTime[x.arrow] < 0)
          x.myTime[x.arrow] = 5;
        else if (x.arrow == 4 && x.myTime[x.arrow] < 0)
          x.myTime[x.arrow] = 9;
        else if (x.arrow == 0 && x.myTime[x.arrow] == 0 && x.myTime[x.arrow + 1] == 0)
          x.myTime[x.arrow + 1] = 1;
        else if (x.arrow == 5){
          if (timeSuffix == "AM"){
            timeSuffix = "PM";
            timeAM = 1;
          }
          else{
            timeSuffix = "AM";
            timeAM = 0;
          }

        if (x.arrow == 0 && x.myTime[x.arrow] == 1 && x.myTime[x.arrow + 1] > 2) 
          x.myTime[x.arrow + 1] = 0;  
        break;
      case btnRIGHT:
        x.arrow +=1;
        if (x.arrow == 2) 
          x.arrow = 3;
        else if (x.arrow > 5)
          x.arrow = 5;
        lcd.clear();
        break;
      case btnLEFT:
        if (x.arrow == 0){
          selected = true;
          break;
        }
        x.arrow -=1;
        if (x.arrow == 2) 
          x.arrow = 1;
        else if (x.arrow < 0)
          x.arrow = 0;
        lcd.clear();
        break;
      case btnSELECT:
        selected = true;
        hour = x.myTime[0]*10 + x.myTime[1];
        minute = x.myTime[3]*10 + x.myTime[4];
        timeIndicator = timeAM;
        break;       
    }
  }
  depth = 0;
}
}

void resetValues(){
  delay(200);
  counter++;
  timeUpdate();
  for (int i = 0; i < 4; i++){
    menu[i][0] = 12;
    menu[i][1] = 0;
    menu[i][2] = 0;
    menu[i][3] = 1;
    menu[i][4] = 100;
  }
  depth = 1;
  selectedUser = -1;
  turnMotorsOn = false;
}


void selectUser(){
  lcd_key = read_LCD_buttons();
  if (lcd_key == btnSELECT){
    if (selectedUser == i) {
      selectedUser = -1;
    }
    else{ 
      selectedUser = i;
    }
  }
}


void verifyAlarm(){
  //creating temp time variables to compare current time to time+interval
  turnMotorsOn = false;
//  inInterval = false;
//  int tempmin = 0; 
//  int temphour = menu[selectedUser][0];
//  if (selectedUser != -1){
//    tempmin = menu[selectedUser][1] + menu[selectedUser][3];
//    if (tempmin > 59){
//      tempmin = 60 - menu[selectedUser][1];
//      temphour++;
//    }
//    if (temphour > 12) {
//      temphour = 1;
//    }
//
//    // checks if the time is within interval
//    if (hour == 12 and temphour == 1){
//      inInterval = true;
//    }
//    else if (hour < temphour){
//      inInterval = true;
//    }
//    else if (hour == temphour){
//      if (minute < tempmin){
//        inInterval = true;
//      }
//    }
//    
//    if ((menu[selectedUser][0] == hour && menu[selectedUser][1] == minute && menu[selectedUser][2] == timeIndicator) || inInterval){
//          turnMotorsOn = true;
//    }

    if ((menu[selectedUser][0]*60 + menu[selectedUser][1] + menu[selectedUser][2]*720) <= (hour*60 + minute + timeIndicator*720) 
       && (hour*60 + minute + timeIndicator*720) <= (menu[selectedUser][0]*60 + menu[selectedUser][1] + menu[selectedUser][2]*720 + menu[selectedUser][3])){
          turnMotorsOn = true;    
    }
 }    


void alarmOn(){
  while(turnMotorsOn) {
    lcd.clear();
    lcd.print(" Q U A K E U P !");
    lcd_key = read_LCD_buttons();
    if (lcd_key == btnSELECT)
    {
      turnMotorsOn = false;
      break;
    }
    digitalWrite(RELAY_PIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(200); 
    counter++;
    timeUpdate();
    delay(200); 
    counter++;
    timeUpdate();
    delay(200); 
    counter++;
    timeUpdate();
    
    digitalWrite(RELAY_PIN, LOW); // turn the LED off by making the voltage LOW
    delay(200);
    counter++;
    timeUpdate();  

    verifyAlarm();
    
    raw = analogRead(VELO_PIN);
    if(raw) {
      buffer= raw * Vin;
      Vout= (buffer)/1024.0;
      buffer= (Vin/Vout) -1;
      R2= R1 * buffer;
    }
    if (( (R2 - prev_R2) > 400) && (prev_R2 != 0)) {
      turnMotorsOn = false;
      selectedUser = -1;
    }
    prev_R2 = R2;
    
  }
}
