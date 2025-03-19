#include <stdio.h> // Padrão
#include <string.h> // Padrão
#include <stdlib.h> // Padrão
#include "pico/stdlib.h" // Padrão
#include "lwip/tcp.h" // Comunicação Wi-fi
#include "pico/cyw43_arch.h" // Controle Wi-fi
#include "hardware/gpio.h" // Interrupção Botões
#include "pico/time.h" // Controle de GPIO (botões, LEDs)...
#include <ctype.h> // Display OLED
#include "inc/ssd1306.h" // Display OLED
#include "hardware/i2c.h" // Tela OlED - Comunicação I2C
#include "hardware/adc.h" // Leitura do Sensor de Temp.
#include "hardware/pwm.h" // Controle do Buzzer
#include "hardware/pio.h" // Controle da matriz de LEDs via PIO
#include "hardware/clocks.h" // Controle do Clock Buzzer
#include "pico/binary_info.h" // Tela OLED
#include "ws2818b.pio.h" // Biblioteca gerada pelo arquivo .pio durante compilação.

#define LED_PIN 12          // LED Azul
#define LED_CONECTADO 11    // LED Verde
#define LED_SEM_CONEXAO 13  // LED Vermelho
#define WIFI_SSID "ADRIANA ALVES"  // Nome da rede Wi-Fi
#define WIFI_PASS "luanalves@1" // Senha da rede Wi-Fi
#define SCREEN_WIDTH 128  // Largura do display
#define SCREEN_HEIGHT 64  // Altura do display
#define CHAR_WIDTH 6      // Largura média de um caractere
#define CHAR_HEIGHT 8     // Altura da fonte
#define BUZZER_PIN 21     // Configuração do pino do buzzer
#define BUZZER_FREQUENCY 2700 // Frequência do buzzer
#define MATRIZ_LINHAS 5   // Número de linhas da matriz de LEDs
#define MATRIZ_COLUNAS 5  // Número de colunas da matriz de LEDs
#define LED_COUNT (MATRIZ_LINHAS * MATRIZ_COLUNAS) // Total de LEDs na matriz
#define LED_PIN_MATRIZ 7  // Pino de controle da matriz de LEDs
#define BUTTON_A 5        // Pino do botão A
#define BUTTON_B_PIN 6    // Pino do botão B

// Estrutura HTML 
#define HTTP_RESPONSE_TEMPLATE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" \
"<!DOCTYPE html><html lang=\"pt-br\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Embarca Tech - Atividade</title><style>body{text-align: center;}.container{display: flex;justify-content: center;gap: 10px;flex-wrap: wrap;}a {display: inline-block;padding: 10px 20px;text-decoration: none;font-size: 16px;font-weight: bold;color: white;background-color: #007BFF;border-radius: 5px;transition: 0.3s;text-align: center;}a:hover { background-color: #0056b3;}p { margin: 10px 0; text-align: center; }.luz-casa, .arcondicionado-casa {display: flex;flex-wrap: wrap;justify-content: center;gap: 10px;}@media (max-width: 600px) {.container { flex-direction: column; align-items: center; }.luz-casa div, .arcondicionado-casa div { width: 100%; }footer { position: relative; }}</style></head><body><div class=\"container-temp\"><div class=\"borda\"><div class=\"hora-data\"><span>Temp. do Ambiente</span></div><div class=\"temp\"><span>%.0fºC</span></div></div></div><div class=\"container\"><p><fieldset><legend>Portão de Casa</legend><a href=\"/led/on\">Abrir Portão</a></p><p><a href=\"/led/off\">Fechar Portão</a></fieldset></p><fieldset><legend>Controle de Luzes</legend><div class=\"luz-casa\"><p><a href=\"/led/matriz/ligar/1/1\">Acender Luz da Sala</a></p><p><a href=\"/led/matriz/desligar/1/1\">Desligar Luz da Sala</a></p><p><a href=\"/led/matriz/ligar/1/2\">Acender Luz do Quarto</a></p><p><a href=\"/led/matriz/desligar/1/2\">Apagar Luz do Quarto</a></p><p><a href=\"/led/matriz/ligar/1/3\">Acender Luz da Cozinha</a></p><p><a href=\"/led/matriz/desligar/1/3\">Apagar Luz da Cozinha</a></p><p><a href=\"/led/matriz/ligar/1/4\">Acender Luz do Banheiro</a></p><p><a href=\"/led/matriz/desligar/1/4\">Apagar Luz do Banheiro</a></p></div></fieldset><fieldset><legend>Controle do Ar-Condicionado</legend><div class=\"arcondicionado-casa\"><p><a href=\"/led/matriz/ligar/1/5\">Ligar Ar-Condicionado</a></p><p><a href=\"/led/matriz/desligar/1/5\">Desligar Ar-Condicionado</a></p></div></fieldset></div></body></html>\r\n"

