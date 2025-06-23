# IcarusOS
# ![IcarusOS](./docs/IcarusOS-Final.png)

# ✨ IcarusOS — A Filosofia do Voo Baixo

## 🔥 Sobre o IcarusOS

**IcarusOS** é um simulador de núcleo de sistemas operacionais, projetado com fins acadêmicos e experimentais. Inspirado no mito de Ícaro, ele explora a dualidade entre o poder das abstrações e a importância do entendimento do funcionamento próximo ao hardware.

Na metáfora:

* ☀️ O **"sol"** representa as abstrações elevadas, que podem tornar o sistema poderoso, porém complexo e menos transparente.
* 🌊 O **"mar"** simboliza a operação em baixo nível, onde reside o controle, a eficiência e a compreensão profunda da máquina.

O **IcarusOS** não busca voar até o sol. Seu propósito é dominar o voo baixo, entendendo cada vento que sopra — cada ciclo, cada processo, cada operação de E/S — tal qual os sistemas operacionais fazem silenciosamente por trás das interfaces modernas.

---

## 📚 Descrição do Projeto

Este simulador implementa os principais componentes de um sistema operacional real:

* 🧠 **Gerenciamento de processos:** com criação, finalização, estados e BCP (Bloco de Controle de Processos).
* ⏱️ **Escalonamento:** algoritmo baseado na menor quantidade de operações de E/S já realizadas pelo processo (empate por ID).
* 🔧 **Sincronização:** suporte a semáforos (P e V), com controle de regiões críticas.
* 💾 **Gerenciamento de E/S:**

  * Disco com algoritmo **SSTF (Shortest Seek Time First)**.
  * Impressora com **fila simples FIFO**.
* 🧠 **Gerenciamento de memória:** paginação simulada.
* 🖥️ **Simulação de hardware:** CPU, Clock e Gerenciador de IO.
* 🗂️ **Interface Ncurses:** exibição visual do estado dos processos, filas, IO e clock em tempo real.

---

## ⚙️ Como Funciona

O sistema simula a execução de **programas sintéticos**, descritos em arquivos texto, que contêm:

* Cabeçalho (nome, ID, prioridade, tamanho, semáforos utilizados)
* Sequência de comandos como:

  * `exec t` — executar por `t` unidades de tempo
  * `read k` — leitura no disco (trilha `k`)
  * `write k` — escrita no disco (trilha `k`)
  * `P(s)` e `V(s)` — operações de semáforo
  * `print t` — imprimir por `t` unidades de tempo

O funcionamento do sistema é regido por um **relógio virtual**, que avança de acordo com a duração das operações executadas.

---

## 🏗️ Como Rodar o IcarusOS

### 🔧 Pré-requisitos

* GCC
* Make
* Biblioteca **Ncurses**
* Biblioteca **Pthreads** (padrão no GCC)

### 📦 Instalando Dependências (Linux/WSL)

```bash
sudo apt update
sudo apt install build-essential libncurses5-dev
```

### 📥 Clonando o projeto

```bash
git clone https://github.com/seu-usuario/IcarusOS.git
cd IcarusOS
```

### 🔨 Compilando

```bash
make
```

> 🔧 O Makefile irá automaticamente:
>
> * Criar os diretórios necessários (`obj/`)
> * Compilar todos os módulos
> * Linkar as bibliotecas Ncurses e Pthreads
> * Gerar o executável: `icarus_sim`

### ▶️ Executando

```bash
./icarus_sim
```

Ou:

```bash
make run
```

### 🧹 Limpando a build

```bash
make clean
```

---

## 📂 Estrutura do Projeto

```plaintext
IcarusOS/
├── Code/
│   ├── CPU/                # Simulação da CPU
│   ├── Clock/              # Relógio do sistema
│   ├── IO/                 # Gerenciador de entrada/saída
│   ├── Interface/          # Interface Ncurses
│   ├── Memoria/            # Gerenciamento de memória (paginação)
│   ├── Nucleo/             # Núcleo do sistema (Kernel e eventos)
│   ├── Process/            # Gerenciamento de processos
│   ├── Semaforo/           # Semáforos (sincronização)
│   ├── escalonador/        # Algoritmo de escalonamento
│   └── Ferramentas/        # Utilitários (listas, comparadores, etc.)
├── docs/                   # Documentação, imagens, banners
├── obj/                    # Arquivos objeto compilados
├── Makefile                # Script de build
├── icarus_sim              # Executável final
└── README.md               # Documentação do projeto
```

---

## 🚦 Especificações Implementadas

✅ Gerenciamento de processos com BCP
✅ Escalonamento por menor quantidade de E/S realizadas (empate por menor ID)
✅ Semáforos P(s) e V(s) com bloqueio e desbloqueio correto
✅ IO de disco com algoritmo SSTF (mínimo tempo de seek)
✅ IO de impressora com fila FIFO simples
✅ Simulação completa do clock, CPU e eventos do sistema
✅ Interface gráfica em terminal via Ncurses

---

## 🔍 Funcionamento Detalhado

* O sistema lê programas sintéticos e cria processos no BCP.
* Eventos são tratados através de duas funções centrais:

  * `interruptControl` → trata interrupções externas (fim de IO, criação de processos, etc.)
  * `sysCall` → trata chamadas do próprio processo (exec, read, write, P, V, print)

Cada evento dispara mudanças no estado dos processos, fila de IO, estado dos dispositivos e altera o escalonamento.

---

## 💡 Filosofia

> **“A sabedoria do voo não está em tocar o sol, mas em dominar a brisa constante da altitude perfeita.”**

O **IcarusOS** é mais do que um simulador. É uma ferramenta de aprendizado e reflexão sobre os fundamentos que sustentam os sistemas operacionais. Ele permite entender e visualizar, de forma concreta, como o núcleo de um SO gerencia processos, recursos e eventos.

---

## 🛠️ Contribuindo

Contribuições são muito bem-vindas!

Você pode:

* Sugerir melhorias
* Reportar bugs
* Implementar novas funcionalidades (ex.: logs, melhorias na interface, outros algoritmos de escalonamento)

---

## 📜 Licença

Distribuído sob a licença [MIT](LICENSE).

---

Se quiser, posso gerar esse arquivo `README.md` pronto, com formatação Markdown impecável, e até gerar badges (ex.: compilação funcionando, licença, versão, etc.)! Me fala se quer. 😎
