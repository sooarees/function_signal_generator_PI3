#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

// Estilo TRON: preto, azul eletrico e linhas de circuito.
// Prototipo de interface para Arduino UNO + MCUFRIEND 2.4" TFT Shield.
// Display reserva: LGDP4532 / ID 0x4532.

#define LCD_ID 0x4532

const int XP = 6;
const int XM = A2;
const int YP = A1;
const int YM = 7;

const int TS_LEFT = 186;
const int TS_RT   = 889;
const int TS_TOP  = 186;
const int TS_BOT  = 876;

const int MINPRESSURE = 180;
const int MAXPRESSURE = 1000;

MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

const int16_t SCR_W = 320;
const int16_t SCR_H = 240;

#define BLACK   0x0000
#define WHITE   0xFFFF
#define GREY    0x4228
#define LTGREY  0x867F
#define DKBLUE  0x008C
#define BLUE    0x025F
#define CYAN    0x07FF
#define YELLOW  0x5FFF
#define ORANGE  0x04FF

enum Waveform {
  WAVE_SINE = 0,
  WAVE_SQUARE,
  WAVE_TRIANGLE,
  WAVE_SAW
};

enum Param {
  PARAM_FREQ = 0,
  PARAM_AMP,
  PARAM_OFFSET,
  PARAM_DUTY
};

enum Screen {
  SCREEN_HOME = 0,
  SCREEN_ADJUST
};

struct Button {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
};

bool inside(Button b, int16_t x, int16_t y);
bool insideTouch(Button b, int16_t x, int16_t y);
void restoreTouchPins();
bool readTouch(int16_t *x, int16_t *y);
void drawTronGrid();
void drawHeader(const __FlashStringHelper *title);
void printTenths(int16_t value);
void printFreqValue(bool compact);
uint8_t valueCharCount(Param param, bool compact);
void printParamValue(Param param, bool compact);
int32_t getParamValue(Param param);
void setParamValue(Param param, int32_t value);
const __FlashStringHelper *waveName(Waveform w);
const __FlashStringHelper *paramName(Param param);
int16_t sineApprox(uint8_t phase);
int16_t previewValue(Waveform w, uint8_t phase);
void drawMiniWave(Button b, Waveform w, bool selected);
void drawWaveButtons();
void drawParamButton(Param param);
void drawHomeScreen();
int16_t valueToSliderPos();
void setValueFromSlider(int16_t touchX);
void drawSlider();
void drawAdjustButton(Button b, const __FlashStringHelper *label, uint16_t fill);
void drawAdjustScreen(Param param);
void updateAdjustValue();
void openAdjust(Param param);
void cancelAdjust();
void saveAdjust();
void nudgeActiveParam(int8_t direction);
void handleHomeTouch(int16_t x, int16_t y);
void handleAdjustTouch(int16_t x, int16_t y);

Waveform waveform = WAVE_SINE;
Screen currentScreen = SCREEN_HOME;
Param activeParam = PARAM_FREQ;
int32_t savedAdjustValue = 0;

uint16_t freqHz = 1000;
uint8_t ampTenthVpp = 50;
int8_t offsetTenthV = 0;
uint8_t dutyPct = 50;

const int8_t sineQuarter[33] = {
  0, 5, 10, 15, 20, 24, 29, 34, 38, 43, 47,
  51, 56, 60, 63, 67, 71, 74, 77, 80, 83,
  86, 88, 90, 92, 94, 96, 97, 98, 99, 100,
  100, 100
};

Button waveButtons[4] = {
  {8, 30, 72, 60},
  {84, 30, 72, 60},
  {160, 30, 72, 60},
  {236, 30, 76, 60}
};

Button paramButtons[4] = {
  {8, 102, 148, 56},
  {164, 102, 148, 56},
  {8, 174, 148, 56},
  {164, 174, 148, 56}
};

Button cancelButton = {16, 198, 132, 34};
Button saveButton = {172, 198, 132, 34};
Button minusButton = {16, 120, 44, 44};
Button plusButton = {260, 120, 44, 44};

