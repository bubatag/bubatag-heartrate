# 🐃 BUBATAG — Monitoramento Biométrico

### Leitura de batimentos cardíacos e temperatura corporal utilizando ESP32 e sensores.

O **BUBATAG Heartrate** é um módulo de desenvolvimento embarcado do projeto BUBATAG, responsável pela aquisição de dados fisiológicos por meio de sensores conectados a um microcontrolador ESP32.

O firmware integra o sensor óptico **MAX30102**, utilizado para estimativa da frequência cardíaca, e o sensor de temperatura **DS18B20**, permitindo realizar leituras de diferentes parâmetros em um único dispositivo.

A proposta é desenvolver e testar a aquisição de dados biométricos que futuramente poderão integrar o sistema de monitoramento inteligente de bubalinos do BUBATAG.

---

## 📌 Sobre o projeto

Este repositório contém o firmware responsável pela comunicação entre o ESP32 e os sensores utilizados no protótipo de monitoramento biométrico.

O desenvolvimento utiliza C++ com framework Arduino e gerenciamento de dependências por meio do PlatformIO.

O firmware realiza a leitura contínua dos dados do MAX30102 e a aquisição periódica da temperatura pelo DS18B20, apresentando os resultados no Monitor Serial.

### Objetivos

- Realizar a leitura da frequência cardíaca por meio do sensor MAX30102.
- Estimar os batimentos por minuto (BPM).
- Calcular uma média das últimas leituras válidas de frequência cardíaca.
- Monitorar a temperatura utilizando o sensor DS18B20.
- Integrar diferentes sensores ao ESP32.
- Desenvolver a base eletrônica e de firmware para o futuro monitoramento fisiológico dos bubalinos.

## ✨ Funcionalidades

| Funcionalidade | Descrição |
|---|---|
| **Leitura cardíaca** | Obtém os dados ópticos do sensor MAX30102. |
| **Detecção de batimentos** | Identifica pulsos por meio do algoritmo `checkForBeat()`. |
| **Cálculo de BPM** | Estima a frequência cardíaca em batimentos por minuto. |
| **Média de BPM** | Calcula a média das últimas quatro leituras válidas. |
| **Leitura de temperatura** | Obtém a temperatura pelo sensor DS18B20. |
| **Leitura não bloqueante** | Evita que a conversão de temperatura interrompa continuamente a aquisição cardíaca. |
| **Monitoramento serial** | Apresenta os resultados e informações de funcionamento no Monitor Serial. |

### Como funciona a leitura

**1. MAX30102 — Frequência cardíaca**

O sensor utiliza tecnologia óptica para identificar variações na intensidade da luz refletida pelos tecidos.

O firmware recebe as amostras infravermelhas (IR), identifica os batimentos e calcula a frequência cardíaca em BPM.

**2. DS18B20 — Temperatura**

O sensor realiza a medição da temperatura e envia os dados ao ESP32 por meio do protocolo OneWire.

O firmware solicita novas conversões periodicamente e atualiza a temperatura apresentada no Monitor Serial.

**3. ESP32 — Processamento**

O microcontrolador recebe e processa as informações dos sensores, organizando as leituras para apresentação no Monitor Serial.

## 🛠️ Tecnologias e componentes utilizados

### Hardware

| Componente | Aplicação |
|---|---|
| **ESP32 DevKit** | Microcontrolador responsável pelo processamento e comunicação com os sensores. |
| **MAX30102** | Sensor óptico utilizado para estimativa da frequência cardíaca. |
| **DS18B20** | Sensor digital utilizado para medição de temperatura. |
| **Protoboard e jumpers** | Utilizados para realizar as conexões elétricas do protótipo. |

### Software

