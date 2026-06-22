# IFES-CAMPUS-LINHARES
PROJETO FINAL - CONTROLE INTELIGENTE
![IFES - LINHARES](image.png)

Controle de Posição de um Aeropêndulo com Lógica Fuzzy

Este projeto apresenta o desenvolvimento de um aeropêndulo controlado por um controlador Fuzzy Mamdani implementado em um ESP32. O objetivo é controlar a posição angular de uma haste acionada por um conjunto motor-hélice, compensando as não linearidades e instabilidades características do sistema.

Hardware Utilizado
ESP32
Motor Brushless (BLDC)
VESC (Electronic Speed Controller)
Encoder incremental para medição angular
Bateria LiPo 11,1 V
Estrutura mecânica impressa em 3D e perfis metálicos

Estratégia de Controle

O sistema utiliza um controlador Fuzzy do tipo Mamdani com duas entradas:

Erro angular
Velocidade do erro

A saída do controlador corresponde a uma correção de corrente aplicada ao motor. Além disso, foi utilizada uma corrente de equilíbrio (bias) responsável por manter o aeropêndulo próximo ao ponto de operação, enquanto o controlador Fuzzy atua na rejeição de perturbações e correção dos desvios de posição.

A sintonia das funções de pertinência, dos singletons e da base de regras foi realizada de forma empírica e experimental, por meio de sucessivos ensaios no protótipo físico. Os parâmetros foram ajustados com base na observação da resposta dinâmica do sistema, buscando um compromisso entre estabilidade, rapidez de resposta e rejeição de perturbações

Características
Controle em malha fechada utilizando realimentação por encoder.
Implementação embarcada em ESP32.
Comunicação com o VESC via UART.
Regras Fuzzy baseadas em conhecimento heurístico.
Controle robusto para um sistema não linear e instável.

Este projeto foi desenvolvido como plataforma experimental para estudos de Controle Inteligente, Sistemas Embarcados e Lógica Fuzzy aplicados ao controle de sistemas dinâmicos.
