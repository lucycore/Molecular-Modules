// 在树莓派上运行OLED1.3 首先需要注意修改 pico内部的iic引脚映射，接着修改Adafruit_SSD1306.h文件，移除掉提示不存在的库文件
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string.h>

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3C
#define SSEQ_TIME_INDEX 10 //screen_sequence pixel life length
#define SSEQ_BASE_UNIT 3 //screen_sequence base unit
#define SSEQ_PIXS_NUM 2730 //screen_sequence max number of pixel
// SSEQ_PIXS_NUM, the first location is a arr [x,0,0], x direct how many data in the SSEQ_PIXS_NUM
#define WHITE SSD1306_WHITE
#define BLACK SSD1306_BLACK

uint8_t SCREEN_SEQUENCE[SSEQ_TIME_INDEX][SSEQ_PIXS_NUM][SSEQ_BASE_UNIT];
  //send_signal();
uint8_t SCREEN_TIME_INDEX = 0;
uint8_t *SCREEN_TIME_INDEX_P = &SCREEN_TIME_INDEX;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void print_pixel(uint8_t screen_sequence[][SSEQ_PIXS_NUM][SSEQ_BASE_UNIT], uint8_t *screen_time_index_p);
void draw_a_line(uint8_t screen_sequence[][SSEQ_PIXS_NUM][SSEQ_BASE_UNIT], uint8_t *screen_time_index_p, 
                uint8_t x_axis, uint8_t y_axis, uint8_t direction, uint8_t number, uint8_t delay = 1);
void draw_a_point(uint8_t screen_sequence[][SSEQ_PIXS_NUM][SSEQ_BASE_UNIT], uint8_t *screen_time_index_p, 
                uint8_t x_axis, uint8_t y_axis, uint8_t delay = 1);
uint8_t boundary_check(uint8_t x_axis, uint8_t y_axis);
void ordered_insert(uint8_t screen_sequence[][SSEQ_PIXS_NUM][SSEQ_BASE_UNIT],uint8_t screen_time_index,
                uint8_t *pix_data);
uint8_t add_screen_time_index(uint8_t screen_time_index);
void clean_screen_sequence(uint8_t screen_sequence[][SSEQ_PIXS_NUM][SSEQ_BASE_UNIT],uint8_t *screen_time_index_p);
void send_signal();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(WHITE);  // Draw white text
}

void loop() {
  // put your main code here, to run repeatedly:
  draw_a_line(SCREEN_SEQUENCE,SCREEN_TIME_INDEX_P,5,5,4,5,2);
  print_pixel(SCREEN_SEQUENCE,SCREEN_TIME_INDEX_P);
  delay(1000);
  print_pixel(SCREEN_SEQUENCE,SCREEN_TIME_INDEX_P);
  delay(1000);
  print_pixel(SCREEN_SEQUENCE,SCREEN_TIME_INDEX_P);
  delay(1000);
  print_pixel(SCREEN_SEQUENCE,SCREEN_TIME_INDEX_P);
  delay(1000);
  print_pixel(SCREEN_SEQUENCE,SCREEN_TIME_INDEX_P);
  delay(1000);
  print_pixel(SCREEN_SEQUENCE,SCREEN_TIME_INDEX_P);
  delay(1000);
  print_pixel(SCREEN_SEQUENCE,SCREEN_TIME_INDEX_P);
  delay(1000);
}

uint8_t boundary_check(uint8_t x_axis, uint8_t y_axis){
  if ((x_axis > SCREEN_WIDTH) && (x_axis < 0) &&
    (y_axis > SCREEN_HEIGHT) && (y_axis < 0)) return 0;
  return 1;
}

