#include <stdio.h> // Padrão
#include <string.h> // Padrão
#include <stdlib.h> // Padrão
#include "pico/stdlib.h" //Padrão
#include "lwip/tcp.h" // Wi-fi
#include "pico/cyw43_arch.h" // Wi-fi
#include <ctype.h> // Tela OLED
#include "inc/ssd1306.h" // Tela OLED
#include "hardware/i2c.h" // Tela OlED
#include "hardware/adc.h" // Sensor de Temp.
#include "hardware/pwm.h" // Buzzer
#include "hardware/pio.h" 
#include "hardware/clocks.h" //Buzzer
#include "pico/binary_info.h" // Tela OLED

#include "ws2818b.pio.h" // Biblioteca gerada pelo arquivo .pio durante compilação.

#define LED_PIN 12          // LED Azul
#define LED_CONECTADO 11    // LED Verde
#define LED_SEM_CONEXAO 13  // LED Vermelho
#define WIFI_SSID "Oxentenet_SAF_2.4G"  // Nome da rede Wi-Fi
#define WIFI_PASS "28021980" // Senha da rede Wi-Fi
#define SCREEN_WIDTH 128  // Largura do display
#define SCREEN_HEIGHT 64  // Altura do display
#define CHAR_WIDTH 6      // Largura média de um caractere
#define CHAR_HEIGHT 8     // Altura da fonte (provavelmente 8px)
#define BUZZER_PIN 21     // Configuração do pino do buzzer
#define BUZZER_FREQUENCY 2700 // Configuração da frequência do buzzer (em Hz)
#define MATRIZ_LINHAS 5
#define MATRIZ_COLUNAS 5
#define LED_COUNT (MATRIZ_LINHAS * MATRIZ_COLUNAS)
#define LED_PIN_MATRIZ 7
#define BUTTON_A 5
#define BUTTON_B_PIN 6

int IP[4] = {}; // Auxiliar de Recurso Alternativo...
int LED[3] = {0, 0, 0};
int LED_2[3] = {0, 0, 0};

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

#define HTTP_RESPONSE_TEMPLATE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" \
"<!DOCTYPE html><html lang=\"pt-br\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Embarca Tech - Atividade</title><style>body{text-align: center;}.container{display: flex;justify-content: center;gap: 10px;flex-wrap: wrap;}a {display: inline-block;padding: 10px 20px;text-decoration: none;font-size: 16px;font-weight: bold;color: white;background-color: #007BFF;border-radius: 5px;transition: 0.3s;text-align: center;}a:hover { background-color: #0056b3;}p { margin: 10px 0; text-align: center; }.luz-casa, .arcondicionado-casa {display: flex;flex-wrap: wrap;justify-content: center;gap: 10px;}@media (max-width: 600px) {.container { flex-direction: column; align-items: center; }.luz-casa div, .arcondicionado-casa div { width: 100%; }footer { position: relative; }}</style></head><body><div class=\"container-temp\"><div class=\"borda\"><div class=\"hora-data\"><span>Temp. do Ambiente</span></div><div class=\"temp\"><span>%.0fºC</span></div></div></div><div class=\"container\"><p><fieldset><legend>Portão de Casa</legend><a href=\"/led/on\">Abrir Portão</a></p><p><a href=\"/led/off\">Fechar Portão</a></fieldset></p><fieldset><legend>Controle de Luzes</legend><div class=\"luz-casa\"><p><a href=\"/led/matriz/ligar/1/1\">Acender Luz da Sala</a></p><p><a href=\"/led/matriz/desligar/1/1\">Desligar Luz da Sala</a></p><p><a href=\"/led/matriz/ligar/1/2\">Acender Luz do Quarto</a></p><p><a href=\"/led/matriz/desligar/1/2\">Apagar Luz do Quarto</a></p><p><a href=\"/led/matriz/ligar/1/3\">Acender Luz da Cozinha</a></p><p><a href=\"/led/matriz/desligar/1/3\">Apagar Luz da Cozinha</a></p><p><a href=\"/led/matriz/ligar/1/4\">Acender Luz do Banheiro</a></p><p><a href=\"/led/matriz/desligar/1/4\">Apagar Luz do Banheiro</a></p></div></fieldset><fieldset><legend>Controle do Ar-Condicionado</legend><div class=\"arcondicionado-casa\"><p><a href=\"/led/matriz/ligar/1/5\">Ligar Ar-Condicionado</a></p><p><a href=\"/led/matriz/desligar/1/5\">Desligar Ar-Condicionado</a></p></div></fieldset></div></body></html>\r\n"

