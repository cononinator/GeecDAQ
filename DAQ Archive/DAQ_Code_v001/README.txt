This was the version of the DAQ code used in Edgeworthstown testing on the 27/02/2024

Polls data from ADC at roughly 2 Hz, we think this is due to the Read/Write cycle of the SD card reader.

We want to implement a queue to save a big chunk of data to the SD card at once.