void draw_a_line(uint8_t screen_sequence[][SSEQ_PIXS_NUM][SSEQ_BASE_UNIT], uint8_t *screen_time_index_p, 
                uint8_t x_axis, uint8_t y_axis, uint8_t direction, uint8_t number, uint8_t delay)
{
  uint8_t sseq_data[] = {0,0,0}; //init
  for (int i=0; i<number; i++){
    switch (direction)
    {
    case 1:
      y_axis -= 1;
      break;
    case 2:
      x_axis += 1;
      y_axis -= 1;
      break;
    case 3:
      x_axis += 1;
      break;
    case 4:
      x_axis += 1;
      y_axis += 1;
      break;
    case 5:
      y_axis += 1;
      break;
    case 6:
      x_axis -= 1;
      y_axis += 1;
      break;
    case 7:
      x_axis -= 1;
      break;
    case 8:
      x_axis -= 1;
      y_axis -= 1;
      break;
    }
    if (boundary_check(x_axis,y_axis)){
      sseq_data[0] = x_axis;
      sseq_data[1] = y_axis;
      sseq_data[2] = 1;
      ordered_insert(screen_sequence,
                  add_screen_time_index(*screen_time_index_p+i),
                  sseq_data);
      sseq_data[2] = 0;
      ordered_insert(screen_sequence,
                  add_screen_time_index(*screen_time_index_p+delay+i),
                  sseq_data);
    }
  }
}

void clean_screen_sequence(uint8_t screen_sequence[][SSEQ_PIXS_NUM][SSEQ_BASE_UNIT],uint8_t *screen_time_index_p){
  memset(&screen_sequence[*screen_time_index_p],0,sizeof screen_sequence[*screen_time_index_p]);
}

void ordered_insert(uint8_t screen_sequence[][SSEQ_PIXS_NUM][SSEQ_BASE_UNIT],uint8_t screen_time_index,
                uint8_t *pix_data)
{ 
  int data_index = screen_sequence[screen_time_index][0][0];
  screen_sequence[screen_time_index][data_index+1][0] = pix_data[0];
  screen_sequence[screen_time_index][data_index+1][1] = pix_data[1];
  screen_sequence[screen_time_index][data_index+1][2] = pix_data[2];
  screen_sequence[screen_time_index][0][0] += 1;
}

uint8_t add_screen_time_index(uint8_t screen_time_index){
  if (screen_time_index > SSEQ_TIME_INDEX){
    return screen_time_index - SSEQ_TIME_INDEX;
  } else {
    return screen_time_index;
  }
}

void print_pixel(uint8_t screen_sequence[][SSEQ_PIXS_NUM][SSEQ_BASE_UNIT], uint8_t *screen_time_index_p){
  int data_index = screen_sequence[*screen_time_index_p][0][0];
  for (int i=1; i<=data_index; i++){
    if (screen_sequence[*screen_time_index_p][i][2]){
      display.drawPixel(
      screen_sequence[*screen_time_index_p][i][0],
      screen_sequence[*screen_time_index_p][i][1],
      WHITE);
    } else {
      display.drawPixel(
      screen_sequence[*screen_time_index_p][i][0],
      screen_sequence[*screen_time_index_p][i][1],
      BLACK);
    }
  }
  display.display();
  clean_screen_sequence(screen_sequence,screen_time_index_p);
  if (*screen_time_index_p == SSEQ_TIME_INDEX){
    *screen_time_index_p = 0;
  } else {
    *screen_time_index_p += 1;
  }
}

void draw_a_point(uint8_t screen_sequence[][SSEQ_PIXS_NUM][SSEQ_BASE_UNIT], uint8_t *screen_time_index_p, 
                uint8_t x_axis, uint8_t y_axis, uint8_t delay)
{
  uint8_t sseq_data[] = {x_axis,y_axis,1}; //light
  ordered_insert(screen_sequence,
                add_screen_time_index(*screen_time_index_p),
                sseq_data);
  sseq_data[2] = 0; //dark
  ordered_insert(screen_sequence,
                add_screen_time_index(*screen_time_index_p+delay),
                sseq_data);
}

void send_signal(){
  Serial.println("Program FINISH HERE");
  
}


//display.drawPixel(1,1,BLACK);
//display.setCursor(40, 30);
//display.println("HOWDY !");
//display.display();