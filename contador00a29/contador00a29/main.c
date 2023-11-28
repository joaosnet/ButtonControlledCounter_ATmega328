/**
 * @file contador00a29.c
 * @brief Contador com controle de botões utilizando registradores de deslocamento.
 *
 * Este programa implementa um contador com controle de botões utilizando registradores de deslocamento.
 * Os botões A e B são utilizados para incrementar e decrementar o contador, respectivamente.
 * O valor do contador é exibido em dois displays de sete segmentos.
 * O programa utiliza interrupções para detectar as ações dos botões e atualizar o valor do contador.
 *
 * @date 24/11/2023
 * @version 1.0
 *
 * @author Joao Neto
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
ISR(PCINT2_vect);//declara uso de PCINT2 (pinos PCINT20:21)

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

// Mapeamento de dígitos para códigos de sete segmentos
uint8_t mapaSeteSegmentos[10] = {
    0b11000000, // 0
    0b11111001, // 1
    0b10100100, // 2
    0b10110000, // 3
    0b10011001, // 4
    0b10010010, // 5
    0b10000010, // 6
    0b11111000, // 7
    0b10000000, // 8
    0b10010000  // 9
};

// Função para atualizar os displays de sete segmentos
void atualizarDisplay() {
    // Mapear os dígitos e enviar para os registradores de deslocamento
    uint8_t digitos[2] = {mapaSeteSegmentos[contador / 10], mapaSeteSegmentos[contador % 10]};
    
    for (int i = 0; i < 2; i++) {
        // Enviar o código do dígito para os displays de sete segmentos
        enviarByte(digitos[i]);
    }
}

// Configurar e inicializar as interrupções
void configurarInterrupcoes() {
    // Configurar os pinos dos botões como entrada
    DDRD &= ~((1 << BOTAO_A_PINO) | (1 << BOTAO_B_PINO));
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
        _delay_ms(100);  // Adicionando um atraso de 100 ms
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