int IP[4] = {}; // Auxiliar de Recurso Alternativo...
int matriz[MATRIZ_LINHAS][MATRIZ_COLUNAS][3] = { // Matriz de LEDs com todos desligados (0, 0, 0)
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
};

int matriz_wifi[5][5][3] = { // Matriz de LEDs ativados para mostrar o wi-fi
    {{0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 255, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 0, 0}},
    {{0, 255, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}},      
    {{0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
};

// Comunicação SDA e SCL do Display OLED
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

struct render_area frame_area = {  //
    start_column : 0,
    end_column : ssd1306_width - 1,
    start_page : 0,
    end_page : ssd1306_n_pages - 1
};

uint8_t ssd[ssd1306_buffer_length]; // Zera o display inteiro

typedef struct { // Estrutura para um LED RGB...
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel...
} npLED_t;
  
npLED_t leds[LED_COUNT]; // Buffer de pixels que formam a matriz...

// Variáveis para uso da máquina PIO
PIO np_pio;
uint sm;

void npInit(uint pin) { // Inicializa a máquina PIO para controle da matriz de LEDs

    uint offset = pio_add_program(pio0, &ws2818b_program);     // Cria programa PIO
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, false); // Toma posse de uma máquina PIO
    if (sm < 0) {
      np_pio = pio1;
      sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }
  
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f); // Inicia programa na máquina PIO obtida
  
    for (uint i = 0; i < LED_COUNT; ++i) {     // Limpa buffer de pixels
      leds[i].R = 0;
      leds[i].G = 0;
      leds[i].B = 0;
    }
}

void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) { // Atribui uma cor RGB a um LED
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
  }
  
void npClear() { // Limpa os valores que estão nos LEDs("Desliga os LEDs"):
    for (uint i = 0; i < LED_COUNT; ++i)
      npSetLED(i, 0, 0, 0);
}

void npWrite() { // Atualiza a matriz de LEDs com os valores do buffer

    for (uint i = 0; i < LED_COUNT; ++i) { // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO
      pio_sm_put_blocking(np_pio, sm, leds[i].G);
      pio_sm_put_blocking(np_pio, sm, leds[i].R);
      pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100); // Espera 100us, sinal de RESET do datasheet
}
  
int getIndex(int x, int y) {
      if (y % 2 == 0) {
          return 24-(y * MATRIZ_LINHAS + x);
      } else { 
          return 24-(y * MATRIZ_LINHAS + (4 - x));
      }
}

void desenharMatriz(int matriz[MATRIZ_LINHAS][MATRIZ_COLUNAS][3]) { // Função que desenha na matriz de LEDs
    for (int linha = 0; linha < MATRIZ_LINHAS; linha++) {
        for (int coluna = 0; coluna < MATRIZ_COLUNAS; coluna++) {
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, matriz[linha][coluna][0], matriz[linha][coluna][1], matriz[linha][coluna][2]);
        }
    }
    npWrite();
}

void abrir_portao(){ // Simula o funcionamento de abertura do portão
    for (int i = 0; i < 3; i++){
            gpio_put(LED_PIN, 1);  // Liga o LED
            sleep_ms(250);  // Espera 250ms
            gpio_put(LED_PIN, 0);  // Desliga o LED
            sleep_ms(250);  // Espera 250ms
        }
        gpio_put(LED_PIN, 1);  // Liga o LED
}

void fechar_portao(){ // Simula o funcionamento de fechar o portão
    gpio_put(LED_PIN, 0);  // Desliga o LED
}

void init_sensor_temperatura(){ // Habilita o leitor de temperatura e seleciona seu pino
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
}

