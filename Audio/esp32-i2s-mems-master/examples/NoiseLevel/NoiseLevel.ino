/** 
 * ESP32 I2S Noise Level Example.
 * 
 * This example calculates a mean noise level.
 * This example is Public Domain.
 * 
 * @author maspetsberger 
 */

#include <driver/i2s.h>

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int BLOCK_SIZE = 1024;

void setup() {
  pinMode(32, INPUT);
  Serial.begin(500000);
  Serial.println("Configuring I2S...");
  esp_err_t err;

  // The I2S config as per the example
  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
      .sample_rate = 16000,                         // 16KHz
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // could only get it to work with 32bits
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // although the SEL config should be left, it seems to transmit on right
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
      .dma_buf_count = 8,                           // number of buffers
      .dma_buf_len = BLOCK_SIZE                     // samples per buffer
  };

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
      .bck_io_num = 15,   // BCKL
      .ws_io_num = 14,    // LRCL
      .data_out_num = -1, // not used (only for speakers)
      .data_in_num = 32   // DOUT
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("I2S driver installed.");
}

void loop() {

  // Read multiple samples at once and calculate the sound pressure
  int32_t samples[BLOCK_SIZE];
  int num_bytes_read = i2s_read_bytes(I2S_PORT, 
                                      (char *)samples, 
                                      BLOCK_SIZE,     // the doc says bytes, but its elements.
                                      portMAX_DELAY); // no timeout
  
  int samples_read = num_bytes_read / 8;
  if (samples_read > 0) {

    float mean = 0;
    for (int i = 0; i < samples_read; ++i) {
      mean += samples[i];
    }
    Serial.println(mean);
  }
}

// actually we would need to call `i2s_driver_uninstall(I2S_PORT)` upon exit. 