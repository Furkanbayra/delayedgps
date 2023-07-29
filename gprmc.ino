#include <SoftwareSerial.h>

const int BUFFER_SIZE = 128;
char buffer[BUFFER_SIZE];
int bufferIndex = 0;
SoftwareSerial NMEA(10, 11);

void setup() {
  while (!Serial)
    ;
  Serial.begin(9600);
  NMEA.begin(9600);
}

void loop() {
  if (NMEA.available()) {
    char receivedChar = NMEA.read();
    // Wait for the start of an NMEA sentence ('$')
    if (receivedChar == '$') {
      bufferIndex = 0; // Start of a new data line, reset bufferIndex
      buffer[bufferIndex] = receivedChar;
      bufferIndex++;
    } else if (bufferIndex > 0 && receivedChar == '\n') {
      // End of the line, check if it is an RMC sentence
      buffer[bufferIndex] = '\0';
      String data = String(buffer);
      if (data.startsWith("$GPRMC")) {
        data.trim();
        delay(60000);
        Serial.println("SS-GPS," + data + "-T");
      }

      bufferIndex = 0; // Reset bufferIndex for the next sentence
    } else if (bufferIndex > 0 && bufferIndex < BUFFER_SIZE - 1) {
      // Store only RMC data in the buffer
      buffer[bufferIndex] = receivedChar;
      bufferIndex++;
    } else {
      // Ignore the data if buffer is full or no valid RMC sentence found yet
    }
  }
}
