# q-learning-simples
O objetivo deste projeto é a familiarização com o algoritmo Q-learning. Para isso, criei um programa simples que faz uso do algoritmo em questão.

## Funcionamente
Temos uma entidade `entity` que possui uma posição `p` numa grid n por n. Essa entidade precisa alcançar um objetivo localizado na posição `p_o`. A cada posição `p` da entidade, será associado um estado `s_p` que, por sua vez, possuirá um conjunto de ações (movimentos) possíveis de serem realizados, a exemplo: andar para cima, baixo, esquerda ou direita. 

O algoritmo, por meio de uma política simples, decidirá como o caminho será escolhido: de forma aleatória ou escolhendo o melhor caminho possível em seu estado `s_p`, e para isso será necessário atribuir um valor a cada ação possível a fim do algoritmo poder decidir a melhor rota a se tomar.
