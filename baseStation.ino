#include <painlessMesh.h>

#define MESH_PREFIX "mesh_network"
#define MESH_PASSWORD "password"
#define MESH_PORT 5555
#define Buzz 14
boolean led = 0;
const int maxNode = 100;
int node[maxNode] = {0};
String pesan[maxNode];

painlessMesh mesh;

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
 
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  } 
 
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void receivedCallback(uint32_t from, String &msg) {
  String data = ("💭 Update dari Node ID : "+String(from)+"("+getValue(msg, ' ', 0)+")");
  if (getValue(msg, ' ', 3) == "BAHAYA") {
    digitalWrite(Buzz, HIGH);
  } else {
    digitalWrite(Buzz, LOW);
  }
  Serial.println(data);
  if (node[0] == 0) {
    node [0] = from;
    pesan [0] = msg;
    Serial.println(pesan[0]);
  } else {
    for (int i=0; i<maxNode; i++) {
      if (node[i] == from){
        pesan[i] = msg;
        break;
      } else {
        if (node[i] == 0) {
          node[i] = from;
          pesan[i] = msg;
          for (int x=0; x<=i; x++) {
            Serial.print(pesan[x]);
            if (x<i) {
              Serial.print("; ");
            }
          }
          break;
        }
      }
    }
  }
  for (int i=0; i<maxNode; i++) {
    if (node[i] = 0){
      for (int x=0; x<=i; x++) {
            Serial.print(pesan[x]);
            if (x<i) {
              Serial.print("\n");
            }
          }
    }
  }
  
  Serial.println("");
  led = !led;
  digitalWrite(LED_BUILTIN, led);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(Buzz, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(Buzz, LOW);
  mesh.setDebugMsgTypes(ERROR);
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
}

void loop() {
  mesh.update();
}
