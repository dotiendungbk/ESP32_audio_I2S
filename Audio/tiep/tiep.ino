/*
 * This is an example to read analog data at high frequency using the I2S peripheral
 * Run a wire between pins 27 & 32
 * The readings from the device will be 12bit (0-4096) 
 */
#include <driver/i2s.h>
#include <FS.h>
#include <SD.h>
#include "Wav.h"
#include "I2S.h"
#define I2S_SAMPLE_RATE 44100
#define ADC_INPUT ADC1_CHANNEL_4 //pin 32
#define OUTPUT_PIN 27
#define OUTPUT_VALUE 3800
#define READ_DELAY 1 //microseconds
const int record_time = 10;  // second
const char filename[] = "/uahdjh.wav";

const int headerSize = 44;
const int waveDataSize = record_time * 88000;
const int numCommunicationData = 8000;
const int numPartWavData = numCommunicationData/4;
byte header[headerSize];
char communicationData[numCommunicationData];
char partWavData[numPartWavData];
File file;
char buffer[2];
int num;
uint16_t adc_reading;

void i2sInit()
{
   i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate =  I2S_SAMPLE_RATE,              // The format of the signal using ADC_BUILT_IN
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 8,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
   };
   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
   i2s_set_adc_mode(ADC_UNIT_1, ADC_INPUT);
   i2s_adc_enable(I2S_NUM_0);
}

// void reader(void *pvParameters) {
//   uint32_t read_counter = 0;
//   uint64_t read_sum = 0;
// // The 4 high bits are the channel, and the data is inverted
//   uint16_t offset = (int)ADC_INPUT * 0x1000 + 0xFFF;
//   size_t bytes_read;
//   while(1){
//     uint16_t buffer[2] = {0};
//     i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, 15);
//     //Serial.printf("%d  %d\n", offset - buffer[0], offset - buffer[1]);
//     if (bytes_read == sizeof(buffer)) {
//       read_sum += offset - buffer[0];
//       read_sum += offset - buffer[1];
//       read_counter++;
//     } else {
//       Serial.println("buffer empty");
//     }
//     if (read_counter == I2S_SAMPLE_RATE) {
//       adc_reading = read_sum / I2S_SAMPLE_RATE / 2;
//       //Serial.printf("avg: %d millis: ", adc_reading);
//       //Serial.println(millis());
//       read_counter = 0;
//       read_sum = 0;
//       i2s_adc_disable(I2S_NUM_0);
//       i2s_adc_enable(I2S_NUM_0);
//     }
//   }
// }

void setup() {
  Serial.begin(115200);
    if (!SD.begin()) Serial.println("SD begin failed");
  else Serial.println("SD intalled success!!!");
  // Put a signal out on pin 
  uint32_t freq = ledcSetup(0, I2S_SAMPLE_RATE, 10);
  Serial.printf("Output frequency: %d\n", freq);
  ledcWrite(0, OUTPUT_VALUE/4);
  ledcAttachPin(OUTPUT_PIN, 0);
  // Initialize the I2S peripheral
  i2sInit();
  // Create a task that will read the data
  // xTaskCreatePinnedToCore(reader, "ADC_reader", 2048, NULL, 1, NULL, 1);
 CreateWavHeader(header, waveDataSize);
  SD.remove(filename);
  file = SD.open(filename, FILE_WRITE);
  if (!file) return;
  file.write(header, headerSize);
  for (int j = 0; j < waveDataSize/numPartWavData; ++j) {
    I2S_Read(communicationData, numCommunicationData);
    for (int i = 0; i < numCommunicationData/8; ++i) {
      partWavData[2*i] = communicationData[8*i + 2];
      partWavData[2*i + 1] = communicationData[8*i + 3];
    }
    file.write((const byte*)partWavData, numPartWavData);
  }
  file.close();
  Serial.println("finish");
}

void loop() {
  //Serial.printf("ADC reading: %d\n", adc_reading);
  if(Serial.available()){
    if(Serial.readString()=="start"){
      Serial.println("recorder start!!!");
      file=SD.open("/start1.wav",FILE_WRITE);
      file.write(header, headerSize);
          while(!Serial.available()){
        i2s_read_bytes(I2S_NUM_0, (char *)buffer, num, 50);
        Serial.println(buffer);
        file.write((byte*)buffer,num);
          // statement
      }
      Serial.println("recorder stop!!!");
      file.close();
    }
  }
}
