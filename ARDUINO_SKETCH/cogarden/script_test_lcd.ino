#include <SoftwareSerial.h>
SoftwareSerial lcd(3, 2);



void aff_panel(int l, int u, int h, int t) {
  
param();
aff_Lu(l);
aff_hu(u);
aff_hy(h);
aff_tmp(t);

}

void param() {

  lcd.begin(9600); 
  clearDisplay();
  setBacklight(150);
}

void aff_Lu(int l)
{
setLCDCursor(0);
lcd.print("L: ");
lcd.print(l);
lcd.print("L");  
}

void aff_hu(int u)
{
setLCDCursor(25);
lcd.print("U: ");
lcd.print(u);
lcd.print("%");
}

void aff_hy(int h)
{
setLCDCursor(9);
lcd.print("H: ");
lcd.print(h);
}

void aff_tmp(int t)
{
setLCDCursor(16);
lcd.print("T: ");
lcd.print(t);
lcd.print("C");
}

void clearDisplay()
{
  lcd.write(0xFE);  // send the special command
  lcd.write(0x01);  // send the clear screen command
}

void setLCDCursor(byte cursor_position)
{
  lcd.write(0xFE);  // send the special command
  lcd.write(0x80);  // send the set cursor command
  lcd.write(cursor_position);  // send the cursor position
}

void setBacklight(byte brightness)
{
  lcd.write(0x80);  // send the backlight command
  lcd.write(brightness);  // send the brightness value
}
