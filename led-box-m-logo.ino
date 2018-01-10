#include "FastLED.h"

// How many leds in your strip?
#define NUM_LEDS_1 150
#define NUM_LEDS_2 98

#define NUM_LEDS NUM_LEDS_1 + NUM_LEDS_2

#define LEDS_PER_ROW 8
#define NUM_ROWS 31


// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN_1 7
#define DATA_PIN_2 8


#define KNOB_PIN A1


#define MAX_ENERGY 999

CRGB leds[NUM_LEDS];

int last_val = -1;


const int numReadings = 10;
float readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
float total = 0;                  // the running total
float average = 0;                // the average

float currentValue = 0;
float energy = 0;

int val;

long max_energy = MAX_ENERGY;

unsigned long last_twinkle = 0;
unsigned char hue = 0;
int run_time = 0;
unsigned long last_read = 0;
unsigned long last_draw = 0;

int dt = 0;

bool running = false;

const byte M_LOGO[8] = 
{
  B00000000,
  B01000010,
  B11100111,
  B11111111,
  B11011011,
  B11000011,
  B11000011,
  B11000011
};

void fadeAll()  {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);

  }
  FastLED.show();
}
void light_row(int row, CRGB colour) {

  if (row > NUM_ROWS) {
    return;
  }

  for (int i = 0; i <  LEDS_PER_ROW; i++) {
    int pixel = (LEDS_PER_ROW * row) + i;
    leds[NUM_LEDS - pixel - 1] = colour;
  }

}






void setup() {

  Serial.begin(115200);
  Serial.println("ON");

  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812, DATA_PIN_1, RGB>(leds, 0, NUM_LEDS_1);
  FastLED.addLeds<WS2812, DATA_PIN_2, RGB>(leds, NUM_LEDS_1, NUM_LEDS_2);

  for (int i = 0; i < NUM_ROWS; i++) {
    hue = map(i, 0, NUM_ROWS, 0, 160);
    light_row(i, CHSV(hue, 255, 255));

  }
  FastLED.show();

  //unsigned long start_time = millis();
  while (leds[0].b > 2) {
    fadeAll();
    delay(30);
  }

  // initialise array

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  //allOff();
  last_read = millis();
}



void readADC() {

  dt = (millis() - last_read);
  last_read = millis();

  total = total - readings[index];
  readings[index] = map(analogRead(A0), 0, 1024, 0, 60); //Raw data reading

  total = total + readings[index];
  index = index + 1;
  if (index >= numReadings) {
    index = 0;
  }

  average = total / numReadings; //Smoothing algorithm (http://www.arduino.cc/en/Tutorial/Smoothing)

  if (average != currentValue) {
    Serial.println(average);
  }

  currentValue = average;



  if (average > 2) {
    if (!running) {
      running = true;
      Serial.println("running");
    }
    energy += ((average * dt) / 1000);
    run_time += dt;


  }
  else {



    if (energy <= 0) {
      if (running) {
        running = false;
        Serial.println("stopped");
        energy = 0;
        last_twinkle = millis() + 5000;
      }

    } else {
      energy -= 3;
      run_time -= dt;
    }

    //energy -= 100;
  }

  if(energy > max_energy){
    energy = max_energy + 1;
  }

  if (running)
    Serial.println(energy);



}

void show_logo(){
  
}


void colours() {
  hue += 30;
  CRGB col = CHSV(hue, 255, 255);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = col;
  }
  FastLED.show();
}



void draw_lines() {


  if (millis() - last_draw >  300) {

    //max_energy = int(map(analogRead(KNOB_PIN), 2, 1024, 0, MAX_ENERGY));

    if (energy > max_energy) {
      colours();
    } else {
      val = map(energy, 0, max_energy, 0, NUM_ROWS);
      val = constrain(val, 0, 31);

      //last_twinkle = millis() - 3000;
      Serial.print("val\t");
      Serial.println(val);

      for (int i = 0; i < NUM_ROWS; i++) {
        hue = 160 - map(i, 0, NUM_ROWS, 0, 160);
        if (i <= val) {
          light_row(i, CHSV(hue, 255, 255));
        } else {
          light_row(i, CRGB::Black);

        }

      }
      FastLED.show();
    }

    last_draw = millis();

  }
}

void twinkle() {
  fadeAll();
  if (millis() - last_twinkle > 1000) {
    hue += 30;
    leds[random(NUM_LEDS)] = CHSV(hue, 255, 255);
    last_twinkle = millis();

  }
  FastLED.show();

}

int matrix_row = NUM_ROWS;
int matrix_col = random(LEDS_PER_ROW);

void matrix() {

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(240);
  }

  if (matrix_row <= 0) {
    matrix_row = NUM_ROWS;
    matrix_col = random(LEDS_PER_ROW);
    hue += 42;
  }

  if (millis() - last_twinkle > 100) {
    last_twinkle = millis();
    matrix_row--;
    int pixel = ((LEDS_PER_ROW) * (matrix_row) ) + matrix_col;

    if (matrix_row % 2) {
      pixel  = ((LEDS_PER_ROW) * (matrix_row) ) + (LEDS_PER_ROW -1 - matrix_col);
      //pixel += 1;
    }
    //    Serial.print(matrix_row);
    //    Serial.print("\t");
    //    Serial.print(matrix_col);
    //    Serial.print("\t");
    //    Serial.println(pixel);
    //
    //leds[NUM_LEDS - pixel - 1] = CRGB::Green;
    leds[NUM_LEDS - pixel - 1] = CHSV(hue, 255, 255);

    FastLED.show();

  }




}


void loop() {
  readADC();

  if (!running) {
    twinkle();
    //matrix();
  } else {
    draw_lines();
  }

  delay(30);

}