const int16_t SLIDER_X = 74;
const int16_t SLIDER_Y = 132;
const int16_t SLIDER_W = 172;
const int16_t SLIDER_H = 20;

unsigned long lastTouchMs = 0;

bool inside(Button b, int16_t x, int16_t y) {
  return x >= b.x && x < (b.x + b.w) && y >= b.y && y < (b.y + b.h);
}

bool insideTouch(Button b, int16_t x, int16_t y) {
  const int16_t margin = 6;
  return x >= (b.x - margin) && x < (b.x + b.w + margin) &&
         y >= (b.y - margin) && y < (b.y + b.h + margin);
}

void restoreTouchPins() {
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
}

bool readTouch(int16_t *x, int16_t *y) {
  TSPoint p = ts.getPoint();
  restoreTouchPins();

  if (p.z < MINPRESSURE || p.z > MAXPRESSURE) {
    return false;
  }

  // Paisagem: 320 x 240.
  *x = map(p.y, TS_TOP, TS_BOT, 0, SCR_W);
  *y = map(p.x, TS_RT, TS_LEFT, 0, SCR_H);
  *x = constrain(*x, 0, SCR_W - 1);
  *y = constrain(*y, 0, SCR_H - 1);
  return true;
}

void drawTronGrid() {
  for (int16_t y = 36; y < SCR_H; y += 48) {
    tft.drawFastHLine(0, y, SCR_W, DKBLUE);
  }

  for (int16_t x = 28; x < SCR_W; x += 58) {
    tft.drawFastVLine(x, 24, SCR_H - 24, DKBLUE);
  }

  tft.drawLine(0, 239, 62, 178, BLUE);
  tft.drawLine(319, 239, 258, 178, BLUE);
}

void drawHeader(const __FlashStringHelper *title) {
  tft.fillRect(0, 0, SCR_W, 24, BLUE);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(8, 5);
  tft.print(title);
}

void printTenths(int16_t value) {
  if (value < 0) {
    tft.print('-');
    value = -value;
  }

  tft.print(value / 10);
  tft.print('.');
  tft.print(value % 10);
}

void printFreqValue(bool compact) {
  if (compact && freqHz >= 1000) {
    tft.print(freqHz / 1000);
    if ((freqHz % 1000) != 0) {
      tft.print('.');
      tft.print((freqHz % 1000) / 100);
    }
    tft.print(F("kHz"));
  } else {
    tft.print(freqHz);
    tft.print(F("Hz"));
  }
}

uint8_t valueCharCount(Param param, bool compact) {
  if (param == PARAM_FREQ) {
    if (compact && freqHz >= 1000) {
      uint8_t count = freqHz >= 10000 ? 2 : 1;
      if ((freqHz % 1000) != 0) count += 2;
      return count + 3;
    }

    if (freqHz >= 10000) return 7;
    if (freqHz >= 1000) return 6;
    if (freqHz >= 100) return 5;
    if (freqHz >= 10) return 4;
    return 3;
  }

  if (param == PARAM_AMP) {
    return ampTenthVpp >= 100 ? 7 : 6;
  }

  if (param == PARAM_OFFSET) {
    return offsetTenthV == 0 ? 4 : 5;
  }

  if (dutyPct >= 100) return 4;
  if (dutyPct >= 10) return 3;
  return 2;
}

void printParamValue(Param param, bool compact) {
  if (param == PARAM_FREQ) {
    printFreqValue(compact);
  } else if (param == PARAM_AMP) {
    printTenths(ampTenthVpp);
    tft.print(F("Vpp"));
  } else if (param == PARAM_OFFSET) {
    if (offsetTenthV > 0) tft.print('+');
    printTenths(offsetTenthV);
    tft.print(F("V"));
  } else {
    tft.print(dutyPct);
    tft.print(F("%"));
  }
}

int32_t getParamValue(Param param) {
  if (param == PARAM_FREQ) return freqHz;
  if (param == PARAM_AMP) return ampTenthVpp;
  if (param == PARAM_OFFSET) return offsetTenthV;
  return dutyPct;
}