typedef struct {
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
  } npLED_t;
  
npLED_t leds[LED_COUNT]; // Declaração do buffer de pixels que formam a matriz.
  
PIO np_pio;   // Variáveis para uso da máquina PIO.
uint sm;

void npInit(uint pin) { /*Inicializa a máquina PIO para controle da matriz de LEDs.*/

    uint offset = pio_add_program(pio0, &ws2818b_program);     // Cria programa PIO.
    np_pio = pio0;
  
    sm = pio_claim_unused_sm(np_pio, false); // Toma posse de uma máquina PIO.
    if (sm < 0) {
      np_pio = pio1;
      sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }
  
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f); // Inicia programa na máquina PIO obtida.
  
    for (uint i = 0; i < LED_COUNT; ++i) {     // Limpa buffer de pixels.
      leds[i].R = 0;
      leds[i].G = 0;
      leds[i].B = 0;
    }
}

void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) { /* Atribui uma cor RGB a um LED.*/
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
  }
  
void npClear() { // Limpa os valores que estão nos LEDs("Desliga os LEDs"):
    for (uint i = 0; i < LED_COUNT; ++i)
      npSetLED(i, 0, 0, 0);
}
  
void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) { // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
      pio_sm_put_blocking(np_pio, sm, leds[i].G);
      pio_sm_put_blocking(np_pio, sm, leds[i].R);
      pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}
  
int getIndex(int x, int y) {
      if (y % 2 == 0) {
          return 24-(y * MATRIZ_LINHAS + x);
      } else { 
          return 24-(y * MATRIZ_LINHAS + (4 - x));
      }
}
  
void desenharMatriz(int matriz[MATRIZ_LINHAS][MATRIZ_COLUNAS][3]) {
    for (int linha = 0; linha < MATRIZ_LINHAS; linha++) {
        for (int coluna = 0; coluna < MATRIZ_COLUNAS; coluna++) {
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, matriz[linha][coluna][0], matriz[linha][coluna][1], matriz[linha][coluna][2]);
        }
    }
    npWrite();
}

float temperatura_ambiente(bool formatado){ // Leitura da temperatura
    uint16_t raw = adc_read();
    const float conversao = 3.3f / ((1<<12) - 1);
    float voltage = raw * conversao;
    float temperature = 27 - (voltage - 0.706)/0.001721;

    float truncated_temperature = ((temperature * 100) / 100.0f) - 4;
    if(formatado == true){
        float truncated_temperature = (int)((temperature * 100) / 100.0f) - 4;
        return truncated_temperature;
    } else{
        return truncated_temperature;
    }

}

int matriz[MATRIZ_LINHAS][MATRIZ_COLUNAS][3] = { // Matriz de LEDs, inicialmente todos desligados (0, 0, 0)
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
};

