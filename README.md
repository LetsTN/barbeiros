# O Problema dos Barbeiros Dorminhocos (com múltiplos barbeiros)

Esse problema foi proposto no trabalho 2 da disciplina de FPP.

Em ciência da computação, o problema do barbeiro dorminhoco é um problema clássico de comunicação inter-threads e sincronização entre múltiplas threads.

A questão é manter os barbeiros ocupados enquanto há clientes, e descansando quando não há nenhum (fazendo isso de uma maneira ordenada).

## O problema (como o professor passou literalmente)

Na barbearia há N barbeiros, N cadeiras de barbeiro e X cadeiras para eventuais clientes
esperarem a vez. Quando não há clientes, os barbeiros sentam-se nas suas cadeiras de barbeiro e cai no sono. Quando chega um cliente, ele precisa acordar algum barbeiro. Se outros clientes chegarem enquanto o barbeiro estiver cortando o cabelo de um cliente, eles se sentarão (se houver cadeiras vazias) ou sairão da barbearia (se todas as cadeiras estiverem ocupadas). O problema é programar o barbeiro e os clientes sem cair em condições de disputa. Esse problema é semelhante a situações com várias filas, como uma mesa de atendimento de telemarketing com diversos atendentes e com um sistema computadorizado de chamadas em espera, atendendo a um número limitado de chamadas que chegam.

Seu programa deve paralelizar isso, de forma a evitar todos os problemas padroes da paralelizacao classica, e receber como argumentos N e X pela linha de comando alem de um Z que eh o numero de clientes minimo que cada barbeiro atendeu.
O codigo final tem que imprimir exatamente na ordem dos barbeiros o quanto de cliente eles ja atenderam. seu codigo final NAO DEVE IMPRIMIR NENHUM DEBUG, ou seja, comentem suas linhas de codigo de debug MAS DEIXEM ELA NO CODIGO para eu analisar.

## Como compilar os arquivos 

Por mais que já tenha o arquivo compilado (```barbeiros```), caso você mesmo queira compilar o arquivo, basta rodar:

```sh
gcc -Wall -pedantic -o barbeiros barbeiros.c -lpthread
```

## Como executar

Para executar o código, basta rodar:

```sh
./barbeiros n x z
```

Onde ```n``` é a quantidade de barbeiros, ```x``` a quantidade de cadeiras de espera e ```z``` a quantidade mínima de clientes que cada barbeiro deve atender.