void setParamValue(Param param, int32_t value) {
  if (param == PARAM_FREQ) {
    freqHz = constrain(value, 1, 20000);
  } else if (param == PARAM_AMP) {
    ampTenthVpp = constrain(value, 0, 100);
  } else if (param == PARAM_OFFSET) {
    offsetTenthV = constrain(value, -50, 50);
  } else {
    dutyPct = constrain(value, 0, 100);
  }
}

const __FlashStringHelper *waveName(Waveform w) {
  if (w == WAVE_SQUARE) return F("Quadrada");
  if (w == WAVE_TRIANGLE) return F("Triang.");
  if (w == WAVE_SAW) return F("Dente");
  return F("Senoide");
}

const __FlashStringHelper *paramName(Param param) {
  if (param == PARAM_AMP) return F("Amplitude");
  if (param == PARAM_OFFSET) return F("Offset");
  if (param == PARAM_DUTY) return F("Duty Cycle");
  return F("Frequencia");
}

int16_t sineApprox(uint8_t phase) {
  uint8_t quadrant = phase >> 6;
  uint8_t local = phase & 0x3F;

  if (quadrant == 1 || quadrant == 3) {
    local = 63 - local;
  }

  uint8_t index = local >> 1;
  uint8_t frac = local & 0x01;
  int16_t a = sineQuarter[index];
  int16_t b = sineQuarter[index + 1];
  int16_t value = a + (((b - a) * frac) / 2);

  if (quadrant >= 2) {
    value = -value;
  }

  return value;
}

int16_t previewValue(Waveform w, uint8_t phase) {
  if (w == WAVE_SQUARE) {
    return phase < 128 ? 100 : -100;
  }

  if (w == WAVE_TRIANGLE) {
    if (phase < 128) return map(phase, 0, 127, -100, 100);
    return map(phase, 128, 255, 100, -100);
  }

  if (w == WAVE_SAW) {
    return map(phase, 0, 255, -100, 100);
  }

  return sineApprox(phase);
}

void drawMiniWave(Button b, Waveform w, bool selected) {
  uint16_t border = selected ? CYAN : LTGREY;
  uint16_t fill = selected ? DKBLUE : BLACK;
  const int16_t margin = 7;
  int16_t left = b.x + margin;
  int16_t top = b.y + margin;
  int16_t width = b.w - (2 * margin);
  int16_t height = b.h - (2 * margin);
  int16_t mid = top + height / 2;

  tft.fillRect(b.x, b.y, b.w, b.h, fill);
  tft.drawRect(b.x, b.y, b.w, b.h, border);

  uint8_t points = width * 2;
  int16_t lastX = left;
  int16_t lastY = mid;

  for (uint8_t i = 0; i < points; i++) {
    uint8_t phase = ((uint16_t)i * 255) / (points - 1);
    int16_t value = previewValue(w, phase);
    int16_t px = left + (((uint16_t)i * (width - 1)) / (points - 1));
    int16_t py = mid - map(value, -120, 120, -(height / 2 - 2), height / 2 - 2);
    py = constrain(py, top + 2, top + height - 3);

    if (i > 0) {
      tft.drawLine(lastX, lastY, px, py, YELLOW);
    }

    lastX = px;
    lastY = py;

    if (w == WAVE_SAW && i == points - 1) {
      int16_t lowY = mid - map(previewValue(w, 0), -120, 120, -(height / 2 - 2), height / 2 - 2);
      lowY = constrain(lowY, top + 2, top + height - 3);
      tft.drawLine(px, py, px, lowY, YELLOW);
    }
  }
}

void drawWaveButtons() {
  drawMiniWave(waveButtons[0], WAVE_SINE, waveform == WAVE_SINE);
  drawMiniWave(waveButtons[1], WAVE_SQUARE, waveform == WAVE_SQUARE);
  drawMiniWave(waveButtons[2], WAVE_TRIANGLE, waveform == WAVE_TRIANGLE);
  drawMiniWave(waveButtons[3], WAVE_SAW, waveform == WAVE_SAW);
}

