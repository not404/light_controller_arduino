// JSON Controlled Lights (Demo)
// Copyright (c) Laurence A. Lee (rubyjedi@gmail.com)
// All Rights Reserved.
//
// Permission to use or modify this software for personal, educational, or non-commercial use
// only is hereby granted. All other rights, including commercial usage, is reserved by the author.
//
// ** This may be "Open Source", but  it IS NOT "Free Software".  **
//
// 1. Implement PWM Control of an LED's Brightness at 5 levels
// 2. Add support for a pair of switches to Increase/Decrease LED Brightness
// 3. Add support for an Optical Sensor to detect ambient light levels
// 4. Convert the LED PWM Circuit - have it drive a Power Transistor instead of a single LED.

// Circuit: Connect a LED to a Digital Pin with PWM, such as Pin 2 on an Arduino UNO
//   PWM analogWrites accept integer values from 0 to 254 to determine the duty cycle of the pin.

#include <aJSON.h>

// Pin Assignments . . .
#define PWM_CONTROL_PIN 9
#define SENSOR_PIN A0
#define SWITCH_UP_PIN 5
#define SWITCH_DOWN_PIN 7

// Remaining Constants . . .
#define CONTROL_INCREMENT 50
#define SENSOR_THRESHOLD  150
#define SERIAL_BAUDRATE 115200
#define SERIAL_BUFFER_LENGTH 255

// ----------------------------------------------------------------------------------------------------

boolean  debug=false; // Verbose Debugging
boolean  serial_data_ready  = false;

int ambient_intensity = 0; // Ambient Intensity Reading
int ambient_threshold = SENSOR_THRESHOLD; // Ambient Intensity Threshold Setting
int intensity = 255;  // PWM Intensity Setting

// Switch States : Off, On, or Auto (On when "Dark" - when Ambient Intensity Reading < Ambient Intensity Threshold Setting)
enum pwm_switch_states {
  OFF=0,
  ON,
  AUTO
};
pwm_switch_states pwm_switch_state = AUTO;

// Serial Buffer Support
int  serial_buffer_head = 0;
char serial_buffer[SERIAL_BUFFER_LENGTH + 1]; // "+ 1" to accomodate a final terminating NULL if the buffer itself is filled up.
char in_char;

// sprintf() support
char string_buffer[SERIAL_BUFFER_LENGTH];

// JSON Parser Support
aJsonObject* json_message;

// ----------------------------------------------------------------------------------------------------
// serialPrintis a basic printf() implementation, hard-coded to write to current Serial Port.
void serialDebug(char *format, ...)
{
  if (!debug)
    return;

  va_list args;
  va_start (args,format);
  vsnprintf(string_buffer,sizeof(string_buffer[0]),format,args);
  va_end (args);
  // string_buffer[ sizeof(string_buffer)/sizeof(string_buffer[0]) - 1]='\0';

  Serial.print(string_buffer);
}

void serialPrint(char *format, ...)
{
  va_list args;
  va_start (args,format);
  vsnprintf(string_buffer,sizeof(string_buffer[0]),format,args);
  va_end (args);
  // string_buffer[ sizeof(string_buffer)/sizeof(string_buffer[0]) - 1]='\0';

  Serial.print(string_buffer);
}

// ----------------------------------------------------------------------------------------------------
// Output a JSON Status-Update message to the Serial Port.
void logStatus() {
  serialPrint("{\"ambientIntensity\":%d, \"intensity\":%d, \"ambientThreshold\":%d, \"lightSwitch\":%d, \"debug\":%d, \"serial_buffer_head\":%d}\n", ambient_intensity, intensity, ambient_threshold, pwm_switch_state, debug, serial_buffer_head);
}

// ----------------------------------------------------------------------------------------------------

