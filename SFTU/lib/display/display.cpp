#include "display.hpp"

Display::Display() {}

Display::~Display() {}

bool Display::init(TwoWire &I2C_Bus) {
  display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_Bus, OLED_RESET);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    return false;
  }

  return true;
}

void Display::begin() {
  // Clear the buffer
  display.clearDisplay();
  display.display();

  drawIntroPage();

  // drawPageBar(false, false, false, false, false, 0, true);
}

void Display::drawData(float input1, float input2, float input3, float input4, float input5, float input6, float input7, float input8, bool updateDisp) {
  // Display 8 inputs in two columns of 4 rows each, smaller font
  uint8_t fontSize = 1;
  uint8_t numDigits = 5;  // e.g., "-12345"
  int16_t startX_left = 2;
  int16_t startX_right = SCREEN_WIDTH / 2 + 2;
  int16_t topMargin = 2;
  int16_t bottomMargin = 2;

  display.clearDisplay();
  display.setTextSize(fontSize);
  display.setTextColor(SSD1306_WHITE);

  int16_t inputs[8];
  inputs[0] = static_cast<int16_t>(round(constrain(input1, -99'999, 99'999)));
  inputs[1] = static_cast<int16_t>(round(constrain(input2, -99'999, 99'999)));
  inputs[2] = static_cast<int16_t>(round(constrain(input3, -99'999, 99'999)));
  inputs[3] = static_cast<int16_t>(round(constrain(input4, -99'999, 99'999)));
  inputs[4] = static_cast<int16_t>(round(constrain(input5, -99'999, 99'999)));
  inputs[5] = static_cast<int16_t>(round(constrain(input6, -99'999, 99'999)));
  inputs[6] = static_cast<int16_t>(round(constrain(input7, -99'999, 99'999)));
  inputs[7] = static_cast<int16_t>(round(constrain(input8, -99'999, 99'999)));

  // Use full display height
  int16_t availableHeight = SCREEN_HEIGHT - topMargin - bottomMargin;
  int16_t spacing = availableHeight / 4;

  for (uint8_t i = 0; i < 4; ++i) {
    // Left column: I1-I4
    int16_t x_left = startX_left;
    int16_t y = topMargin + i * spacing;
    display.setCursor(x_left, y);
    char bufL[8];
    sprintf(bufL, "%05d", abs(inputs[i]));
    display.print("I");
    display.print(i + 1);
    display.print("=");
    display.print(inputs[i] < 0 ? "-" : "+");
    display.print(bufL);
    display.print("U");

    // Right column: I5-I8
    int16_t x_right = startX_right;
    display.setCursor(x_right, y);
    char bufR[8];
    sprintf(bufR, "%05d", abs(inputs[i + 4]));
    display.print("I");
    display.print(i + 5);
    display.print("=");
    display.print(inputs[i + 4] < 0 ? "-" : "+");
    display.print(bufR);
    display.print("U");
  }

  if (updateDisp) {
    display.display();
  }
}