void drawParamButton(Param param) {
  Button b = paramButtons[param];
  bool enabled = param != PARAM_DUTY || waveform == WAVE_SQUARE;
  uint16_t border = enabled ? LTGREY : GREY;
  uint16_t text = enabled ? WHITE : GREY;

  tft.fillRect(b.x, b.y, b.w, b.h, BLACK);
  tft.drawRect(b.x, b.y, b.w, b.h, border);

  tft.setTextColor(LTGREY);
  tft.setTextSize(2);
  tft.setCursor(b.x + 8, b.y + 6);
  tft.print(paramName(param));

  tft.setTextColor(text);
  tft.setTextSize(2);
  tft.setCursor(b.x + 8, b.y + 33);
  printParamValue(param, true);
}

void drawHomeScreen() {
  currentScreen = SCREEN_HOME;
  tft.fillScreen(BLACK);
  drawTronGrid();
  drawHeader(F("Gerador de Funcoes"));
  drawWaveButtons();
  drawParamButton(PARAM_FREQ);
  drawParamButton(PARAM_AMP);
  drawParamButton(PARAM_OFFSET);
  drawParamButton(PARAM_DUTY);
}

int16_t valueToSliderPos() {
  uint16_t half = SLIDER_W / 2;

  if (activeParam == PARAM_FREQ) {
    if (freqHz <= 1000) {
      return map(freqHz, 1, 1000, 0, half);
    }
    return map(freqHz, 1000, 20000, half, SLIDER_W);
  }

  if (activeParam == PARAM_AMP) {
    return map(ampTenthVpp, 0, 100, 0, SLIDER_W);
  }

  if (activeParam == PARAM_OFFSET) {
    return map(offsetTenthV, -50, 50, 0, SLIDER_W);
  }

  return map(dutyPct, 0, 100, 0, SLIDER_W);
}

void setValueFromSlider(int16_t touchX) {
  int16_t pos = constrain(touchX - SLIDER_X, 0, SLIDER_W);
  uint16_t half = SLIDER_W / 2;

  if (activeParam == PARAM_FREQ) {
    if (pos <= half) {
      freqHz = map(pos, 0, half, 1, 1000);
    } else {
      int32_t raw = map(pos, half, SLIDER_W, 1000, 20000);
      freqHz = constrain(((raw + 50) / 100) * 100, 1000, 20000);
    }
  } else if (activeParam == PARAM_AMP) {
    ampTenthVpp = map(pos, 0, SLIDER_W, 0, 100);
  } else if (activeParam == PARAM_OFFSET) {
    offsetTenthV = map(pos, 0, SLIDER_W, -50, 50);
  } else {
    int16_t raw = map(pos, 0, SLIDER_W, 0, 100);
    dutyPct = constrain(((raw + 1) / 3) * 3, 0, 100);
  }
}

void drawSlider() {
  int16_t pos = valueToSliderPos();
  int16_t knobX = SLIDER_X + pos;

  tft.fillRect(SLIDER_X - 8, SLIDER_Y - 18, SLIDER_W + 16, 58, BLACK);
  tft.drawRect(SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H, LTGREY);
  tft.fillRect(SLIDER_X + 2, SLIDER_Y + 7, SLIDER_W - 4, 6, GREY);
  tft.fillRect(SLIDER_X + 2, SLIDER_Y + 7, pos, 6, CYAN);
  tft.fillRect(knobX - 5, SLIDER_Y - 8, 11, SLIDER_H + 16, YELLOW);
}

void drawAdjustButton(Button b, const __FlashStringHelper *label, uint16_t fill) {
  tft.fillRect(b.x, b.y, b.w, b.h, fill);
  tft.drawRect(b.x, b.y, b.w, b.h, LTGREY);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(b.x + 12, b.y + 9);
  tft.print(label);
}