// Setup input, output, and serial
void setup() {
  pinMode(PWM_CONTROL_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(SWITCH_UP_PIN, INPUT);
  pinMode(SWITCH_DOWN_PIN, INPUT);

  Serial.begin(SERIAL_BAUDRATE);
}

// Main application loop
void loop() {

  // Read the Ambient Light Level
    ambient_intensity = map( analogRead(SENSOR_PIN), 0,900, 0,255);

  // Write a NonZero Value to the PWM Pin if our conditions are met:
  // PWM Switch is "ON", or
  // PWM Switch is "AUTO" and the Ambient Intensity levels are below the Ambient Threshold Setting. (Too Dark, turn on the lights!)
    if ( (pwm_switch_state==ON) || (pwm_switch_state==AUTO) && (ambient_intensity < ambient_threshold) ) {
      analogWrite(PWM_CONTROL_PIN, intensity);
    } else {
      analogWrite(PWM_CONTROL_PIN, 0);
    }

  // Check to see if any Switches are pressed, and adjust PWM Intensity accordingly
    if (digitalRead(SWITCH_UP_PIN)==LOW) { 
      intensity   += CONTROL_INCREMENT; 
    } else if (digitalRead(SWITCH_DOWN_PIN)==LOW) { 
      intensity -= CONTROL_INCREMENT; 
    }
    intensity = constrain(intensity,0,255); // Sanity/Safety check: maintain intensity within constraints of PWM.



  // Read data from Serial Port, and process it if we have enough data (Newline Detected)
    while ( Serial.available() && (serial_buffer_head < SERIAL_BUFFER_LENGTH) && (!serial_data_ready) ) {
      in_char = Serial.read();
      serial_buffer[serial_buffer_head++] = in_char;
      serial_data_ready = (in_char == '\n');

      // We have a line of data, but we'll try to accomodate "CR/LF" ("\r\n") pairing if other end is using it.
      if (serial_data_ready) {
        in_char = Serial.peek();
        if ( (in_char == '\r') || (in_char == '\n') ) {
          serial_buffer[serial_buffer_head++] = Serial.read();
        }
      }
      
      serial_buffer[serial_buffer_head] = '\0'; // Always zero-terminate, so we can treat this as a String.
    };

  // If we have Serial Data ready, process buffer as a JSON command string
    if (serial_data_ready) {
      serialDebug("ECHO: %s",serial_buffer);

      json_message = aJson.parse(serial_buffer);
    
      // If we have a JSON Message, process it. NOTE: We ignore everything else, and ONLY yield a logStatus() message for valid commands.
      if (json_message) {
          char* command = aJson.getObjectItem(json_message, "command")->valuestring;
          serialDebug("JSON COMMAND RECEIVED %s\n", command);
    
          if (strcmp(command,"debug")==0) {
            char* new_state = aJson.getObjectItem(json_message, "state")->valuestring;
            debug = (strcmp(new_state,"on")==0);
            logStatus();
          } else if (strcmp(command,"getStatus")==0) {
            logStatus();
          } else if (strcmp(command,"lightSwitch")==0) {
            char* new_state = aJson.getObjectItem(json_message, "state")->valuestring;
            if (strcmp(new_state,"off")==0) {
              pwm_switch_state = OFF;
            } else if (strcmp(new_state,"on")==0) {
              pwm_switch_state  = ON;
            } else if (strcmp(new_state,"auto")==0) {
              pwm_switch_state = AUTO;
            };
            logStatus();
          } else if (strcmp(command,"setAmbientThreshold")==0) {
            ambient_threshold = constrain( aJson.getObjectItem(json_message, "threshold")->valueint, 0, 255);
            logStatus();
          } else if (strcmp(command,"setIntensity")==0) {
            intensity = constrain( aJson.getObjectItem(json_message, "intensity")->valueint, 0, 255);
            logStatus();
          };
          
          aJson.deleteItem( json_message ); // Clean up when finished : Don't leave aJson hanging with a dangling reference.
      }

      // This line of data is done. Reset the Serial Buffer to receive a new line.f
        serial_buffer_head = 0;
        serial_data_ready = false;
    }
}

