#include "FastLED.h"
#include "digits.h"
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


#define MAX_ENERGY 500

#define START_RABBIT_POS 22
#define START_SNAKE_LENGTH 12


CRGB leds[NUM_LEDS];

int last_val = -1;

enum {  WAITING, COUNT_DOWN, START_GAME, PLAYING, WIN, LOSE, END_GAME };

int state = WAITING;

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

int count = 0;

//Game variables
int snake_length = 3;
int rabbit_pos = 22;
unsigned long game_time;
int level = 1;
int LEVEL_TIME[] = {55000, 50000, 48000, 45000, 40000, 15000, 14000, 13000, 12000, 10000};

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

void show_logo() {
  count = random(23) * 8;
  for (int j = 0; j < 8; j++) {
    byte line = M_LOGO[j];

    //Serial.println(line);

    for (byte mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask

      if (line & mask) {
        leds[count] = CRGB::Red;
        //Serial.print("*");
      } else {
        //Serial.print(" ");
        leds[count] = CRGB::Black;
      }
      count++;
    }
    ///Serial.println();
  }
  FastLED.show();
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

  show_logo();

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
  readings[index] = map(analogRead(A3), 0, 1024, 0, 60); //Raw data reading

  total = total + readings[index];
  index = index + 1;
  if (index >= numReadings) {
    index = 0;
  }

  average = total / numReadings; //Smoothing algorithm (http://www.arduino.cc/en/Tutorial/Smoothing)

  if (average != currentValue) {
    //Serial.println(average);
  }

  currentValue = average;



  if (average > 2) {
    if (!running) {
      running = true;
      //Serial.println("0 running");
    }
    energy += ((average * dt) / 1000);
    run_time += dt;


  }
  else {



    if (energy <= 0) {
      if (running) {
        running = false;
        //Serial.println("0 stopped");
        energy = 0;
        last_twinkle = millis() + 5000;
      }

    } else {
      //energy -= 10;
      run_time -= dt;
    }

    //energy -= 100;
  }

  if (energy > max_energy) {
    energy = max_energy + 1;
  }

  //if (running)
  //  Serial.println(energy);



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
  //fadeAll();
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
      pixel  = ((LEDS_PER_ROW) * (matrix_row) ) + (LEDS_PER_ROW - 1 - matrix_col);
      //pixel += 1;
    }
    leds[NUM_LEDS - pixel - 1] = CHSV(hue, 255, 255);

    FastLED.show();

  }


}

void show_digit(int digit) {
  //FastLED.clear();
  int start_row = 8;
  for (int row = 0; row < 8; row++) {
    byte line = DIGITS[digit][row];

    //Serial.println(line);
    int col = 0;
    for (byte mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
      //int pixel = ((start_row + row) * 8) +  col;
      int pixel  = ((start_row + row) * 8) + (7 - col);

      if ((start_row + row) % 2) {
        //pixel  = ((start_row + row) * 8) + (7 - col);
        pixel = ((start_row + row) * 8) +  col;//pixel += 1;
      }

      if (line & mask) {
        leds[pixel] = CRGB::Red;
        //Serial.print("*");
      } else {
        //Serial.print(" ");
        leds[pixel] = CRGB::Black;
      }
      //Serial.println(pixel);

      col++;
    }
    //Serial.println();
  }
  FastLED.show();
}



//Show 3 to 1 count_down
void count_down() {
  FastLED.clear();
  // FastLED.show();
  show_digit(3);
  delay(1000);
  show_digit(2);
  delay(1000);
  show_digit(1);
  delay(1000);
  FastLED.clear();
  FastLED.show();


}

void draw_game() {

  FastLED.clear();

  //Draw Snake
  for (int i = 0; i < snake_length; i++) {
    leds[NUM_LEDS - 1 - i] = CRGB::Green;
  }

  //Draw Player
  leds[NUM_LEDS - 1 - rabbit_pos] = CRGB::Blue;

  FastLED.show();
  last_draw = millis();
}


unsigned long fade_time = millis();
unsigned long last_run = millis();
int prev_snake = -1;
int prev_rabbit = -1;

void loop() {

  switch (state) {
  case WAITING:
    readADC();

    if (!running) {
      fadeAll();
      if (millis() - fade_time > 5000) {
        show_logo();
        fade_time = millis();
      }

      //twinkle();
      //matrix();
    } else {
      if (millis() - last_run > 10000) {
        state = COUNT_DOWN;
      }
    }

    delay(30);

    break;
  case COUNT_DOWN:
    Serial.println("0 Count Down");

    count_down();
    state = START_GAME;

    break;
  case START_GAME:
    energy = 0;
    rabbit_pos = START_RABBIT_POS;
    snake_length = START_SNAKE_LENGTH;
    state = PLAYING;
    game_time = millis();
    last_read = millis();
    break;

  case PLAYING:
    readADC();

    snake_length = map(energy, 0, MAX_ENERGY, START_SNAKE_LENGTH, NUM_LEDS - 1);
    rabbit_pos = map(millis() - game_time, 0, LEVEL_TIME[level], START_RABBIT_POS, NUM_LEDS - 1);
    if (snake_length != prev_snake) {
      Serial.print("Snake ");
      Serial.print(snake_length);
      Serial.print(" Energy: ");
      Serial.println(energy);

    }

    if (snake_length >= rabbit_pos) {
      state = WIN;
      Serial.print("Level: ");
      Serial.print(level);
      Serial.print(" Snake: ");
      Serial.print(snake_length);
      Serial.print(" Rabbit: ");
      Serial.print(rabbit_pos);
      Serial.print(" Energy: ");
      Serial.println(energy);


    }
    else if (rabbit_pos >= NUM_LEDS - 1) {
      state = LOSE;
    } else {
      draw_game();
    }

    delay(30);
    break;

  case WIN:
    //Serial.print("0 Win ");
    //Serial.println(level);
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Green;
    }
    FastLED.show();
    while (leds[0].g > 0) {
      fadeAll();
      delay(5);
    }
    level++;
    if (level > 9) {
      state = END_GAME;
      break;
    }
    show_digit(level);
    delay(500);
    FastLED.clear();
    energy = 0;
    state = START_GAME;
    //rabbit_pos = START_PLAYER_POS;
    //snake_length = START_SNAKE_LENGTH;
    break;

  case LOSE:
    Serial.println("0 Lose");

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Red;
    }

    FastLED.show();
    while (leds[0].r > 0) {
      fadeAll();
      delay(30);
    }
    state = END_GAME;
    break;

  case END_GAME:
    energy = 0;
    level = 0;
    FastLED.clear();
    FastLED.show();
    last_run = millis();
    running = false;
    //delay(3000);
    state = WAITING;
    break;
  }


}


