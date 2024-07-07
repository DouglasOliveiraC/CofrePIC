/* 03/07/2024
 * Nome : Douglas Oliveira de Carvalho
 * Nusp :14637740
 * 
 * Relatório breve do Projeto Substituição PI7
 *
 * Devido a limitações do simulador, o uso da EEPROM não foi possível. 
 * Em vez disso, utilizamos um `unsigned char` global com quatro casas para armazenar a senha.
 *
 * Problemas Encontrados:
 * 1. Mensagens Residuais:
 *    - Após definir e aceitar uma senha, ou ao abrir o cofre com a senha correta, mensagens residuais no display causam anomalias.
 *    - Suspeitamos que esse problema esteja relacionado ao simulador, que causa atrasos na atualização do display.
 *
 * Implementações Adicionais:
 * 
 * 1. Timeout:
 *    - Implementado um timeout para reiniciar o programa e apagar a tela se o usuário demorar muito para inserir ou digitar a senha.
 *
 * Comentários Finais:
 * - Apesar dos problemas de display, a funcionalidade principal do cofre está operando corretamente.
 * - A próxima tela aparece corretamente após o atraso causado, possivelmente, pelo simulador.
 */



/*
 * Dimensionamento do Timer0 para Interrupção a Cada 10ms
 *
 * Parâmetros:
 * FOSC = 20MHz
 * TOSC = 50ns
 *
 * Cálculos:
 * Adotando n = 4 · TOSC (clock interno):
 * T1 = TOSC · 4
 * T1 = 0.2?s
 *
 * Escolhendo o Prescaler = 1:256, n = 256:
 * T2 = n · T1
 * T2 = 256 · 0.2?s
 * T2 = 51.2?s
 *
 * Verificação de Overflow:
 * Tdesejado = 10ms
 * m = round(Tdesejado / T2)
 * m = round(10ms / 51.2?s)
 * m = round(10ms / 51.2 * 10^-3ms)
 * m = 195 < 255
 *
 * Configuração do Timer0:
 * - Timer0 configurado para overflow a cada 10ms.
 * - Utilizando prescaler de 1:256.
 *
 * Configuração do Registrador:
 * - Prescaler: 1:256
 * - Valor de Inicialização do Timer0: 255 - 195 = 60

 */


#include <xc.h>
#include <stdio.h>
// Configurações do PIC16F886
#pragma config FOSC = EC
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config MCLRE = ON
#pragma config CP = OFF
#pragma config CPD = OFF
#pragma config BOREN = ON
#pragma config IESO = ON
#pragma config FCMEN = ON
#pragma config LVP = OFF

// CONFIG2
#pragma config BOR4V = BOR40V
#pragma config WRT = OFF

#define _XTAL_FREQ 20000000  // Define a frequência do oscilador para 20MHz
#define DEBOUNCE_TIME 20 // Tempo de debounce em microssegundos
 

