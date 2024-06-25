#include <msp430.h>

volatile unsigned char TXData = 'A';
volatile unsigned char TXByteCtr;
volatile unsigned char currentAddress = 0x48;

void setup_timer();

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Detenemos el watchdog timer

  P3SEL |= 0x03;                            // Asignamos 3.0 y 3.1 a USCI_B0
  UCB0CTL1 |= UCSWRST;                      // Habilitamos el reset de software
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C maestro en modo sincrónico
  UCB0CTL1 = UCSSEL_1 + UCSWRST;            // ACLK
  UCB0BR0 = 33;                             // fSCL = ACLK/33
  UCB0BR1 = 0;
  UCB0I2CSA = currentAddress;               // Dirección del esclavo inicial
  UCB0CTL1 &= ~UCSWRST;                     // Limpiamos reset y continuamos operando
  UCB0IE |= UCTXIE;                         // Habilitamos la interrupción de transmisión

  setup_timer();                            // Configuramos temporizador

  while (1)
  {
    __bis_SR_register(LPM0_bits + GIE);     // Entramos en LPM0 con interrupciones habilitadas
    __no_operation();                       // No operation, solo para depuración

    // Cambiamos la dirección del esclavo para la próxima transmisión
    if (currentAddress == 0x48) {
      currentAddress = 0x50;
    } else if (currentAddress == 0x50) {
      currentAddress = 0x52;
    } else if (currentAddress == 0x52) {
      currentAddress = 0x48;
    }

    UCB0I2CSA = currentAddress;             // Actualizamos la dirección del esclavo

    TXData = 'A';                          // Enviamos 'A' a la nueva dirección

    TXByteCtr = 1;                          // Cargamos dato

    while (UCB0CTL1 & UCTXSTP);             // Aseguramos condición de parada
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, Condición de inicio

    __bis_SR_register(LPM0_bits + GIE);     // Entramos en LPM0 con interrupciones habilitadas
    __no_operation();                       // No operation, solo para depuración

    // Esperamos 2 segundos antes de enviar 'B'
    __delay_cycles(2000000);

    TXData = 'B';                          // Enviamos 'B' a todas las direcciones

    TXByteCtr = 1;                          // Cargamos dato

    while (UCB0CTL1 & UCTXSTP);             // Aseguramos condición de parada
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, Condición de inicio

    __bis_SR_register(LPM0_bits + GIE);     // Entramos en LPM0 con interrupciones habilitadas
    __no_operation();                       // No operation, solo para depuración
  }
}

// Configurar el temporizador para un retraso de 2 segundos
void setup_timer()
{
  TA0CCTL0 = CCIE;                          // Habilitar interrupción del temporizador A
  TA0CCR0 = 32768 * 2 - 1;                  // Establecer contador para 2 segundos (para enviar 'A')
  TA0CTL = TASSEL_1 + MC_1 + TACLR;         // ACLK, modo up, limpiar temporizador
}

// ISR del temporizador A0
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
  __bic_SR_register_on_exit(LPM0_bits);     // Salimos de LPM0
}

// USCIAB0_ISR trasnmite cualquier dato cargado en TXByteCtr (Hoja de datos)
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR (void)
#else
#error "Unsupported compiler"
#endif
{
  switch(__even_in_range(UCB0IV,12))
  {
    case  0: break;                         // Vector  0: No hay interrupciones
    case  2: break;                         // Vector  2: ALIFG
    case  4: break;                         // Vector  4: NACKIFG
    case  6: break;                         // Vector  6: STTIFG
    case  8: break;                         // Vector  8: STPIFG
    case 10: break;                         // Vector 10: RXIFG
    case 12:                                // Vector 12: TXIFG
      if (TXByteCtr)                        // Verificar contador de byte TX
      {
        UCB0TXBUF = TXData;                 // Cargamos buffer de TX
        TXByteCtr--;                        // Decrementamos contador
      }
      else
      {
        UCB0CTL1 |= UCTXSTP;                // I2C en condición de paro
        UCB0IFG &= ~UCTXIFG;                // Limpiamos bandera de TX
        __bic_SR_register_on_exit(LPM0_bits); // Salimos de LPM0
      }
      break;
    default: break;
  }
}
