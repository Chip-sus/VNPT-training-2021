void dataToNodeRed()
{
  // Sending data to MQTT broker and display on node-red UI
  String str_lux = "";
  str_lux = String(lux, 0);
  byte c_lux[str_lux.length() + 1];                   // buffer to send to node-red
  str_lux.toCharArray(c_lux, str_lux.length() + 1);
  client.publish("lux", c_lux);

  String str_temp = "";
  str_temp = String(lm35_data, 0);
  byte c_temp[str_temp.length() + 1];
  str_temp.toCharArray(c_temp, str_temp.length() + 1);
  client.publish("temp", c_temp);

  String str_humid = "";
  // cannot convert "int" to String and push to NodeRed, so I change to float first
  float fhumid = (float) humid;
  str_humid = String(fhumid, 0);
  byte c_humid[str_humid.length() + 1];
  str_humid.toCharArray(c_humid, str_humid.length() + 1);
  client.publish("humid", c_humid);

  // Sending alert limits
  String lim_lux = "";
  float fluxAlert = (float)luxAlert;
  lim_lux = String(fluxAlert, 0);
  byte c_luxLim[lim_lux.length() + 1];
  lim_lux.toCharArray(c_luxLim, lim_lux.length() + 1);
  client.publish("luxLim", c_luxLim);

  String lim_temp = "";
  float ftempAlert = (float)tempAlert;
  lim_temp = String(ftempAlert, 0);
  byte c_tempLim[lim_temp.length() + 1];
  lim_temp.toCharArray(c_tempLim, lim_temp.length() + 1);
  client.publish("tempLim", c_tempLim);

  String lim_humid = "";
  float fhumidAlert = (float)humidAlert;
  lim_humid = String(fhumidAlert, 0);
  byte c_humidLim[lim_humid.length() + 1];
  lim_humid.toCharArray(c_humidLim, lim_humid.length() + 1);
  client.publish("humidLim", c_humidLim);
}

void chmod()                  // triggered when press first button
{
  mode++;
  if(mode == 4) mode = 0;
  scroll = 0;
  lcd.clear();
  dl();
  Serial.println("changed");
}

void offAlert()
{
  if(alert == 1 && (lux < luxAlert || lm35_data > tempAlert || humid > humidAlert))
  {
    alert = 0;
  }
}

void dl()                     // for debouncing (?)
{
  delay(150);
}

void mode0()
{
  // nothing
}

void mode1()
{
  // This mode changes the TEMPERATURE limit
  if(digitalRead(BUTTON3) == 0)         // check if button2 is pressed
  {
    while(digitalRead(BUTTON3) == 0){}  // check untill release the button
    tempAlert++;
    delay(50);
  }
  
  if(digitalRead(BUTTON2) == 0)
  {
    while(digitalRead(BUTTON2) == 0){}
    tempAlert--;
    delay(50);
  }
  if(tempAlert >= 60) tempAlert = 60;
  if(tempAlert <= 0) tempAlert = 0;
  
  lcd.setCursor(0,0);
  lcd.print("LM35 sensor");

  lcd.setCursor(0,1);
  lcd.print("Limit: ");
  lcd.print(tempAlert);
  lcd.write(byte(0));
  lcd.print("C");
  lcd.print("   ");
  delay(100);
}

void mode2()
{
  // This mode changes the HUMIDITY limit
  if(digitalRead(BUTTON3) == 0)         // check if button2 is pressed
  {
    while(digitalRead(BUTTON3) == 0){}  // check untill release the button
    humidAlert += 2;
    delay(50);
  }
  
  if(digitalRead(BUTTON2) == 0)
  {
    while(digitalRead(BUTTON2) == 0){}
    humidAlert -= 2;
    delay(50);
  }
  if(humidAlert >= 100) humidAlert = 100;
  if(humidAlert <= 0) humidAlert = 0;

  lcd.setCursor(0,0);
  lcd.print("DHT11 sensor");

  lcd.setCursor(0,1);
  lcd.print("Limit: ");
  lcd.print(humidAlert);
  lcd.print("%");
  lcd.print("   ");
  delay(100);
}

void mode3()
{
  // This mode changes the LUX limit
  if(digitalRead(BUTTON3) == 0)         // check if button2 is pressed
  {
    while(digitalRead(BUTTON3) == 0){}  // check untill release the button
    luxAlert += 10;
    delay(50);
  }
  
  if(digitalRead(BUTTON2) == 0)
  {
    while(digitalRead(BUTTON2) == 0){}
    luxAlert -= 10;
    delay(50);
  }
  if(luxAlert >= 1000) luxAlert = 1000;
  if(luxAlert <= 0) luxAlert = 0;

  lcd.setCursor(0,0);
  lcd.print("LDR NSL06S53");

  lcd.setCursor(0,1);
  lcd.print("Limit: ");
  lcd.print(luxAlert);
  lcd.print("   ");
  delay(100);
}
