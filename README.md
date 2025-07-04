# ![IcarusOS](./docs/IcarusOS-Final.png)

# ✨ IcarusOS 

## 🔥 Sobre o IcarusOS

**IcarusOS** é um simulador de núcleo de sistemas operacionais, projetado com fins acadêmicos e experimentais. Inspirado no mito de Ícaro, ele explora a dualidade entre o poder das abstrações e a importância do entendimento do funcionamento próximo ao hardware.

Na metáfora:

* ☀️ O **"sol"** representa as abstrações elevadas, que podem tornar o sistema poderoso, porém complexo e menos transparente.
* 🌊 O **"mar"** simboliza a operação em baixo nível, onde reside o controle, a eficiência e a compreensão profunda da máquina.

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
git clone https://github.com/Ventoland/IcarusOS.git
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

## 💡 Filosofia

> **“A sabedoria do voo não está em tocar o sol, mas em dominar a brisa constante da altitude perfeita.”**

---

## 🛠️ Contribuindo

Contribuições são muito bem-vindas!

Você pode:

* Sugerir melhorias
* Reportar bugs
* Implementar novas funcionalidades (ex.: logs, melhorias na interface, outros algoritmos de escalonamento)
