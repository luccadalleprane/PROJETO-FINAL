```html
<h1 align="center">Controle de Posição de um Aeropêndulo com Lógica Fuzzy</h1>

<p align="center">
  <strong>Projeto Final - Controle Inteligente</strong><br>
  Instituto Federal do Espírito Santo (IFES) - Campus Linhares
</p>

<p align="center">
  <img src="image.png" alt="IFES - Linhares" width="600">
</p>

<h2>📖 Sobre o Projeto</h2>

<p>
Este projeto apresenta o desenvolvimento de um sistema de controle de posição angular para um aeropêndulo utilizando
Lógica Fuzzy do tipo Mamdani implementada em um ESP32.
</p>

<p>
O aeropêndulo consiste em uma haste articulada acionada por um conjunto motor-hélice, caracterizado por uma dinâmica
não linear e naturalmente instável. O objetivo do controlador é estabilizar a posição angular da haste e rejeitar
perturbações externas, mantendo o sistema próximo ao ponto de operação desejado.
</p>

<hr>

<h2>🔧 Hardware Utilizado</h2>

<ul>
  <li>ESP32</li>
  <li>Motor Brushless DC (BLDC)</li>
  <li>VESC (Vedder Electronic Speed Controller)</li>
  <li>Encoder incremental para medição da posição angular</li>
  <li>Bateria LiPo 11,1 V</li>
  <li>Estrutura mecânica composta por peças impressas em 3D e perfis metálicos</li>
</ul>

<hr>

<h2>🧠 Estratégia de Controle</h2>

<p>
O sistema utiliza um controlador Fuzzy Mamdani com duas variáveis de entrada:
</p>

<ul>
  <li>Erro angular</li>
  <li>Velocidade do erro</li>
</ul>

<p>
A saída do controlador corresponde a uma correção de corrente aplicada ao motor. Além disso, foi implementada uma
corrente de equilíbrio (<em>bias current</em>), responsável por manter o aeropêndulo próximo ao ponto de operação,
enquanto o controlador Fuzzy realiza a compensação de perturbações e corrige desvios de posição.
</p>

<p>
A sintonia das funções de pertinência, dos singletons e da base de regras foi realizada de forma empírica e experimental,
por meio de sucessivos ensaios no protótipo físico. Os parâmetros foram ajustados com base na observação da resposta
dinâmica do sistema, buscando um compromisso entre estabilidade, rapidez de resposta e rejeição de perturbações.
</p>

<hr>

<h2>⚙️ Principais Características</h2>

<ul>
  <li>Controle em malha fechada com realimentação por encoder;</li>
  <li>Implementação embarcada em ESP32;</li>
  <li>Comunicação com o VESC via UART;</li>
  <li>Controlador Fuzzy Mamdani baseado em regras heurísticas;</li>
  <li>Compensação de perturbações através de corrente de equilíbrio e ação Fuzzy;</li>
  <li>Aplicação em um sistema não linear e instável.</li>
</ul>

<hr>

<h2>🎯 Objetivos do Projeto</h2>

<ul>
  <li>Desenvolver uma plataforma experimental para estudos de controle inteligente;</li>
  <li>Aplicar técnicas de Lógica Fuzzy em sistemas dinâmicos reais;</li>
  <li>Avaliar o desempenho de estratégias de controle em um aeropêndulo;</li>
  <li>Integrar hardware embarcado, eletrônica de potência e algoritmos inteligentes em uma única solução.</li>
</ul>

<hr>

<h2>🛠️ Tecnologias Utilizadas</h2>

<ul>
  <li>ESP32</li>
  <li>Arduino Framework</li>
  <li>Lógica Fuzzy Mamdani</li>
  <li>VESC UART</li>
  <li>Impressão 3D</li>
  <li>Controle Embarcado</li>
</ul>

<hr>

<h2>📚 Contexto Acadêmico</h2>

<p>
Este trabalho foi desenvolvido como projeto final da disciplina de <strong>Controle Inteligente</strong> do curso de
Engenharia de Controle e Automação do <strong>Instituto Federal do Espírito Santo (IFES) - Campus Linhares</strong>,
servindo como plataforma de estudos em Sistemas Embarcados, Controle Inteligente e Lógica Fuzzy aplicada ao controle
de sistemas dinâmicos.
</p>
```
