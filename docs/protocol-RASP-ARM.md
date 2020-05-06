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
* Baudrate: `to be determined` / Taxa de transferência: a ser determinado

== Emolduração do pacote de dados (Data packet framing)

Cada pacote de dados precisa ter uma sinalização de início e término, devido a se ter adotado controle de fluxo por hardware. Esse início e fim é sinalizado por uma sequência de bytes, sendo adotada a sequência de 2 bytes: 0x80 0x00 (`80 00`). Isso tem o efeito colateral de que cada ocorrência do byte `80` em um pacote de dados precisa ser protegida, isto é, precisa ser transformada para que uma possível ocorrência da sequência `80 00` dentro de um pacote não seja interpretada como término ou fim de pacote. Portanto, cada ocorrência do byte `80` deve ser substituída por `80 XX` onde `XX` é o número de bytes consecutivos iguais a `80`, onde `XX` pode variar de 1 a 255. Caso existam mais bytes `80` em sequência, outra sequência de bytes `80 XX` deve ser utilizada até que o número total de bytes `80` em sequência seja atingido.
Exemplos:

* Data Packet: `00 01 0f 80 fd`

* Transmited as: `*80 00* 01 0f 80 01 fd *80 00*`

Onde: os dois primeiros bytes sinalizam o início do pacote, `80 01` corresponde a uma ocorrência do byte `80` e a sequência final `80 00` corresponde ao final do pacote. Caso pacotes adjacentes sejam transmitidos, não há a necessidade de se duplicar as sequências `80 00`, apenas uma é necessária. Exemplo:

* Data packet 1: `2f 5c 98 80 80 04`
* Data packet 2: `3f 49 5b 81 00`

* Transmitted as: `*80 00* 2f 5c 98 *80 02* 04 *80 00* 3f 49 5b 81 00 *80 00*`

== Comandos e Pacotes de Dados

Cada comando corresponde a um pacote de dados com pelo menos 2 bytes de tamanho. O equipamento interno ao qual se refere esse comando é selecionado por ele mesmo, no primeiro byte, pelos 4 bits menos significativos. O segundo byte seleciona um parâmetro (ou registrador) desse equipamento ou uma ação a ser executada.

image::packet.svg[Pacote de dados]

NOTE: Para os dados, o LSB é enviado primeiramente.

.Número para o equipamento interno `N`
[cols="^,<4", options="header"]
|===
| `N` | Equipamento
| `0` | Respirador
| `1` | Bomba de Infusão #1
| `2` | Bomba de Infusão #2
| `3` | Bomba de Infusão #3
| `4` | Bomba de Infusão #4
| `5` | Monitor Cardíaco
| `6` | Monitor da Taxa de Oxigenação
|===


.Comandos originados do Mestre -- Raspberry: `CMD` é o número do comando.
[cols="2", options="header"]
|===
| Command Name | CMD 
| Execution command | `0` 
| Set parameter  | `1`
| Get parameter  | `2`
|===

A resposta dada pelo escravo (ARM) é composta por 3 bytes: O primeiro byte é o de estado e os dois seguintes correspondem a `CMD+N P`, ou seja, o número do comando e o número do equipamento no segundo byte e o número do parâmetro ou da ação executada no terceiro byte.

.Bytes de Estado para a Resposta do Escravo
[options="headers"]
|===
| Comando executado com sucesso | `e0`
| Erro durante a execução do comando | `e1`
| Comando desconhecido | `e2`
| Número de bytes de dados incorretos | `e3`
| Erro na syntaxe do comando | `e4`
| Parâmetro ou ação inexistente | `e5`
| Dados incorretos | `e6` 
| Erro não determinado | `e7`
|===

=== Respirador

.Ações para o Respirador
[cols="<2,^,^", options="header"]
|===
| Nome da Ação | `CMD+N` | `P`
| Para o processo | `00` | `00`
| Continua o processo | `00` | `01` 
| Coloca o cilindro na posição 0 (o processo deve estar parado)  | `00` | `02`
|===

.Parâmetros para o Respirador
[cols="<5,^,^,^,^,^", options="header"]
|===
| Parâmetro | `P` | Tipo | _Default_| Unidade | Acesso 
| Firmware Version, tamanho variável | `00` | _uint8_[n] | software_firmware | | RO 
| Modo de operação, da tabela "Modos de Operação" | `01` | _uint_8_ | 0 | | RW 
| Volume de ar inspirado | `02` | _float_32_ | 200 | mL | RW
| Volume de O~2~ inspirado | `03` | _float_32_ | 0 | mL | RW
| Tempo total de inspiração (s) | `04` |  _float_32_ | 0,4 | s | RW
| Volume expirado forçado | `05` | _float_32_ | 0 | mL | RW
| Tempo de expiração forçada | `05` | _float_32_ | 0 | s | RW
| Tempo de expiração natural | `06` | _float_32_ | 0,6 | s | RW
| Pressão positiva máxima | `07` | _float_32_ | 0,1 | Bar | RW
| Pressão negativa máxima (valor absoluto) | `08` | _float_32_ | 0,05 | Bar | RW
|===

.Modos de Operação
[cols="2", option="headers"]
|===
| Modo de operação | Byte
| Inspiração a fluxo constante | 0x00
| Inspiração a pressão constante | 0x01
|===

=== Bombas de Infusão

.Ações para a Bomba de Infusão
[cols="<5,^", options="header"]
|===
| Nome da Ação (`CMD=0`, `N=0,1,2,3`) | `P`
| Avança até o final do curso e depois volta para a posição inicial, a seringa deve estar ausente, caso contrário, não executa a ação e retorna erro. | `00`
| Habilita o processo | `01` 
| Desabilita o processo  | `02`
|===

Exemplo:

* Para calibrar a bomba de infusão número 2:
+
Pacote de dados: `02 00`
+
Transmissão: `80 00 02 00 80 00`
+
Resposta em caso de sucesso: `80 00 e0 02 00 80 00`
+
Em caso de não conseguir atingir sensores: `80 00 e1 02 00 80 00`

.Parâmetros para o Respirador
[cols="<5,^,^,^,^,^", options="header"]
|===
| Parâmetro | `P` | Tipo |  _Default_ | Unidade | Acesso
| Retorna posição atual do êmbolo em passos, negativo se o valor não está calibrado | `00` | _int_32_| -1 | passos | RO
| Retorna o valor máximo de passos do sistema mecânico, se estiver calibrado, se não, retorna -1 | `01` | _int32_ | -1 | passos | RO 
| Retorna verdadeiro se o sensor de seringa está pressionado | `02` | _int8_ | | Boolean | RO
| Registro de voltas a executar: executa um passo e decrementa o parâmetro ou registrador. Repete o ciclo até o registrador estar zerado. A leitura desse parâmetro indica o número de passos que faltam para terminar o processo. O tempo de intervalo entre cada passo é determinado pelo registrador seguinte | `03` | _int32_ | 0 | passos | RW
| Intervalo de tempo entre cada passo, utilizado para a instrução acima. | `04` | _float32_ | 200 | us | RW 
|===
