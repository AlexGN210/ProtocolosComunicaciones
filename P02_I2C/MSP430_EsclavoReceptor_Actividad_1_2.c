#include <msp430.h>

volatile unsigned char RXData;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Detener el watchdog timer

  P1DIR |= BIT0;                            // P1.0 como salida para el LED
  P1OUT &= ~BIT0;                           // Apagar el LED inicialmente

  P3SEL |= 0x03;                            // Asignar 3.0 y 3.1 a USCI_B0
  UCB0CTL1 |= UCSWRST;                      // Habilitar reinicio de software para la configuración inicial
  UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, modo sincrónico
  UCB0I2COA = 0x48;                         // Dirección del esclavo 0x48
  UCB0CTL1 &= ~UCSWRST;                     // Limpiar reinicio de software, iniciar operación
  UCB0IE |= UCRXIE;                         // Habilitar interrupción de recepción RX

  __bis_SR_register(LPM0_bits + GIE);       // Entrar en LPM0, habilitar interrupciones globalmente

  while (1)
  {
    __no_operation();                       // Ciclo infinito
  }
}

// ISR de interrupción para USCI_B0
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR(void)
#else
#error "Compiler not supported!"
#endif
{
  switch (__even_in_range(UCB0IV, 12))
  {
    case 0: break;                          // Vector 0: No interrupts
    case 2: break;                          // Vector 2: ALIFG
    case 4: break;                          // Vector 4: NACKIFG
    case 6: break;                          // Vector 6: STTIFG
    case 8: break;                          // Vector 8: STPIFG
    case 10:                                // Vector 10: RXIFG
      RXData = UCB0RXBUF;                   // Leer dato recibido del buffer RX

      if (RXData == 'A')
        P1OUT |= BIT0;                      // Encender LED si se recibe 'A'
      else if (RXData == 'B')
        P1OUT &= ~BIT0;                     // Apagar LED si se recibe 'B'

      break;
    case 12: break;                         // Vector 12: TXIFG
    default: break;
  }
}