void Display::drawForce(float forceInput, bool updateDisp) {
  static uint8_t fontSize = 2;
  static uint8_t numDigits = 9;

  static int16_t cursorX = ((SCREEN_WIDTH - (numDigits * fontSize * 6)) / 2);
  static int16_t cursorY = ((SCREEN_HEIGHT - 20) / 2) + 20 - ((fontSize - 1) * 8);

  display.fillRect(0, 20, SCREEN_WIDTH, (SCREEN_HEIGHT - 20), SSD1306_BLACK);
  display.setTextSize(fontSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(cursorX, cursorY);

  int16_t force = static_cast<int16_t>(round(constrain(forceInput, -100'000, 100'000)));

  char buf[6];
  sprintf(buf, "%05d", abs(force));

  display.print("F=");  // best accuracy is about 10N, 0.05% FS (20kN)
  display.print(force < 0 ? "-" : "+");
  display.print(buf);
  display.println("N");

  if (updateDisp) {
    display.display();
  }
}

void Display::updateForce(float forceInput) { m_force = forceInput; }

void Display::drawPageBar(bool cell, bool sd, bool rf, bool armed, bool ready, float battPer, bool forceUpdateAll) {
  drawData(m_force, false);
  // Update Cell icon if changed
  if ((cell != m_mainPageStatus.cell) || forceUpdateAll) {
    display.fillRect(0, 0, 16, 16, SSD1306_BLACK);
    drawBitMap(cell ? load_cell_on : load_cell_off, 0, 0);
    m_mainPageStatus.cell = cell;
  }

  // Update SD icon if changed
  if ((sd != m_mainPageStatus.sd) || forceUpdateAll) {
    display.fillRect(16, 0, 16, 16, SSD1306_BLACK);
    drawBitMap(sd ? SD_on : SD_off, 16, 0);
    m_mainPageStatus.sd = sd;
  }

  // Update RF icon if changed
  if ((rf != m_mainPageStatus.rf) || forceUpdateAll) {
    display.fillRect(32, 0, 16, 16, SSD1306_BLACK);
    drawBitMap(rf ? RF_on : RF_off, 32, 0);
    m_mainPageStatus.rf = rf;
  }

  // Update Armed icon if changed
  if ((armed != m_mainPageStatus.armed) || forceUpdateAll) {
    display.fillRect(48, 0, 16, 16, SSD1306_BLACK);
    drawBitMap(armed ? armed_on : armed_off, 48, 0);
    m_mainPageStatus.armed = armed;
  }

  // Update Ready icon if changed
  if ((ready != m_mainPageStatus.ready) || forceUpdateAll) {
    display.fillRect(64, 0, 16, 16, SSD1306_BLACK);
    drawBitMap(ready ? ready_on : ready_off, 64, 0);
    m_mainPageStatus.ready = ready;
  }

  // Update Battery icon if the displayed icon (rounded to nearest 10) is
  // different
  if ((((int)(battPer + 5) / 10) != ((int)(m_mainPageStatus.battPer + 5) / 10)) || forceUpdateAll) {
    // Determine battery icon based on battPer
    Bitmap newBattIcon;
    if (battPer < 10)
      newBattIcon = batt_0;
    else if (battPer < 20)
      newBattIcon = batt_10;
    else if (battPer < 30)
      newBattIcon = batt_20;
    else if (battPer < 40)
      newBattIcon = batt_30;
    else if (battPer < 50)
      newBattIcon = batt_40;
    else if (battPer < 60)
      newBattIcon = batt_50;
    else if (battPer < 70)
      newBattIcon = batt_60;
    else if (battPer < 80)
      newBattIcon = batt_70;
    else if (battPer < 90)
      newBattIcon = batt_80;
    else if (battPer < 100)
      newBattIcon = batt_90;
    else
      newBattIcon = batt_100;

    display.fillRect(80, 0, display.width() - 80, 16, SSD1306_BLACK);
    drawBitMap(newBattIcon, 80, 0);
    m_mainPageStatus.battPer = battPer;
  }

  if (forceUpdateAll) {
    display.fillRect(0, 17, display.width(), 2, SSD1306_BLACK);
    display.fillRect(0, 17, display.width(), 2, SSD1306_WHITE);
  }

  // Finally, update the display with all drawn changes.
  display.display();
}

void Display::drawIntroPage() {
  display.clearDisplay();

  display.drawBitmap((display.width() - Sunburnlogo.width) / 2, (display.height() - Sunburnlogo.height) / 2, Sunburnlogo.bmp, Sunburnlogo.width, Sunburnlogo.height, 1);

  display.display();

  for (int i = 0; i < 6; i++) {
    vTaskDelay(pdMS_TO_TICKS(100));
    display.invertDisplay(true);
    vTaskDelay(pdMS_TO_TICKS(100));
    display.invertDisplay(false);
  }
  vTaskDelay(pdMS_TO_TICKS(1000));
  display.clearDisplay();
  display.display();
}

void Display::drawBitMap(Bitmap data, int16_t x, int16_t y) {
  if (x == -1) {
    x = (display.width() - data.width) / 2;
  }

  if (y == -1) {
    y = (display.height() - data.height) / 2;
  }

  display.drawBitmap(x, y, data.bmp, data.width, data.height, 1);
  display.display();
}

void Display::showSuccess(String msg) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println(msg + " Success!");

  display.display();
  vTaskDelay(pdMS_TO_TICKS(500));
  display.clearDisplay();
  display.display();
}

void Display::showError(String msg, uint16_t duration_ms) {
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(msg);

  display.display();
  vTaskDelay(pdMS_TO_TICKS(duration_ms));
  display.clearDisplay();
  display.display();
}