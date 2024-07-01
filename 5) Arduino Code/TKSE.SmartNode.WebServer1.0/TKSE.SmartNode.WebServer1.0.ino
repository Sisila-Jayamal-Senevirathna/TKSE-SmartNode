#include <WiFi.h>

const char* ssid = "CISIntl";
const char* password = "$unny4ever";

int statusLed = 2;          // D2
byte sensorInterrupt = 18;  // D18
unsigned long oldTime = 0;
volatile byte pulseCount = 0;  // To store pulse counts
bool SensorStatus = 0;
bool SensorStatusBefore = 0;
const char* valveStatus = "OFF";

WiFiServer server(80);

// Variable to store the HTTP request
String header;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  pinMode(statusLed, OUTPUT);
  pinMode(sensorInterrupt, INPUT_PULLUP); // Set pin 18 as input with internal pull-up resistor
  attachInterrupt(digitalPinToInterrupt(sensorInterrupt), pulseCounter, FALLING); // Attach interrupt to sensor pin

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  valveStatusUpdate();
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            if (header.indexOf("GET /status") >= 0) {
              // Send the valve status as JSON
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type: application/json");
              client.println("Connection: close");
              client.println();
              client.print("{\"valveStatus\":\"");
              client.print(valveStatus);
              client.println("\"}");
            } else {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              // Display the HTML web page
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");

              // CSS to style the on/off buttons and the status circle
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
              client.println("#valveStatus { font-size: 24px; }");
              client.println("#statusCircle { width: 20px; height: 20px; border-radius: 50%; display: inline-block; margin-left: 10px; background-color: black; }");
              client.println("</style></head>");

              // Web Page Heading
              client.println("<body><h1>Valve Indicator</h1>");
              client.println("<p>@ Team Automation</p>");
              client.println("<div>");
               client.println("<div><h2>Valve 01 State - </h2><span id='valveStatus'>OFF</span><div id='statusCircle'></div></div>");
              // client.print("<h2>Valve 01 State - </h2>");
              // client.println("<span id='valveStatus'>OFF</span>");
              // client.println("<div id='statusCircle'></div>");
              client.println("</div>");
              client.println("<div id='currentTime' style='font-size: 20px; margin-top: 20px;'></div>");

              

              client.println("<script>");
              client.println("setInterval(function() {");
              client.println("  fetch('/status').then(function(response) { return response.json(); }).then(function(data) {");
              client.println("    var statusLabel = document.getElementById('valveStatus');");
              client.println("    var statusCircle = document.getElementById('statusCircle');");
              client.println("    statusLabel.innerHTML = data.valveStatus;");
              client.println("    if (data.valveStatus === 'ON') {");
              client.println("      statusLabel.style.color = 'red';");
              client.println("      statusCircle.style.backgroundColor = 'red';");
              client.println("    } else {");
              client.println("      statusLabel.style.color = 'black';");
              client.println("      statusCircle.style.backgroundColor = 'black';");
              client.println("    }");
              client.println("  });");
              client.println("}, 1000);");

              // JavaScript to update date and time
              client.println("setInterval(function() {");
              client.println("  var now = new Date();");
              client.println("  var timeString = now.toLocaleTimeString();");
              client.println("  var dateString = now.toLocaleDateString();");
              client.println("  document.getElementById('currentTime').innerHTML = dateString + ' ' + timeString;");
              client.println("}, 1000);");
              client.println("</script>");
              // The HTTP response ends with another blank line
              client.println("</body></html>");
            }
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void valveStatusUpdate() {
  if ((millis() - oldTime) > 1000) {  // Only process counters once per second
    oldTime = millis();

    if (pulseCount > 0) {
      pulseCount = 0;  // Reset the pulse count
      SensorStatus = 1;
    } else {
      SensorStatus = 0;
    }
  }

  if (SensorStatus != SensorStatusBefore) {
    if (SensorStatus == 1) {
      Serial.println("Valve is ON");
      valveStatus = "ON";
      digitalWrite(statusLed, HIGH);
    } else {
      Serial.println("Valve is OFF");
      valveStatus = "OFF";
      digitalWrite(statusLed, LOW);
    }
    SensorStatusBefore = SensorStatus;
  }
}

void pulseCounter() {
  pulseCount++;  // Increment pulse count on interrupt
}