float temperatura_ambiente(bool formatado){ // Faz a Leitura da Temperatura
    uint16_t raw = adc_read();
    const float conversao = 3.3f / ((1<<12) - 1);
    float voltage = raw * conversao;
    float temperature = 27 - (voltage - 0.706)/0.001721;

    float truncated_temperature = ((temperature * 100) / 100.0f) - 4;
    if(formatado == true){ // Se a informação precisa ser formatada ele entra no if 
        float truncated_temperature = (int)((temperature * 100) / 100.0f) - 4;
        return truncated_temperature;
    } else{ // Se não precisa ser formatada ele mostra a informação completa da leitura feita
        return truncated_temperature;
    }
}

static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        tcp_close(tpcb); // Cliente fechou a conexão
        return ERR_OK;
    }

    char *request = (char *)p->payload; // Processa a requisição HTTP

    if (strstr(request, "GET /led/matriz/ligar/1/1")) { // As funções que podem ser realizadas no servidor
        matriz[0][0][0] = 255;  // Vermelho
        matriz[0][0][1] = 255;    // Verde
    } else if (strstr(request, "GET /led/matriz/desligar/1/1")) {
        matriz[0][0][0] = 0;    // Vermelho
        matriz[0][0][1] = 0;    // Verde
    } else if (strstr(request, "GET /led/matriz/ligar/1/2")) {
        matriz[0][1][0] = 255;  // Vermelho
        matriz[0][1][1] = 255;    // Verde
    } else if (strstr(request, "GET /led/matriz/desligar/1/2")) {
        matriz[0][1][0] = 0;    // Vermelho
        matriz[0][1][1] = 0;    // Verde
    } else if (strstr(request, "GET /led/matriz/ligar/1/3")) {
        matriz[0][2][0] = 255;  // Vermelho
        matriz[0][2][1] = 255;    // Verde
    } else if (strstr(request, "GET /led/matriz/desligar/1/3")) {
        matriz[0][2][0] = 0;    // Vermelho
        matriz[0][2][1] = 0;    // Verde
    } else if (strstr(request, "GET /led/matriz/ligar/1/4")) {
        matriz[0][3][0] = 255;  // Vermelho
        matriz[0][3][1] = 255;    // Verde
    } else if (strstr(request, "GET /led/matriz/desligar/1/4")) {
        matriz[0][3][0] = 0;    // Vermelho
        matriz[0][3][1] = 0;    // Verde
    } else if (strstr(request, "GET /led/matriz/ligar/1/5")) { // "Ar-condicionado"
        matriz[1][1][2] = 255;    // Azul
    } else if (strstr(request, "GET /led/matriz/desligar/1/5")) {
        matriz[1][1][2] = 0;    // Azul
    }
    
    desenharMatriz(matriz);

    if (strstr(request, "GET /led/on")) {     // Ação de Abrir e Fechar o Portão
        abrir_portao();
    } else if (strstr(request, "GET /led/off")) {
        fechar_portao();
    }

    char http_response[4096];
    snprintf(http_response, sizeof(http_response), HTTP_RESPONSE_TEMPLATE, temperatura_ambiente(true));

    int len = strlen(http_response);  // Envia a resposta HTTP em partes caso precise :)
    int sent = 0;
    while (sent < len) {
        int to_send = len - sent > 4096 ? 4096 : len - sent; // Envia em partes de 1024 bytes
        err_t write_err = tcp_write(tpcb, http_response + sent, to_send, TCP_WRITE_FLAG_COPY);
        if (write_err != ERR_OK) {
            printf("Erro ao enviar resposta HTTP\n");
            break;
        }
        sent += to_send;
    }
    tcp_output(tpcb); // Força o envio dos dados
    sleep_ms(500);
    pbuf_free(p); // Libera o buffer recebido
    return ERR_OK;
}

static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);  // Associa o callback HTTP
    return ERR_OK;
}

static void start_http_server(void) {  // Inicia o Servidor HTTP
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        return;
    }

    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) { // Liga o servidor na porta 80
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }

    pcb = tcp_listen(pcb);  // Coloca o PCB em modo de escuta
    tcp_accept(pcb, connection_callback);  // Associa o callback de conexão
    printf("Servidor HTTP rodando na porta 80...\n");
}

