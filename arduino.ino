#include <Servo.h>

Servo thumb;
Servo index;
Servo middle;
Servo ring;
Servo pinky;

String receivedData = ""; 
int values[5];  

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(1);

  thumb.attach(3);
  index.attach(5);
  middle.attach(6);
  ring.attach(9);
  pinky.attach(10);

}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();  
    if (c == '\n') {  
      parseData(receivedData);  
      receivedData = ""; 
    } else {
      receivedData += c;  
    }
  }

  displayValues();
  delay(100);  
}

void parseData(String data) {
  int index = 0;
  int startIndex = 0;

  for (int i = 0; i < 5; i++) {
    index = data.indexOf(',', startIndex);  
    if (index == -1) { 
      index = data.length();  
    }
    values[i] = data.substring(startIndex, index).toInt(); 
    startIndex = index + 1;  
  }
}

void displayValues() {
  Serial.print("Odebrane wartości: ");
  for (int i = 0; i < 5; i++) {
    thumb.write(values[0]);
    index.write(values[1]);
    middle.write(values[2]);
    ring.write(values[3]);
  //pinky.write(values[4]);
    Serial.print(values[i]);
    Serial.print(" ");
  }
  Serial.println();
}