// Variáveis globais
unsigned char password[4] = {0xFF, 0xFF, 0xFF, 0xFF};  // Senha utilizada durante o programa
unsigned char user_input[4];               // Entrada do usuário
volatile unsigned char key_ready = 0;      // Flag para indicar que uma tecla foi capturada
volatile unsigned char display_digits[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // Dígitos a serem mostrados no display
volatile unsigned int hold_counter = 0 ; // Temporizador
volatile unsigned char hold_flag2 = 0; // Flag global para indicar o estado do programa
volatile unsigned char key_pressed = 0xFF;  // Última tecla capturada
volatile unsigned char current_key_pressed = 0xFF;  // Estado atual da tecla





// Função para inicializar o display de 7 segmentos
void display_init(void) {
    TRISA &= 0b11110000;  // RA0 a RA3 como saída
    TRISC = 0x00;  // RC0 a RC6 como saída
    PORTA &= 0b11110000;  // Inicializa RA0 a RA3 como 0
    PORTC = 0x00;  // Inicializa RC0 a RC6 como 0
}



// Função para mostrar um dígito no display de 7 segmentos
void display_digit(unsigned char digit, unsigned char position) {
    const unsigned char segment_map[19] = {
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
        0b00000111, // 7
        0b01111111, // 8
        0b01101111, // 9
        0b01111001, // E 10     clSE = 10, 5 ,12 ,13
        0b01010000, // r 11     Error =16, 11 , 11 ,10
        0b00111000, // L 12     OPen = 14,10,18,0
        0b01011000, // c 13 
        0b01010100, // n 14
        0b01000000,  // -    15
        0b01011100,  // o    16
        0b00000000, // vazio 17   
        0b01110011 // P      18
    };

    unsigned char segment_code;
    if (digit < 20) {
        segment_code = segment_map[digit]; // Obter o código do segmento para o dígito
    } else {
        segment_code = 0b01000000; // Caractere '-' para dígitos inválidos
    }
    PORTC = segment_code; // RC0 a RC6 como segmentos do display
    PORTA = (1 << position); // RA0 a RA3 como controle dos dígitos
}

// Função para atualizar o display de 7 segmentos
void update_display(void) {
    static unsigned char digit_position = 0;

    // Limpa o display antes de atualizar
    PORTC = 0x00;
    PORTA = 0x00;

    if (display_digits[digit_position] != 0xFF) {
        display_digit(display_digits[digit_position], digit_position);
    }
    digit_position = (digit_position + 1) % 4;
}

void clear_display(void) {
    // Apaga todos os segmentos e desativa todos os dígitos
    PORTC = 0x00; // Apaga todos os segmentos
    PORTA = 0x00; // Desativa todos os dígitos
}

// Função para mostrar uma mensagem no display
void display_message(const unsigned char* message, unsigned int duration_ms) {
    unsigned int elapsed_time = 0;
    while (elapsed_time < duration_ms) {
        for (unsigned char i = 0; i < 4; i++) {
            display_digits[i] = message[i];
        }
        __delay_ms(5); // Pequeno atraso para permitir a multiplexação do display
        elapsed_time += 5;
    }
   
}

//Mapeamento de teclas do teclado matricial corrigido
const unsigned char key_map[4][3] = {
    {3, 2, 1}, 
    {6, 5, 4}, 
    {9, 8, 7}, 
    {0x0F, 0, 0x0E}  // 0x0E para '*', 0x0F para '#'
};

void get_key_combined(void) {
    static unsigned char row = 0;
    static unsigned char last_key_pressed = 0xFF; // Guarda a última tecla pressionada
    static unsigned int debounce_counter = 0; // Contador para debounce
    static unsigned char debounce_state = 0; // Estado do debounce

    // Configura RB0-RB3 como saída e RB4-RB7 como entrada
    TRISB = 0xF0; 
    // Coloca a linha atual em 0
    PORTB = ~(1 << row); 
    __delay_us(5); // Pequeno atraso para estabilizar

    for (unsigned char col = 0; col < 4; col++) {
        if (!(PORTB & (1 << (col + 4)))) { // Verifica se a coluna atual está em 0
            unsigned char current_key = key_map[row][col]; // Captura a tecla atual

            if (current_key != last_key_pressed) {
                debounce_counter = 0; // Reinicia o contador de debounce
                debounce_state = 0; // Reinicia o estado de debounce
                last_key_pressed = current_key;
            } else {
                if (debounce_state == 0) { // Estado inicial, tecla foi detectada
                    key_pressed = current_key; // Salva a tecla pressionada
                    key_ready = 1; // Indica que uma tecla foi capturada
                    debounce_state = 1; // Avança para o próximo estado de debounce
                } else if (debounce_state == 1) { // Estado de debounce
                    debounce_counter++;
                    if (debounce_counter >= DEBOUNCE_TIME) { // Verifica se o tempo de debounce foi atingido
                        current_key_pressed = current_key; // Atualiza o estado atual da tecla
                        debounce_state = 2; // Avança para o próximo estado
                    }
                }
            }
            return;
        }
    }

    // Se nenhuma tecla foi pressionada na linha atual, avança para a próxima linha
    row = (row + 1) % 4; // Avança para a próxima linha
    if (row == 0) { // Se completou a varredura de todas as linhas
        last_key_pressed = 0xFF; // Reseta a última tecla pressionada ao reiniciar o ciclo de varredura
        debounce_counter = 0; // Reseta o contador de debounce
        debounce_state = 0; // Reinicia o estado de debounce
    }
}

// Função para capturar a senha do usuário
void capture_user_input(void) {
    for (unsigned char i = 0; i < 4; i++) {
        user_input[i] = 0xFF;
        display_digits[i] = 0xFF;
    }
    
    for (unsigned char i = 0; i < 4; i++) {
        while (!key_ready); // Espera até que uma tecla seja capturada
        key_ready = 0; // Reseta a flag de tecla capturada

        // Move os dígitos atuais para a direita
        for (unsigned char j = 3; j > 0; j--) {
            user_input[j] = user_input[j - 1];
            display_digits[j] = user_input[j - 1];
        }

        // Coloca a nova tecla na posição mais à esquerda
        user_input[0] = key_pressed;
        display_digits[0] = key_pressed;
    }
}


unsigned char capture_user_input_senha(void) {
    unsigned char senha_valida = 1; // Assume que a senha é válida inicialmente
    unsigned char senha[4] ;
    
    // Inicializa os arrays com um valor que não acende nenhum segmento
    for (unsigned char i = 0; i < 4; i++) {
        user_input[i] = 0xFF;
        display_digits[i] = 0xFF;
    }

    unsigned char index = 0;
    while (index < 4) {
        while (!key_ready); // Espera até que uma tecla seja capturada
        key_ready = 0; // Reseta a flag de tecla capturada

        // Verifica se a tecla capturada é '*' ou '#', marcando a senha como inválida
        if (key_pressed == 0x0E || key_pressed == 0x0F) {
            senha_valida = 0; // Senha inválida
        }

        // Move os dígitos atuais para a direita
        for (unsigned char j = 3; j > 0; j--) {
            user_input[j] = user_input[j - 1];
            display_digits[j] = display_digits[j - 1];
        }

        // Armazena o novo dígito na posição mais à esquerda
        user_input[0] = key_pressed;
        display_digits[0] = key_pressed;

        // Armazena o dígito real no array password
        senha[index] = key_pressed;

        index++;
    }
    key_ready = 0; // Reseta a flag de tecla capturada
    // Espera pela tecla '*'
    while (key_pressed != 0x0F) {
        while (!key_ready); // Espera até que uma tecla seja capturada
        key_ready = 0; // Reseta a flag de tecla capturada
    }
    
    clear_display(); // Limpa o display após a captura de senha
    
    // Atualiza a variável global password
    for (unsigned char i = 0; i < 4; i++) {
        password[i] = senha[i];
    }
    return senha_valida;
}


// Função para verificar a senha do usuário
unsigned char verifica_senha(void) {
    unsigned char senha[4];
    unsigned char senha_valida = 1;

    // Inicializa os arrays com um valor que não acende nenhum segmento
    for (unsigned char i = 0; i < 4; i++) {
        user_input[i] = 0xFF;
        display_digits[i] = 0xFF;
    }

    unsigned char index = 0;
    while (index < 4) {
        while (!key_ready); // Espera até que uma tecla seja capturada
        key_ready = 0; // Reseta a flag de tecla capturada

        // Move os dígitos atuais para a direita
        for (unsigned char j = 3; j > 0; j--) {
            user_input[j] = user_input[j - 1];
            display_digits[j] = display_digits[j - 1];
        }

        // Armazena o novo dígito na posição mais à esquerda
        user_input[0] = 0b01000000; // Exibe um traço '-' no display
        display_digits[0] = 0b01000000; // Exibe um traço '-' no display

        // Armazena o dígito real no array senha
        senha[index] = key_pressed;

        index++;
    }
    key_ready = 0; // Reseta a flag de tecla capturada

    // Espera pela tecla '*'
    while (key_pressed != 0x0F) {
        while (!key_ready); // Espera até que uma tecla seja capturada
        key_ready = 0; // Reseta a flag de tecla capturada
    }

    clear_display(); // Limpa o display após a captura de senha

    // Compara a senha inserida com a senha armazenada
    for (unsigned char i = 0; i < 4; i++) {
        if (senha[i] != password[i]) {
            senha_valida = 0; // Senha inválida
            break;
        }
    }

    return senha_valida;
}

// Função para inicializar o motor de passo
void stepper_init(void) {
    // Inicializa bits RC7, RB7, RA5 e RA4 para saída digital
    TRISC7 = 0;  // RC7 como saída
    TRISB7 = 0;  // RB7 como saída
    TRISA5 = 0;  // RA5 como saída
    TRISA4 = 0;  // RA4 como saída
    RC7 = 0;  // Zera saída do motor de passo
    RB7 = 0;  // Zera saída do motor de passo
    RA5 = 0;  // Zera saída do motor de passo
    RA4 = 0;  // Zera saída do motor de passo
}

// Função para controlar o motor de passo
void stepper_set(int steps, unsigned int step_time) {
    const unsigned char states[] = {0b0001, 0b0010, 0b0100, 0b1000};  // Sequência de ativação das fases
    static int i = 0;
    while (steps != 0) {
        // Atualiza os pinos conforme a sequência de ativação
        RA4 = (states[i] & 0b0001) ? 1 : 0;
        RA5 = (states[i] & 0b0010) ? 1 : 0;
        RB7 = (states[i] & 0b0100) ? 1 : 0;
        RC7 = (states[i] & 0b1000) ? 1 : 0;
        
        // Atualiza o índice da sequência
        i = (steps > 0) ? (i + 1) : (i - 1);
        i = (i > 3) ? 0 : (i < 0) ? 3 : i;
        steps += (steps > 0) ? -1 : 1;
        
        // Atraso entre os passos
        for (int j = 0; j < step_time; j++) {
            __delay_ms(10);
        }
    }
}

// Inicializa Timer 0 com interrupcao de 5ms
void t0_init(void) {
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PSA = 0;
    OPTION_REGbits.PS = 7;
    TMR0 = 0xff - 195;
    TMR0IE = 1;
    GIE = 1; // Habilita interrupções globais
    
    }

// Função de interrupção do Timer 0
void __interrupt() ISR() {
    if (TMR0IE && TMR0IF) { // Verifica se a interrupção foi causada pelo Timer 0
        static int tick1 = 0;     // Variavel para cumprir leitura do teclado a cada 10 ms
        
        
        //Incrementa contador
        hold_counter++;
         
        // Continuamente atualiza o display
        update_display();
        
        //Atualiza o teclado a cada 10 ms
        if(tick1 ==1){
        // Recebe uma letra do teclado
        get_key_combined();
        tick1 = 0;
        }
        tick1++;  
        
        TMR0IF = 0; // Limpa a flag de interrupção do Timer 0
        TMR0 = 0xff - 195; // Reinicia o Timer 0 para gerar uma nova interrupção em 5ms
    }    
}


void main(void) {
    
    // Inicializa o display de 7 segmentos
    display_init();

    // Inicializa o Timer 0
    t0_init();
    
    //Inicializa motor de passo
    stepper_init();
    
    
    const unsigned int hold_threshold = 100; // aproximadamente 2 seg
    unsigned char hold_flag1 = 0; // Flag para indicar início da contagem de tempo para tecla *
    unsigned char timeout =0;
    
    
    while (1) {
        clear_display(); // Limpa o display após apertar a tecla
        // Captura uma tecla para determinar o modo de operação
        while (key_ready) {
            timeout =0;
            
            // Se capturada a tecla '*'
            if (current_key_pressed == 0x0E) { // 0x0E representa a tecla '*'
                
                hold_counter = 0; // Zera o contador
                
                //Se a senha nao foi definida
                if( hold_flag2 != 3){
                    hold_flag1 = 1; // Levanta a flag para indicar que o contador deve ser incrementado
                }
                
                 // Loop para verificar se a tecla * continuamente pressionada
                while (hold_flag1 == 1) {
                    __delay_ms(50); // Pequeno atraso
                    
                    if (hold_counter >= hold_threshold) {
                        hold_flag1 = 0; // Para de contar após atingir o threshold
                        if(hold_flag2 != 3){//Se nao foi definida a senha
                            hold_flag2 = 1; // Levanta a flag para exibir 0000
                            
                        }
                    }
                    
                    if (current_key_pressed != 0x0E) {
                        hold_flag1 = 0; // Se a tecla foi solta, reseta a flag
                        break; // Sai do loop
                    }
                }
               
                //*********************************
                // Abertura do cofre - Caso 2
                
                hold_counter = 0; // Zera o contador(TIMEOUT 1 minuto= 6000*10ms)
                while (hold_flag2 == 3 && hold_counter < 6000) { // Caso a senha já tenha sido definida
                                     
                    clear_display(); // Limpa o display novamente
                    __delay_ms(50);
                    
                    key_ready =0;
                    __delay_ms(10); // Pequeno atraso
                    
                    while(!key_ready && hold_counter < 6000){ 
                        
                        unsigned char on[4] = {14, 0, 17 , 17}; // Indicação de inicio
                        display_message(on,500);
                        if (hold_counter >= 600){
                            clear_display(); // Limpa o display novamente
                            timeout = 1;
                            break;
                        }
                   
                    }
                    
                    //Se nao foi acionado o timeout continua o processo
                    if(!timeout){
                        
                        unsigned char senha_valida = verifica_senha();
                        clear_display(); // Limpa o display novamente

                        // Procedimento da verificação da senha
                        if (senha_valida) {
                            stepper_set(-4,500);
                            unsigned char Open[4] = {14, 10, 18, 0}; // Indicação de sucesso
                            display_message(Open, 2000); // Mostrar sucesso por 2 segundos
                            clear_display(); // Limpa o display novamente
                            __delay_ms(100);

                            key_ready = 0; // Reseta a flag de tecla capturada
                            key_pressed = 0xFF;
                            break;
                        } else {
                            unsigned char error[4] = {16, 11, 11, 10}; // Indicação de erro
                            display_message(error, 2000); // Mostrar erro por 2 segundos
                            clear_display(); // Limpa o display novamente
                            __delay_ms(100);

                            key_ready = 0; // Reseta a flag de tecla capturada
                            key_pressed = 0xFF;
                            hold_counter = 0;
                        }
                    }
                }
                
                clear_display(); // Limpa o display novamente
                __delay_ms(50);
                
                
                //******************************
                //Definicão de senha - Caso 1
                hold_counter = 0; // Zera o contador(TIMEOUT 1 minuto= 6000*10ms)
                while (hold_flag2 == 1 && hold_counter < 6000) {
                    
                    //Habilita entrada do teclado
                    key_ready = 0;
                    __delay_ms(10); // Pequeno atraso
                    
                    while(!key_ready && hold_counter < 6000){
                        unsigned char inicio[4] = {0, 0, 0, 0}; // Indicação de inicio
                        display_message(inicio,10); // Dura até nova tecla
                        if(hold_counter >= 6000){
                            timeout = 1;
                            break;
                        }
                    }
                    
                    clear_display(); // Limpa o display após apertar a tecla
           
                    if(!timeout){
                    // Captura a nova senha
                    unsigned char senha_valida = capture_user_input_senha();
                    clear_display(); // Limpa o display novamente
                    
                    
                    // Procedimento da inserçao da senha
                    if(senha_valida){   
                        stepper_set(4,500);
                        hold_flag2 = 3; // Reseta a flag2 para senha definida
                        unsigned char cLSE[4] = {10, 5 , 12, 13}; // Indicação de sucesso
                        display_message(cLSE, 200); // Mostrar sucesso por 2 segundos
                        clear_display(); // Limpa o display novamente
                        
                        __delay_ms(100);
                        
                        // Reseta flags e variáveis
                        key_ready = 0; // Reseta a flag de tecla capturada
                        key_pressed = 0xFF;               
                        break;
                    }else {
                        unsigned char erro[4] = {16, 11, 11, 10}; // Indicação de erro
                        display_message(erro, 2000); // Mostrar erro por 2 segundos
                        clear_display(); // Limpa o display novamente
                        
                        __delay_ms(100);
                        
                        // Reseta flags e variáveis
                        key_ready = 0; // Reseta a flag de tecla capturada
                        key_pressed = 0xFF;
                        hold_counter = 0;
                    }
                    }
                }
            }
        }  
    }
     // Reseta flags e variáveis após processar a tecla
            key_ready = 0; // Reseta a flag de tecla capturada
            key_pressed = 0xFF;    
            clear_display();
}
