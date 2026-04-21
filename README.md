[[ru]](./README-ru.md)
[[en]](./README.md)

# Ai-Talap PLC host software

The repository contains software for configuration and control of
Ai-Talap series Programmable Logic Controllers.  The Ai-Talap Modular
PLC line is produced by [IT-P Engineering](http://https://itpengineering.com).

## Requirements

The software has been developed in [Debian
GNU/Linux](http://debian.org), specifically Debian 11 (bullseye), and
is known to work in Debian 12 (bookworm) too.  It should be portable
to any platform where
[ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)
runs.

You need some Ai-Talap hardware to exercise these tools; as a minimum,
one of Ai-Talap base modules (__AT241BC__, __AT241CPU__, or
__AT340CPU__) in the same Ethernet LAN as your computer.

## Installation

1. Clone this repository along with dependencies:

   ```
   git clone https://github.com/ITPEngineering/Ai-Talap
   cd Ai-Talap
   git submodule update --init --recursive
   ```
2. Say `make -C matiec` to build `matiec` translators (only necessary
   to control PLC, not needed for __AT241BC__ RTU module)



