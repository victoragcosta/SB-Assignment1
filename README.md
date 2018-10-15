# Software básico - Trabalho 01

## Descrição:

Desenvolvimento de um montador e de um ligador para um assembly simplificado.
Trabalho 01 da disciplina Software básico 2018/2 da Universidade de Brasília.

## Integrantes:

Nome | Matrícula | Sistema operacional
---  | --- | ---
André Laranjeira | 16/0023777 | Manjaro linux 64-bit
Victor André Gris Costa | 16/0019311 | WSL Ubuntu 14.04 no Windows 10 64-bit

## Compilação e Execução do Código:

Para compilar o código do montador basta acessar a pasta /Montador e execute o comando make.

Para executar o montador basta chamar ```./Montador nome_do_arquivo_sem_asm``` na pasta /Montador e ele gerará os arquivos *.pre e *.obj na mesma pasta que está.

Para compilar o código do ligador basta acessar a pasta /Ligador e execute o comando make.

Para executar o ligador basta chamar ```./Ligador arquivo1 arquivo2 arquivo3 arquivo4``` na pasta /Ligador e ele gerará os arquivos *.e na mesma pasta que está.

---

## Tratamento de Erros
### Passagem 0:

* Léxicos:
  * [x] tokens inválidos (EQU e IF)
* Sintáticos:
  * [x] label antes de IF

### Passagem 1:

* Léxicos:
  * [x] tokens inválidos (Todo o resto)
* Sintáticos:
  * [x] dois rótulos na mesma linha
  * [x] seção inválida
* Semânticos:
  * [x] declarações e rótulos repetidos
  * [x] seção TEXT faltante

### Passagem 2:

* Sintáticos:
  * [ ] instruções inválidas
  * [ ] diretivas inválidas
  * [ ] divisão por zero (para constante)
  * [ ] instruções com a quantidade de operando inválida
  * [ ] tipo de argumento inválido

* Semânticos:
  * [ ] declarações e rótulos ausentes
  * [ ] pulo para rótulos inválidos
  * [ ] pulo para seção errada
  * [ ] diretivas ou instruções na seção errada
  * [ ] modificação de um valor constante
