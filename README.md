# Timing_STM32MPU1

This is a simple application intended to measure the Round Trip Time of messages between the coprocessors on the STM32MP157C-DK2 module.

This application works on the Cortex A7 side and is intended to be supplemented by the OpenAMP_tty_echo_CM4 example project for the M4 that supplies the echo functionality on the RPMSG channels.
building this application requires the OpenSTLinux SDK.