int matriz_wifi[5][5][3] = {
    {{0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 255, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 0, 0}},
    {{0, 255, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}},      
    {{0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
};

void abrir_portao(){
    for (int i = 0; i < 3; i++){
            gpio_put(LED_PIN, 1);  // Liga o LED
            sleep_ms(250);  // Espera 350ms
            gpio_put(LED_PIN, 0);  // Desliga o LED
            sleep_ms(250);  // Espera 350ms
        }
        gpio_put(LED_PIN, 1);  // Liga o LED
}

void fechar_portao(){
    gpio_put(LED_PIN, 0);  // Desliga o LED
}

static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        tcp_close(tpcb); // Cliente fechou a conexão
        return ERR_OK;
    }

    char *request = (char *)p->payload; // Processa a requisição HTTP

    if (strstr(request, "GET /led/matriz/ligar/1/1")) {
        matriz[0][0][0] = 255;  // Vermelho
        matriz[0][0][1] = 255;    // Verde
    }
    
    if (strstr(request, "GET /led/matriz/desligar/1/1")) {
        matriz[0][0][0] = 0;    // Vermelho
        matriz[0][0][1] = 0;    // Verde
    }
    
    if (strstr(request, "GET /led/matriz/ligar/1/2")) {
        matriz[0][1][0] = 255;  // Vermelho
        matriz[0][1][1] = 255;    // Verde
    }
    
    if (strstr(request, "GET /led/matriz/desligar/1/2")) {
        matriz[0][1][0] = 0;    // Vermelho
        matriz[0][1][1] = 0;    // Verde
    }
    if (strstr(request, "GET /led/matriz/ligar/1/3")) {
        matriz[0][2][0] = 255;  // Vermelho
        matriz[0][2][1] = 255;    // Verde
    }
    
    if (strstr(request, "GET /led/matriz/desligar/1/3")) {
        matriz[0][2][0] = 0;    // Vermelho
        matriz[0][2][1] = 0;    // Verde
    }
    if (strstr(request, "GET /led/matriz/ligar/1/4")) {
        matriz[0][3][0] = 255;  // Vermelho
        matriz[0][3][1] = 255;    // Verde
    }
    
    if (strstr(request, "GET /led/matriz/desligar/1/4")) {
        matriz[0][3][0] = 0;    // Vermelho
        matriz[0][3][1] = 0;    // Verde
    }
    
    if (strstr(request, "GET /led/matriz/ligar/1/5")) { // Arcondicionado
        matriz[1][1][2] = 255;    // Azul
    }
    
    if (strstr(request, "GET /led/matriz/desligar/1/5")) {
        matriz[1][1][2] = 0;    // Azul
    }
    
    desenharMatriz(matriz);

    if (strstr(request, "GET /led/on")) {
        abrir_portao();
    } else if (strstr(request, "GET /led/off")) {
        fechar_portao();
    }

    char http_response[2048];
    snprintf(http_response, sizeof(http_response), HTTP_RESPONSE_TEMPLATE, temperatura_ambiente(true));

    int len = strlen(http_response);  // Envia a resposta HTTP em partes caso precise :)
    int sent = 0;
    while (sent < len) {
        int to_send = len - sent > 2048 ? 2048 : len - sent; // Envia em partes de 1024 bytes
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

static void start_http_server(void) {
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
    gpio_set_function(pin, GPIO_FUNC_PWM);    // Configurar o pino como saída de PWM

    uint slice_num = pwm_gpio_to_slice_num(pin);    // Obter o slice do PWM associado ao pino

    pwm_config config = pwm_get_default_config();  // Configurar o PWM com frequência desejada
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true);

    pwm_set_gpio_level(pin, 0); // Iniciar o PWM no nível baixo
}

void beep(uint pin, uint duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(pin); // Obter o slice do PWM associado ao pino

    pwm_set_gpio_level(pin, 3072);   // Configurar o duty cycle para 50% (ativo)

    sleep_ms(duration_ms);  // Temporização

    pwm_set_gpio_level(pin, 0); // Desativar o sinal PWM (duty cycle 0)

    sleep_ms(100); // Pausa de 100ms
}

int main(){
    stdio_init_all();   // Inicializa os tipos stdio padrão presentes ligados ao binário

    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    i2c_init(i2c1, ssd1306_i2c_clock * 1000); // Inicialização do i2c
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    gpio_init(LED_SEM_CONEXAO); // Configura o LED que pode ser controlado como saída.
    gpio_set_dir(LED_SEM_CONEXAO, GPIO_OUT);
    gpio_init(LED_CONECTADO);
    gpio_set_dir(LED_CONECTADO, GPIO_OUT);
    
    gpio_init(LED_PIN); // Configura o LED que pode ser controlado como saída.
    gpio_set_dir(LED_PIN, GPIO_OUT);

    pwm_init_buzzer(BUZZER_PIN);
    beep(BUZZER_PIN, 500); // Bipe de 500ms

    npInit(LED_PIN_MATRIZ);
    npClear();

    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B_PIN);

    ssd1306_init(); // Inicialização completa da Tela OLED SSD1306

    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length]; // Zera o display inteiro
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    restart:

    gpio_put(LED_SEM_CONEXAO, true); // Liga o LED

    char *text[] = {
        "   Conectando   ",
        "     . ao        ",
        "     Wi-Fi!   ",
        };

        int lines = count_of(text);  // Número de linhas de texto
        int text_height = lines * CHAR_HEIGHT; // Altura total do bloco de texto
        int start_y = (ssd1306_height - text_height) / 2; // Centralização vertical

        for (uint i = 0; i < lines; i++) {
            int text_width = strlen(text[i]) * CHAR_WIDTH;
            int x = (ssd1306_width - text_width) / 6; // Centralização horizontal
            int y = start_y + (i * CHAR_HEIGHT); // Ajuste vertical

            ssd1306_draw_string(ssd, x, y, text[i]);
        }
        render_on_display(ssd, &frame_area);

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
        printf("Falha ao conectar ao Wi-Fi\n");
    
        char *text[] = {
            "  Falha ao conectar   ",
            };
    
        int y = 0;
        for (uint i = 0; i < count_of(text); i++){
            ssd1306_draw_string(ssd, 5, y, text[i]);
            y += 8;
        }
        render_on_display(ssd, &frame_area);
        return 1;
    } else {
        printf("Conectado.\n");
        gpio_put(LED_SEM_CONEXAO, false);  // Desliga o LED

        char *text[] = {
            "   Conectado   ",
            "     . ao        ",
            "     Wi-Fi!   ",
            };

            int lines = count_of(text);  // Número de linhas de texto
            int text_height = lines * CHAR_HEIGHT; // Altura total do bloco de texto
            int start_y = (ssd1306_height - text_height) / 2; // Centralização vertical

            for (uint i = 0; i < lines; i++) {
                int text_width = strlen(text[i]) * CHAR_WIDTH;
                int x = (ssd1306_width - text_width) / 6; // Centralização horizontal
                int y = start_y + (i * CHAR_HEIGHT); // Ajuste vertical

                ssd1306_draw_string(ssd, x, y, text[i]);
            }

            render_on_display(ssd, &frame_area);

        beep(BUZZER_PIN, 500); // Bipe de 500ms

        for (size_t i = 0; i < 4; i++){
            gpio_put(LED_CONECTADO, true);
            desenharMatriz(matriz_wifi);
            sleep_ms(250);  // Espera 350ms
            gpio_put(LED_CONECTADO, false);
            desenharMatriz(matriz);
            sleep_ms(250);  // Espera 350ms
        }
        
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr); // Formatando o Endereço IP
        printf("Endereço IP %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
        IP[0] = ip_address[0]; // Recurso Alternativo :)
        IP[1] = ip_address[1];
        IP[2] = ip_address[2];
        IP[3] = ip_address[3];
    }

    printf("Wi-Fi conectado!\n");
    printf("Acesse o Endereço IP %d.%d.%d.%d/led/on", IP[0], IP[1], IP[2], IP[3]);

    start_http_server(); // Inicia o servidor HTTP

    while(true) {
        cyw43_arch_poll();  // Mantém o Wi-Fi ativo
        printf("Temperatura: %fC \n", temperatura_ambiente(false));

        if (gpio_get(BUTTON_A) == 0) {  // Botão pressionado (nível lógico baixo)
            printf("Botão \"A\" foi pressionado\n");
            abrir_portao();
        } else if (gpio_get(BUTTON_B_PIN) == 0){
            printf("Botão \"B\" foi pressionado\n");
            fechar_portao();
        }
        sleep_ms(1000);
    }
    cyw43_arch_deinit();  // Desliga o Wi-Fi 
    return 0;
}