void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);  // Configurar o pino como saída de PWM

    uint slice_num = pwm_gpio_to_slice_num(pin);  // Obter o slice do PWM associado ao pino

    pwm_config config = pwm_get_default_config();  // Configurar o PWM com frequência desejada
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0); // Iniciar o PWM no nível baixo
}

void beep(uint pin, uint duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(pin); // Obter o slice do PWM associado ao pino
    pwm_set_gpio_level(pin, 3072);   // Configurar o duty cycle para 50%
    sleep_ms(duration_ms);  // Temporização
    pwm_set_gpio_level(pin, 0); // Desativar o sinal PWM
    sleep_ms(100); // Pausa de 100ms
}

void button_callback(uint gpio, uint32_t events) { // Função para tratar a interrupção do botão
    static uint32_t last_time_button_A = 0;
    static uint32_t last_time_button_B = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A) {
        if (current_time - last_time_button_A < 200) { // 200ms de debounce
            return; // Ignora interrupções muito próximas no tempo
        }
        last_time_button_A = current_time;

        printf("Botão \"A\" foi pressionado\n");
        gpio_put(LED_PIN, 1);  // Liga o LED
    }

    else if (gpio == BUTTON_B_PIN) {
        if (current_time - last_time_button_B < 200) { // 200ms de debounce
            return; // Ignora interrupções muito próximas no tempo
        }
        last_time_button_B = current_time;

        printf("Botão \"B\" foi pressionado\n");
        gpio_put(LED_PIN, 0);  // Desliga o LED
    }
}

void init_botoes_com_interrupcao() { // Interrupção com botões
    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, button_callback);     // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, button_callback);
}

