# LoRa transceiver
> ESP32C3 + SX1262 (HT-CT62)


# SFTU
> ESPS3

> SX1276 (RFM95W)

TODO:
- [ ] Produce a MAC & Windows compatible, simple GUI for:
  - [ ] console communication (text)
  - [ ] visualisation of states (mosfets, armed, battery voltage/level, RSSI, SD, sample rate, inputs, gain, etc)
  - [ ] Control over output states visually
  - [ ] RF settings controlled visually
- [ ] Ensure RF info is always sent, even if busy. I think ATM, it is capable of missing sending messages if it is receiving
- [ ] For timestamps, need to fix what happens when we reach max time in microseconds (around 71 minutes). Currently, it will wrap back to zero, which is bad. easy solution is use uint64_t instead of uint32_t
- [ ] In the data, there is an inconsistency in the sampling period every 0.5 to 1 second. I suspect this may be related to block write to the SD. or it could be priority/vTaskDelay
- [ ] There is frequency content at around 44.5Hz, what is causing this? Interestingly, the sporadic periods mentioned above are 0.024s, 0.09s, 0.025s - this could be related
