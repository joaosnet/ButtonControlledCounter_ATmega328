/*
 * contador00a29.c
 *
 * Created: 24/11/2023 15:27:09
 * Author : Joao Neto
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define cpl_bit(Y,bit_x) (Y^=(1<<bit_x)) //troca o estado do bit x
#define tst_bit(y,bit) (y&(1<<bit))//retorna 0 ou 1 conforme leitura do bit
// Defina os pinos para os botões A e B
#define BOTAO_A_PINO   PCINT20
#define BOTAO_B_PINO   PCINT21
// Defina os pinos para os registradores de deslocamento
#define DS_PINO    PB0   // Data
#define SH_CP_PINO PB1   // Clock
#define ST_CP_PINO PB2   // Latch
ISR(PCINT2_vect);//declara uso de PCINT1 (pinos PCINT8:10)

// Variáveis globais para a contagem e estado dos botões
volatile uint8_t contador = 0;
volatile uint8_t estadoBotaoA = 0;
volatile uint8_t estadoBotaoB = 0;

// Função para inicializar os registradores de deslocamento
void inicializarShiftRegisters() {
    // Configurar os pinos como saída
    DDRB |= (1 << DS_PINO) | (1 << SH_CP_PINO) | (1 << ST_CP_PINO);
}

// Função para enviar um byte para os registradores de deslocamento
void enviarByte(uint8_t dado) {
    for (int i = 7; i >= 0; i--) {
        // Configurar o pino de dados (DS)
        if (dado & (1 << i)) {
            PORTB |= (1 << DS_PINO);
        } else {
            PORTB &= ~(1 << DS_PINO);
        }

        // Pulsar o pino de clock (SH_CP)
        PORTB |= (1 << SH_CP_PINO);
        PORTB &= ~(1 << SH_CP_PINO);
    }

    // Pulsar o pino de latch (ST_CP)
    PORTB |= (1 << ST_CP_PINO);
    PORTB &= ~(1 << ST_CP_PINO);
}

// Função para atualizar os displays de sete segmentos
void atualizarDisplay() {
    // Mapear os dígitos e enviar para os registradores de deslocamento
    uint8_t digitos[2] = {contador / 10, contador % 10};
    
    for (int i = 0; i < 2; i++) {
        // Enviar o código do dígito para os displays de sete segmentos
        enviarByte(digitos[i]);
    }
}

// Configurar e inicializar as interrupções
void configurarInterrupcoes() {
    // Configurar os pinos dos botões como entrada
    DDRD &= ~(1 << BOTAO_A_PINO) | (1 << BOTAO_B_PINO);
    // Ativar resistores de pull-up para os botões
    PORTD |= (1 << BOTAO_A_PINO) | (1 << BOTAO_B_PINO);

    // Configurar a interrupção para o botão A (PCINT20)
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT20);

    // Configurar a interrupção para o botão B (PCINT21)
    PCMSK2 |= (1 << PCINT21);

    // Ativar interrupções globais
    sei();
}

int main(void) {
    inicializarShiftRegisters();
    configurarInterrupcoes();

    while (1) {
        // Atualizar os displays de sete segmentos
        atualizarDisplay();
    }

    return 0;
}

// Rotina de interrupção para os botões A e B (PCINT20 e PCINT21)
ISR(PCINT2_vect) {
    _delay_ms(10); // Debounce
    if (!(PIND & (1 << BOTAO_A_PINO))) {
        // Incrementar o contador
        contador = (contador + 1) % 30;
    }
    if (!(PIND & (1 << BOTAO_B_PINO))) {
        // Decrementar o contador
        contador = (contador == 0) ? 29 : contador - 1;
    }
}