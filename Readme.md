# Projeto EmbarcaTech - Facilidade para o Dia-a-dia!

> Status do Projeto: Finalizado! :heavy_check_mark:

O projeto foi desenvolvido na linguagem C e utiliza o **[Kit BitDogLab](https://github.com/BitDogLab)** como base de desenvolvimento. O principal objetivo é facilitar o controle de elementos do dia-a-dia, como iluminação, climatização e segurança, através de uma interface acessível e intuitiva possibilitada por um servidor HTTP gerado pela placa que fica responsável por gerenciar esse controle de forma remota.

### Informações Adicionais

• Versão do Pico SDK: **[V1.5.1]()** <br>
• Placa de Desenvolvimento: **[Raspberry Pi Pico W]()**

## • Kit BitDogLab:

<div align="center">
  <p>Figura 1 - Kit BitDogLab</p>
  <img src="https://github.com/IsacBM/projetoEmbarcaTech/blob/main/REDME-FOTOS/1739573643015.jpg" width="500" alt="imagem-bitdoglab-frontal">
  <p>Kit BitDogLab Parte Frontal  - 2025</p>
</div>
<hr>
<div align="center">
  <p>Figura 2 - Kit BitDogLab</p>
  <img src="https://github.com/IsacBM/projetoEmbarcaTech/blob/main/REDME-FOTOS/1739573643001.jpg" width="500" alt="imagem-bitdoglab-inferior">
  <p>Kit BitDogLab Parte Inferior - 2025</p>
</div>

<h4 align="center">Saiba mais sobre o Kit Aqui:</h4>

<p align="center">
 <a href="https://github.com/BitDogLab/BitDogLab?tab=readme-ov-file" target="_blank"><img align="center" alt="Isac-PYTHON" height="40" width="105" src="https://img.shields.io/badge/visualizar-1db954?style=for-the-badge&logo=visualizar&logoColor=#00C7B7" target="_blank"></a>
</p>

## • Bibliotecas Utilizadas:

### 📌 Bibliotecas Padrão  
Essas bibliotecas são fornecidas pela linguagem C e são usadas para funcionalidades gerais, como entrada e saída de dados, manipulação de strings e alocação de memória.  

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
```

### 📡 Bibliotecas para Conectividade (Wi-Fi)  
Essas bibliotecas são utilizadas para comunicação via Wi-Fi na Raspberry Pi Pico W.  

```c
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
```

###  🖥️ Bibliotecas para Display OLED
Utilizadas para exibir informações na tela OLED.

```c
#include <ctype.h>
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
```

###  🌡️ Biblioteca para Sensor de Temperatura
Utilizada para a leitura de sensores analógicos.

```c
#include "hardware/adc.h"
```

###  🔊 Bibliotecas para Controle do Buzzer
Essas bibliotecas permitem gerar sinais PWM para controle do buzzer.

```c
#include "hardware/pwm.h"
#include "hardware/clocks.h"
```

###  🎨 Biblioteca para LED RGB
Biblioteca gerada automaticamente a partir do código PIO para controle do LED RGB WS2818B.

```c
#include "ws2818b.pio.h"
```

###  ⚙️ Outras Bibliotecas de Hardware
Utilizadas para operações de baixo nível com a Raspberry Pi Pico.


```c
#include "pico/stdlib.h"
#include "hardware/pio.h"
```

## • Resultados:

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/RcdylnDFs00/0.jpg)](https://www.youtube.com/embed/RcdylnDFs00)

## • Desenvolvedor Responsavél:
<div align="center">
 
|  [<img src="https://avatars.githubusercontent.com/u/91500212?v=4" width=115><br><sub>Isac B. Matos</sub>](https://github.com/IsacBM) |
| :---: |

</div>
<h4 align="center">
<strong>#EmbarcaTech</strong>💙 <br>
</h4>
