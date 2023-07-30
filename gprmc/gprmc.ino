#include <SoftwareSerial.h>

const int BUFFER_SIZE = 128;
char inputBuffer[BUFFER_SIZE];
int bufferIndex = 0;
SoftwareSerial NMEA(10, 11);

unsigned long previousMillis = 0;
const unsigned long interval = 60000;
bool rmcFound = false;

void setup() {
  while (!Serial)
    ;
  Serial.begin(9600);
  NMEA.begin(9600);
}

void processRMC(const char* rmcData) {
  Serial.println("SS-GPS," + String(rmcData) + "-T");
}

void clearBuffer() {
  memset(inputBuffer, 0, sizeof(inputBuffer));
  bufferIndex = 0;
}

void loop() {
  unsigned long currentMillis = millis();

  if (!rmcFound) {
    while (NMEA.available()) {
      char receivedChar = NMEA.read();

      if (receivedChar == '$') {
        clearBuffer();
      }

      if (bufferIndex < BUFFER_SIZE - 1) {
        inputBuffer[bufferIndex] = receivedChar;
        bufferIndex++;
      }

      int endOfSentence = -1;
      for (int i = 0; i < bufferIndex - 1; i++) {
        if (inputBuffer[i] == '\r' && inputBuffer[i + 1] == '\n') {
          endOfSentence = i;
          break;
        }
      }

      if (endOfSentence >= 0) {
        inputBuffer[endOfSentence] = '\0';  
        String sentence = String(inputBuffer);
        memmove(inputBuffer, inputBuffer + endOfSentence + 2, bufferIndex - endOfSentence - 2);
        bufferIndex -= endOfSentence + 2;
        if (sentence.startsWith("$GPRMC")) {
          sentence.trim();               
          processRMC(sentence.c_str());  
          rmcFound = true;              
          Serial.println("RMC FOUND");
          Serial.println("Buffer DATA: " + String(inputBuffer));
          Serial.println("Buffer SIZE: " + String(bufferIndex));

          break;
        }
      }
    }
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (rmcFound) {
      clearBuffer();
      rmcFound = false;
    }
  }
}
