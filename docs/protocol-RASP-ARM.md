= Interface RASP <--> ARM
José Roberto B. A. Monteiro <jrm@sc.usp.br>
v0.1, 2020-05-01
:sectanchors:
:toc:
:data-uri:

== Protocolo físico adotado: UART

* NO hardware flow control / Sem controle de fluxo por hardware
* No parity bit / Sem bit de paridade
* 8 bits of data / 8 bits de dados
* 2 stop bits for transmission and one for reception.
* Baudrate: `to be determined` / Taxa de transferência: a ser determinado

== Syntaxe dos Comandos

O formato de envio de dados entre os dispositivos é ASCII, os comandos são separados dos parâmetros por espaço (0x20) e cada conjunto (comando e parâmetro) é separado por _linefeed_ (0x0a) ou _carriage return_ (0x0d).

Os comandos possuema seguinte syntaxe:

`x:cmd\[!] \[parâmetro] \n[\r]`

Onde: `x` pode ser:

* `r` para o respirador,
* `1` a `4` para as bombas de infusão de 1 a 4,
* `m` para o monitor cardíaco e
* `o` para o monitor de taxa de oxigenação do sangue

A presença do `!` vai indicar se é uma escrita ou uma leitura do parâmetro (ausência do `!`).

O `parâmetro` vai depender se o comando possui ou não um parâmetro a ser enviado.

=== Respostas do ARM

A resposta do ARM:
. para comandos que não requisitam parâmetros, é o próprio comando e o valor do código de erro ou sucesso (_status-code_), e
. para comandos que requisitam parâmetros, é o comando e o parâmtetro, caso não tenha ocorrido nenhum erro, ou o comando e o código do erro (_status-code_), no caso da ocorrência de erro.

.Códigos de estado para a resposta a um comando
[options="headers"]
|===
| Descrição | _status-code_
| Comando executado com sucesso | `success`
| Erro durante a execução do comando | `exec-error`
| Comando desconhecido | `unknown-command`
| Número de parâmetros incorretos | `parameter-number-error`
| Módulo não existente | `unknown-module`
| Tentativa de escrever em parâmetro somente leitura | `read-only-parameter`
| Tentative de ler parâmetro somente escrita | `write-only-parameter`
| Comando somente de execução | `exec-only-cmd`
| Erro não determinado | `undetermined`
|===

=== Comandos Gerais

O comando pode ser geral, para todo o equipamento, ou específicos para um módulo, por exemplo, específico para o respirador ou para uma bomba de infusão.

.Comandos Gerais
|===
| Nome | Descrição
| `firmware` | retorna a versão do firmware instalado
|===

=== Respirador

.Comandos que executam ações
[cols="<2,<", options="header"]
|===
| Nome da Ação | Comando
| Para o processo, colocando o cilindro na posição inicial | `stop`
| Inicia o processo | `start`
|===

Exemplo:

* Raspberry: `r:stop \n`
* ARM: `r:stop _status-code_ \n`

.Parâmetros para o Respirador
[cols="<,<4,^,>,<", options="header"]
|===
| Parâmetro | Descrição | Tipo | _Default_ | Unidade
| `mode` | Modo de operação, da tabela "Modos de Operação" | _uint_ | 0 |
| `air` | Volume de ar inspirado | _uint32_ | 200 | mL
| `O2` | Volume de O~2~ a inspirado | _uint32_ | 0 | mL
| `insp-time` | Tempo total de inspiração | _uint32_ | 400 | ms
| `vexp`  | Volume expirado forçado | _uint32_ | 0 | mL
| `texpf` | Tempo de expiração forçada | _uint32_ | 0 | ms
| `texpn` | Tempo de expiração natural | _uint32_ | 600 | ms
| `ppress` | Valor da pressão positiva máxima | _uint32_ | 80 | mmHg
| `npress` | Valor da pressão negativa máxima (valor absoluto) | _uint32_ | 20 | mmHg
|===

.Modos de Operação
[cols="2", option="headers"]
|===
| Modo de operação | Valor
| Inspiração a fluxo constante | 0
| Inspiração a pressão constante | 1
|===

Exemplos:

* Para alterar o modo de operação para pressão constante:

** Rasp: `r:mode! 1\n`
** ARM: `r:mode! _status-code_\n`

* Para ler o modo de operação:

** Rasp: `r:mode\n`
** ARM: `r:mode 1\n`

=== Bombas de Infusão

.Ações para a Bomba de Infusão
[cols="<,<5", options="header"]
|===
| Comando | Descrição
| `calib` | Avança até o final do curso e depois volta para a posição inicial, a seringa deve estar ausente, caso contrário, não executa a ação e retorna erro.
| `start` | Habilita o processo
| `stop` | Desabilita o processo.
|===

Exemplo:

* Para calibrar a bomba de infusão número 2:

** Rasp: `2:calib\n`
** Resposta em caso de sucesso: `2:calib success\n`
** Em caso de não conseguir atingir sensores: `2:calib exec-error\n`

.Parâmetros para as bombas de infusão
[cols="<,<4,^,>,<,^", options="header"]
|===
| Parâmetro | Descrição | Tipo | _Default_ | Unidade | Acesso
| `steps` | Posição atual do êmbolo em passos, retorna -1 se o valor não está calibrado  | _int32_ | -1 | passos | RO
| `max-steps` | Valor máximo de passos do sistema mecânico, se estiver calibrado, se não, retorna -1 | _int32_ | -1 | passos | RO
| `ser`| Retorna verdadeiro se o sensor de seringa está pressionado | _boolean_ | `false` | | RO
| `go` | Registro de voltas a executar: executa um passo e decrementa o parâmetro ou registrador. Repete o ciclo até o registrador estar zerado. A leitura desse parâmetro indica o número de passos que faltam para terminar o processo. O tempo de intervalo entre cada passo é determinado pelo registrador `time-step` | _int32_ | 0 | passos | RW
| `time-step` | Intervalo de tempo entre cada passo, utilizado para a instrução acima. | _uint32_ | 200 | us | RW 
|===
