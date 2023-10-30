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

byte calculateChecksum(const String& sentence) {  // checksum oluşturucu
  byte checksum = 0;

  // '$' karakterinden sonra başlayarak '*' karakterine kadar olan tüm karakterleri XOR işlemine tabi tutun
  for (int i = 1; i < sentence.length(); i++) {
    if (sentence[i] == '*') break;  // '*' karakterine ulaşıldığında döngüyü durdurun
    checksum ^= sentence[i];
  }
  return checksum;
}


String createNmeaSentence(const String& sentenceId, const String& data) {  // NMEA sentence oluşturucu
  String sentence = "$" + sentenceId + "," + data;
  byte checksum = calculateChecksum(sentence);
  String checksumHex = String(checksum, HEX);
  sentence += "*" + checksumHex;
  return sentence;
}

String modifytime(const String& originalTime) {
  int hour = originalTime.substring(0, 2).toInt();
  int minute = originalTime.substring(2, 4).toInt();
  int second = originalTime.substring(4, 6).toInt();
  hour = (hour + 3) % 24;
  String modifiedTime = String(hour, DEC) + String(minute, DEC) + String(second, DEC);
  return modifiedTime;
}

int splitString(String input, char separator, String* parts, int maxParts) {
  int partCount = 0;
  int startIndex = 0;
  int endIndex = input.indexOf(separator);

  while (endIndex != -1 && partCount < maxParts - 1) {
    parts[partCount] = input.substring(startIndex, endIndex);
    startIndex = endIndex + 1;
    endIndex = input.indexOf(separator, startIndex);
    partCount++;
  }

  // Add the last part of the string
  if (partCount < maxParts) {
    parts[partCount] = input.substring(startIndex);
    partCount++;
  }

  return partCount;
}

void processRMC(const char* rmcData) {
  String a = String(rmcData);
  String rpmcparts[12];
  int numofParts = splitString(a, ',', rpmcparts, 12);
  String extime = rpmcparts[1];
  String newTime = modifytime(extime);
  String field2 = rpmcparts[2];
  String field3 = rpmcparts[3];
  String field4 = rpmcparts[4];
  String field5 = rpmcparts[5];
  String field6 = rpmcparts[6];
  String field7 = rpmcparts[8];
  String field8 = rpmcparts[9];
  String field9 = rpmcparts[10];
  String field10 = rpmcparts[11];
  String field11 = rpmcparts[12];
  String rawrmcData = newTime + "," + field2 + "," + field3 + "," + field4 + "," + field5 + "," + field6 + "," + field7 + "," + field8 + "," + field9 + "," + field10 + "," + field11;
  String rmcSentence = createNmeaSentence("GPRMC", rmcData);
  Serial.println("SS-GPS," + rmcSentence + "-T");
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