void init_comunicacao_I2C(){ // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000); 
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

void init_controle_leds(){ // Configura o LED que pode ser controlado como saída.
    gpio_init(LED_SEM_CONEXAO); 
    gpio_set_dir(LED_SEM_CONEXAO, GPIO_OUT);
    gpio_init(LED_CONECTADO);
    gpio_set_dir(LED_CONECTADO, GPIO_OUT);

    gpio_init(LED_PIN); // Configura o LED que pode ser controlado como saída.
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

void limpar_display(uint8_t *ssd, struct render_area *frame_area) {
    calculate_render_area_buffer_length(frame_area);
    memset(ssd, 0, ssd1306_buffer_length); // Zera o buffer do display
    render_on_display(ssd, frame_area);    // Renderiza o display limpo
}

void mensagem_init(char *text[], int num_linhas) {
    int text_height = num_linhas * CHAR_HEIGHT; // Altura total do bloco de texto
    int start_y = (ssd1306_height - text_height) / 2; // Centralização vertical

    for (uint i = 0; i < num_linhas; i++) {
        int text_width = strlen(text[i]) * CHAR_WIDTH;
        int x = (ssd1306_width - text_width) / 2; // Centralização horizontal
        int y = start_y + (i * CHAR_HEIGHT); // Ajuste vertical

        ssd1306_draw_string(ssd, x, y, text[i]);
    }

    render_on_display(ssd, &frame_area);
}

void OLED_conexao_wifi(){ // Messagem de inicialização de conexão
    printf("Se conctando ao wi-fi \n");
    gpio_put(LED_SEM_CONEXAO, true); // Liga o LED

    char *text[] = {
        "Conectando   ",
        "   . ao        ",
        "  Wi-Fi!   ",
    };

    mensagem_init(text, (sizeof(text) / sizeof(text[0]))); // Passa o tamanho do array
}

void formatar_ip(int ip[4], char *ip_str) {
    snprintf(ip_str, 16, "IP %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}

void mensagem_init_com_ip(char *text[], int num_linhas, int ip[4]) {
    char ip_str[16]; // Buffer para armazenar o IP formatado
    formatar_ip(ip, ip_str); // Formata o IP

    char *text_com_ip[num_linhas + 1]; // Adiciona o IP como uma nova linha no array de texto
    for (int i = 0; i < num_linhas; i++) {
        text_com_ip[i] = text[i];
    }
    text_com_ip[num_linhas] = ip_str; // Adiciona o IP como última linha

    int text_height = (num_linhas + 1) * CHAR_HEIGHT; // Altura total do bloco de texto (incluindo o IP)
    int start_y = (ssd1306_height - text_height) / 2; // Centralização vertical

    for (uint i = 0; i < num_linhas + 1; i++) {
        int text_width = strlen(text_com_ip[i]) * CHAR_WIDTH; // Largura do texto
        int x = ((ssd1306_width - text_width) / 2) - 20; // Centralização horizontal
        int y = start_y + (i * CHAR_HEIGHT); // Ajuste vertical

        ssd1306_draw_string(ssd, x, y, text_com_ip[i]); // Desenha o texto no display
    }

    render_on_display(ssd, &frame_area); // Renderiza o display
}

void OLED_wifi_conectado() {
    printf("Conectado.\n");
    gpio_put(LED_SEM_CONEXAO, false);  // Desliga o LED

    char *text[] = {
        "   Conectado   ",
        "     . ao        ",
        "    Wi-Fi!   ",
    };

    uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr); // Formatando o Endereço IP
    printf("Endereço IP %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    IP[0] = ip_address[0]; // Recurso Alternativo :)
    IP[1] = ip_address[1];
    IP[2] = ip_address[2];
    IP[3] = ip_address[3];

    mensagem_init_com_ip(text, (sizeof(text) / sizeof(text[0])), IP);     // Mostra a mensagem de conexão e o IP no Display

    beep(BUZZER_PIN, 500); // Bipe de 500ms

    for (size_t i = 0; i < 4; i++) {
        gpio_put(LED_CONECTADO, true);
        desenharMatriz(matriz_wifi);
        sleep_ms(250);  // Espera 250ms
        gpio_put(LED_CONECTADO, false);
        desenharMatriz(matriz);
        sleep_ms(250);  // Espera 250ms
    }

    printf("Wi-Fi conectado!\n Acesse o Endereço IP %d.%d.%d.%d/led/on ", IP[0], IP[1], IP[2], IP[3]);
}

void falha_conexao_wifi(){   // Mensagem de Falha na Conexão
    printf("Falha ao conectar ao Wi-Fi\n");
    
    char *text[] = {
        " . Falha       ",
        "  . ao        ",
        " Conectar     ",
    };
    mensagem_init(text, (sizeof(text) / sizeof(text[0]))); // Mostra no Display
}

void reiniciar_conexao_wifi() { // Faz uma nova tentativa de conexão caso não consiga se conectar inicialmente
    printf("Reiniciando tentativa de conexão Wi-Fi...\n");

    sleep_ms(5000); // Aguarda 5 segundos antes de tentar reconectar
    OLED_conexao_wifi();
    sleep_ms(500);
    
    printf("Tentando reconectar ao Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) { // Tenta reconectar ao Wi-Fi
        falha_conexao_wifi();
        reiniciar_conexao_wifi(); // Tenta reconectar de novo
    } else {
        OLED_wifi_conectado(); // Exibe mensagem de sucesso no OLED
    }
}

int main(){
    stdio_init_all();   // Inicializa os tipos stdio padrão presentes ligados ao binário

    init_sensor_temperatura();
    init_comunicacao_I2C();
    init_controle_leds();
    pwm_init_buzzer(BUZZER_PIN);
    beep(BUZZER_PIN, 500); // Bipe de 500ms
    npInit(LED_PIN_MATRIZ);
    npClear();
    ssd1306_init(); // Inicialização completa da Tela OLED SSD1306
    init_botoes_com_interrupcao(); // Definindo os botões como uma interrupção

    limpar_display(ssd, &frame_area);
    OLED_conexao_wifi();
    sleep_ms(5000);

    printf("Iniciando servidor HTTP\n");

    if (cyw43_arch_init()) { // Inicializa o Wi-Fi
        printf("Erro ao inicializar o Wi-Fi\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");
    sleep_ms(10000);

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        falha_conexao_wifi(); // Mensagem de Falha na Conexão...
        reiniciar_conexao_wifi(); // Reinicia a tentativa de conexão...
    } else {
        OLED_wifi_conectado(); 
    }

    start_http_server(); // Inicia o servidor HTTP

    while(true) {
        cyw43_arch_poll();  // Mantém o Wi-Fi ativo
        printf("Temperatura: %fC \n", temperatura_ambiente(false));
        sleep_ms(1000);
    }
    cyw43_arch_deinit();  // Desliga o Wi-Fi 
    limpar_display(ssd, &frame_area);
    return 0;
}
