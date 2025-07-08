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
- [ ] For timestamps, need to fix what happens when we reach max time in microseconds (around 71 minutes). Currently it will wrap back to zero, which is bad. easy solution is use uint64_t instead of uint32_t
