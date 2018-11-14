# Temperature
Project on STM32F103C8 ("Blue Pill").

It's an exercise in programming STM32 basing as much as possible on uC's datasheet and Reference Manual and using as few libraries as possible (it started with StdPeriph Library, but now it doesn't use it at all).

Reading temperature from DHT11 digital temperature sensor, LM35 analog temperature sensor and STM32's internal temperature sensor.

The data is displayed on TM1638 based board and sent over USART.

Connections:

uC    ---   LM35
PA0   ---   Vout

uC    ---   DHT11
PA6   ---   DIO

uC    ---   TM1638
PB12  ---   STB
PB13  ---   CLK
PB15  ---   DIO

uC    ---   UART<>USB
PA10  ---   TX
PA9   ---   RX