void drawAdjustScreen(Param param) {
  activeParam = param;
  currentScreen = SCREEN_ADJUST;
  savedAdjustValue = getParamValue(param);

  tft.fillScreen(BLACK);
  drawTronGrid();
  drawHeader(paramName(param));

  updateAdjustValue();
  drawSlider();

  tft.fillRect(minusButton.x, minusButton.y, minusButton.w, minusButton.h, DKBLUE);
  tft.drawRect(minusButton.x, minusButton.y, minusButton.w, minusButton.h, LTGREY);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(minusButton.x + 14, minusButton.y + 9);
  tft.print('-');

  tft.fillRect(plusButton.x, plusButton.y, plusButton.w, plusButton.h, DKBLUE);
  tft.drawRect(plusButton.x, plusButton.y, plusButton.w, plusButton.h, LTGREY);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(plusButton.x + 13, plusButton.y + 9);
  tft.print('+');

  drawAdjustButton(cancelButton, F("Cancelar"), BLACK);
  drawAdjustButton(saveButton, F("Salvar"), BLUE);
}

void updateAdjustValue() {
  uint8_t chars = valueCharCount(activeParam, false);
  int16_t textW = chars * 18;
  int16_t x = (SCR_W - textW) / 2;

  tft.fillRect(0, 62, SCR_W, 48, BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(x, 78);
  printParamValue(activeParam, false);
}

void openAdjust(Param param) {
  if (param == PARAM_DUTY && waveform != WAVE_SQUARE) {
    return;
  }
  drawAdjustScreen(param);
}

void cancelAdjust() {
  setParamValue(activeParam, savedAdjustValue);
  drawHomeScreen();
}

void saveAdjust() {
  drawHomeScreen();
}

void nudgeActiveParam(int8_t direction) {
  if (activeParam == PARAM_FREQ) {
    int16_t step = freqHz < 1000 ? 1 : 100;
    setParamValue(activeParam, (int32_t)freqHz + ((int32_t)direction * step));
  } else if (activeParam == PARAM_AMP) {
    setParamValue(activeParam, (int16_t)ampTenthVpp + direction);
  } else if (activeParam == PARAM_OFFSET) {
    setParamValue(activeParam, (int16_t)offsetTenthV + direction);
  } else {
    setParamValue(activeParam, (int16_t)dutyPct + (direction * 3));
  }

  updateAdjustValue();
  drawSlider();
}

void handleHomeTouch(int16_t x, int16_t y) {
  for (uint8_t i = 0; i < 4; i++) {
    if (insideTouch(waveButtons[i], x, y)) {
      waveform = (Waveform)i;
      drawWaveButtons();
      drawParamButton(PARAM_DUTY);
      return;
    }
  }

  for (uint8_t i = 0; i < 4; i++) {
    if (insideTouch(paramButtons[i], x, y)) {
      openAdjust((Param)i);
      return;
    }
  }
}

void handleAdjustTouch(int16_t x, int16_t y) {
  if (insideTouch(cancelButton, x, y)) {
    cancelAdjust();
    lastTouchMs = millis();
    return;
  }

  if (insideTouch(saveButton, x, y)) {
    saveAdjust();
    lastTouchMs = millis();
    return;
  }

  if (insideTouch(minusButton, x, y)) {
    nudgeActiveParam(-1);
    return;
  }

  if (insideTouch(plusButton, x, y)) {
    nudgeActiveParam(1);
    return;
  }

  Button sliderTouch = {SLIDER_X - 10, SLIDER_Y - 24, SLIDER_W + 20, 68};
  if (inside(sliderTouch, x, y)) {
    setValueFromSlider(x);
    updateAdjustValue();
    drawSlider();
  }
}

void setup() {
  Serial.begin(9600);

  tft.reset();
  tft.begin(LCD_ID);
  tft.setRotation(1);

  drawHomeScreen();
  Serial.println(F("Interface Arduino pronta. Display ID 0x4532."));
}

void loop() {
  int16_t x;
  int16_t y;

  if (readTouch(&x, &y)) {
    unsigned long now = millis();
    unsigned long delayMs = currentScreen == SCREEN_ADJUST ? 45 : 110;

    if (now - lastTouchMs > delayMs) {
      lastTouchMs = now;

      if (currentScreen == SCREEN_ADJUST) {
        handleAdjustTouch(x, y);
      } else {
        handleHomeTouch(x, y);
      }
    }
  }
}
