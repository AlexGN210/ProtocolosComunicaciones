from machine import UART, Pin

# Configuración del UART
uart = UART(0, baudrate=4800)

# Configura los pines para UART0 en parity. eveen es 0. Odd es 1. y none es None.
uart.init(9600, bits=8, parity=1, stop=2, tx=Pin(0), rx=Pin(1))

while True:
   
        uart.write('A')
        
        
        
    