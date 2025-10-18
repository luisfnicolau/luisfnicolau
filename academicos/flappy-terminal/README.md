# Flappy Bird - Terminal

Um jogo Flappy Bird completo implementado em C que roda diretamente no terminal!

## 🎮 Como Jogar

- **Pássaro**: Representado pelo caractere `#`
- **Obstáculos**: Representados pelo caractere `|` empilhados
- **Objetivo**: Passe pelos obstáculos sem colidir para marcar pontos

## 🎯 Controles

- **ESPAÇO**: Pular / Iniciar o jogo
- **R**: Reiniciar (após game over)
- **Q**: Sair (após game over)

## 🚀 Como Compilar e Executar

### Método 1: Usando Makefile
```bash
# Compilar
make

# Executar
make run

# Ou executar diretamente
./flappy_bird
```

### Método 2: Compilação Manual
```bash
gcc -Wall -Wextra -std=c99 -O2 -o flappy_bird flappy_bird.c
./flappy_bird
```

## 📋 Requisitos do Sistema

- Compilador C (gcc)
- Terminal com suporte a códigos de escape ANSI
- Sistema Unix/Linux/macOS

### Instalação de Dependências

**macOS:**
```bash
xcode-select --install
```

**Ubuntu/Debian:**
```bash
sudo apt-get install build-essential
```

**CentOS/RHEL:**
```bash
sudo yum groupinstall 'Development Tools'
```

## 🎮 Funcionalidades

- ✅ Física realista com gravidade
- ✅ Obstáculos com alturas aleatórias
- ✅ Sistema de pontuação
- ✅ Detecção de colisões
- ✅ Interface visual no terminal
- ✅ Controles responsivos
- ✅ Sistema de reinício

## 🔧 Estrutura do Código

- **Configurações**: Constantes definidas no topo do arquivo
- **Estruturas**: `Passaro`, `Obstaculo`, `Tela`
- **Funções de Sistema**: Configuração do terminal, limpeza da tela
- **Funções do Jogo**: Lógica principal, renderização, input

## 🎨 Personalização

Você pode modificar as seguintes constantes para personalizar o jogo:

```c
#define LARGURA_TELA 80        // Largura da tela
#define ALTURA_TELA 20         // Altura da tela
#define GRAVIDADE 0.2          // Força da gravidade (valores menores = mais suave)
#define IMPULSO -1             // Força do pulo (valores negativos)
#define LARGURA_OBSTACULO 4    // Largura dos obstáculos
#define VELOCIDADE_JOGO 0.5    // Velocidade do jogo (obstáculos se movem a cada 2 frames)
#define FPS_DELAY 33333        // Delay entre frames (33ms = 30 FPS)
```


## 🐛 Resolução de Problemas

**Terminal não limpa corretamente:**
- Certifique-se de que seu terminal suporta códigos ANSI
- Teste em um terminal diferente (Terminal.app, iTerm2, etc.)

**Compilação falha:**
- Verifique se o gcc está instalado
- Execute `make install` para ver instruções de instalação

**Jogo não responde:**
- Certifique-se de pressionar ESPAÇO para iniciar
- O jogo pode parecer travado se não houver input

---

**Divirta-se! 🎮**


**Contribuintes**
Luis Felipe Nicolau
Giovanna Nascimento Chaves