| Tecnologia | Aplicação |
|---|---|
| [C++](https://isocpp.org/) | Linguagem utilizada no firmware. |
| [Arduino Framework](https://docs.arduino.cc/) | Framework para programação do ESP32. |
| [PlatformIO](https://platformio.org/) | Ambiente de desenvolvimento, compilação e envio do firmware. |
| [Visual Studio Code](https://code.visualstudio.com/) | Editor de código recomendado. |

### Bibliotecas utilizadas

As dependências são declaradas no arquivo `platformio.ini` e instaladas automaticamente pelo PlatformIO.

| Biblioteca | Aplicação |
|---|---|
| SparkFun MAX3010x | Comunicação com o MAX30102 e detecção de batimentos. |
| DallasTemperature | Gerenciamento das leituras do DS18B20. |
| OneWire | Comunicação com o sensor DS18B20. |
| TinyGPSPlus | Dependência declarada para funcionalidades relacionadas a GPS. |

**Observação:** a biblioteca TinyGPSPlus já está declarada no projeto, mas sua utilização ainda não está implementada no firmware atual.

## 🔌 Conexões dos componentes

Antes de executar o firmware, conecte os sensores ao ESP32 conforme a tabela abaixo.

### 1. MAX30102 — Sensor cardíaco

O MAX30102 utiliza comunicação I²C.

| MAX30102 | ESP32 |
|---|---|
| VIN | 3V3 |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

### 2. DS18B20 — Sensor de temperatura

O DS18B20 utiliza o protocolo OneWire.

| DS18B20 | ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| DATA | GPIO 4 |

**Importante:** para o DS18B20 em montagem convencional, utilize um resistor de pull-up de aproximadamente 4,7 kΩ entre DATA e VCC. Verifique também o modelo e a identificação dos terminais do seu sensor antes de realizar as conexões.

### Resumo das conexões

| Sensor | Protocolo | Pino de dados |
|---|---|---|
| MAX30102 | I²C | GPIO 21 (SDA) e GPIO 22 (SCL) |
| DS18B20 | OneWire | GPIO 4 |

## 💻 Como configurar e executar o projeto

Siga as etapas abaixo para preparar o ambiente, compilar e carregar o firmware no ESP32.

### 1. Pré-requisitos

Antes de começar, instale:

- **Visual Studio Code:** editor de código.
- **PlatformIO IDE:** extensão para desenvolvimento de sistemas embarcados.
- **Git:** ferramenta para clonar o repositório.
- **Driver USB:** conforme o conversor USB-serial utilizado na sua placa ESP32.

Links para instalação:

- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO IDE](https://platformio.org/install/ide?install=vscode)
- [Git](https://git-scm.com/)

### 2. Clonar o repositório

Abra o terminal ou PowerShell e execute:

```bash
git clone https://github.com/bubatag/bubatag-heartrate.git
```

Entre na pasta do projeto:

```bash
cd bubatag-heartrate
```

### 3. Abrir o projeto no Visual Studio Code

Com o Visual Studio Code instalado, abra a pasta do projeto:

```bash
code .
```

Certifique-se de que a extensão PlatformIO IDE esteja instalada e habilitada.

O PlatformIO identificará automaticamente o arquivo `platformio.ini` e preparará o ambiente de desenvolvimento.

### 4. Instalar as dependências

As bibliotecas necessárias estão declaradas no arquivo `platformio.ini`.

Ao abrir o projeto pelo PlatformIO, as dependências serão instaladas automaticamente durante a preparação do ambiente ou na primeira compilação.

Caso esteja utilizando o PlatformIO CLI, execute:

```bash
pio pkg install
```

### 5. Conectar o ESP32

Conecte a placa ESP32 ao computador utilizando um cabo USB compatível com transferência de dados.

Verifique se a placa foi reconhecida pelo sistema operacional e identifique a porta serial correspondente.

No Windows, normalmente será identificada como uma porta `COM`, por exemplo:

```text
COM4
```

A porta pode variar de acordo com o computador e o dispositivo conectado.

### 6. Compilar o firmware

Antes de enviar o código para a placa, execute a compilação:

```bash
pio run
```

Esse comando verifica o código e compila o firmware para o ESP32.

### 7. Enviar o firmware para o ESP32

Com a placa conectada, execute:

```bash
pio run --target upload
```

O PlatformIO realizará o envio do firmware para o microcontrolador.

Caso necessário, selecione a porta serial correta nas configurações do projeto ou informe-a diretamente:

```bash
pio run --target upload --upload-port COM4
```

Substitua `COM4` pela porta correspondente à sua placa.

### 8. Abrir o Monitor Serial

Após o envio do firmware, execute:

```bash
pio device monitor --baud 115200
```

A velocidade de comunicação configurada no projeto é de **115200 baud**.

Se tudo estiver funcionando corretamente, o Monitor Serial apresentará as mensagens de inicialização dos sensores e, posteriormente, as leituras biométricas.

## 📊 Monitoramento e saída serial

O firmware apresenta informações dos sensores periodicamente pelo Monitor Serial.

### Exemplo de saída

```text
==========================================
   BUBATAG - Monitor Biometrico Completo
==========================================

Inicializando MAX30102...
MAX30102 encontrado!
DS18B20 inicializado!
Sistema pronto!

------------------------------------------
IR: 15234
Batimento Cardiaco: 72.5 BPM (Media: 71 BPM)
Temperatura Corporal: 38.2 C
------------------------------------------
```

*Os valores apresentados são apenas ilustrativos. As leituras reais dependem dos sensores, das condições de utilização e da qualidade do sinal.*

### Interpretação dos dados

| Informação | Significado |
|---|---|
| `IR` | Intensidade da leitura infravermelha recebida pelo MAX30102. |
| `BPM` | Estimativa instantânea dos batimentos por minuto. |
| `Media` | Média calculada a partir das últimas quatro leituras válidas. |
| `Temperatura Corporal` | Temperatura obtida pelo DS18B20 em graus Celsius. |

### Intervalos de atualização

- **MAX30102:** leitura realizada continuamente durante a execução do firmware.
- **DS18B20:** atualização da temperatura a cada 5 segundos.
- **Monitor Serial:** apresentação dos resultados a cada 5 segundos.

## 📁 Estrutura do projeto

O repositório utiliza uma estrutura simples do PlatformIO para organização do firmware.

```text
bubatag-heartrate/
│
├── .vscode/
│   └── settings.json       # Configurações do ambiente de desenvolvimento
│
├── src/
│   └── main.cpp            # Firmware principal do projeto
│
├── .gitignore              # Arquivos e diretórios ignorados pelo Git
│
├── platformio.ini          # Configuração da placa e dependências
│
└── README.md               # Documentação do projeto
```

### Principais arquivos

| Arquivo | Responsabilidade |
|---|---|
| `src/main.cpp` | Contém a lógica de inicialização, leitura e processamento dos sensores. |
| `platformio.ini` | Define a placa ESP32, framework Arduino, velocidade serial e bibliotecas. |
| `.gitignore` | Evita o versionamento de arquivos desnecessários. |
| `.vscode/` | Armazena configurações do ambiente de desenvolvimento. |

## ⚠️ Observações e limitações

Este repositório representa uma etapa de desenvolvimento e testes do módulo biométrico do BUBATAG.

Alguns pontos importantes:

- As leituras do MAX30102 dependem de contato adequado, estabilidade e qualidade do sinal óptico.
- O algoritmo utilizado é destinado à estimativa de frequência cardíaca e pode apresentar variações nas medições.
- A utilização do MAX30102 em bubalinos precisa de validação experimental, considerando as características anatômicas e as condições de fixação do sensor.
- A leitura do DS18B20 depende do posicionamento e do contato térmico adequado.
- O firmware atual apresenta os dados pelo Monitor Serial; a transmissão para a API e o armazenamento em banco de dados fazem parte de etapas de integração do ecossistema BUBATAG.

Os valores obtidos neste protótipo não devem ser considerados diagnósticos veterinários.

## 🚧 Próximas etapas

O desenvolvimento do módulo biométrico poderá evoluir com a implementação de novas funcionalidades:

- Integração com módulos de comunicação sem fio.
- Implementação de leitura e processamento de dados GPS.
- Envio das informações biométricas para a API do BUBATAG.
- Armazenamento e histórico de medições.
- Integração com a plataforma web de monitoramento.
- Avaliação e validação das leituras em condições reais de utilização com bubalinos.
- Otimização do consumo energético para utilização em coleiras inteligentes.

## 🎓 Sobre o desenvolvimento

O BUBATAG é um Projeto Integrador desenvolvido no curso de **Tecnologia em Desenvolvimento de Software Multiplataforma**, da Faculdade de Tecnologia de Registro (Fatec Registro), vinculada ao Centro Paula Souza.

O projeto reúne conhecimentos de eletrônica, sistemas embarcados, Internet das Coisas (IoT) e desenvolvimento de software, buscando desenvolver uma solução tecnológica para auxiliar o monitoramento e o bem-estar dos bubalinos.

---

<div align="center">

**🐃 BUBATAG — Conectando tradição e inovação.**

*Tecnologia a serviço do bem-estar animal e da produção sustentável.*

[💻 Repositório](https://github.com/bubatag/bubatag-heartrate) · [🔌 API](https://github.com/bubatag/API-bubatag) · [🌐 Plataforma](https://github.com/bubatag/sistema-frontend)

</div>
