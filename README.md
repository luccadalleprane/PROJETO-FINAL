# Controle de Posição de um Aeropêndulo com Lógica Fuzzy

**Projeto Final da disciplina de Controle Inteligente**

**Instituto Federal do Espírito Santo (IFES) – Campus Linhares**

![IFES - Linhares](image.png)

## 📖 Sobre o Projeto

Este projeto apresenta o desenvolvimento de um sistema de controle de posição angular para um aeropêndulo utilizando Lógica Fuzzy do tipo Mamdani.

O aeropêndulo consiste em uma haste articulada acionada por um conjunto motor-hélice, caracterizado por uma dinâmica não linear e naturalmente instável. O objetivo do controlador é estabilizar a posição angular da haste e rejeitar perturbações externas, mantendo o sistema próximo ao ponto de operação desejado.

---

## 🔧 Hardware Utilizado

* ESP32
* Motor Brushless DC (BLDC)
* VESC (Vedder Electronic Speed Controller)
* Encoder incremental para medição da posição angular
* Bateria LiPo 11,1 V
* Estrutura mecânica composta por peças impressas em 3D e perfis metálicos

---

## 🧠 Estratégia de Controle

O sistema utiliza um controlador Fuzzy Mamdani com duas variáveis de entrada:

* Erro angular
* Velocidade do erro

A saída do controlador corresponde a uma correção de corrente aplicada ao motor. Além disso, foi implementada uma corrente de equilíbrio (*bias current*), responsável por manter o aeropêndulo próximo ao ponto de operação, enquanto o controlador Fuzzy realiza a compensação de perturbações e corrige desvios de posição.

A sintonia das funções de pertinência, dos singletons e da base de regras foi realizada de forma empírica e experimental, por meio de sucessivos ensaios no protótipo físico. Os parâmetros foram ajustados com base na observação da resposta dinâmica do sistema, buscando um compromisso entre estabilidade, rapidez de resposta e rejeição de perturbações.

---
\begin{figure}[htbp]
    \centering
    \begin{tikzpicture}[
        >=latex, 
        scale=0.85, every node/.style={transform shape},
        font=\sffamily\small,
        % Estilização dos Blocos (Largura padronizada para alinhamento vertical perfeito)
        block/.style={draw, thick, fill=blue!5, rectangle, minimum height=1.1cm, minimum width=3.8cm, align=center, rounded corners=2pt},
        fuzzy/.style={draw, thick, fill=green!5, rectangle, minimum height=1.1cm, minimum width=3.8cm, align=center, rounded corners=2pt},
        safety/.style={draw, thick, fill=red!5, rectangle, minimum height=1.1cm, minimum width=3.8cm, align=center, rounded corners=2pt},
        plant/.style={draw, thick, fill=orange!5, rectangle, minimum height=1.1cm, minimum width=3.8cm, align=center, rounded corners=2pt},
        sum/.style={draw, circle, thick, minimum size=6mm, inner sep=0pt}
    ]

    % ==========================================
    % EIXO CENTRAL (Fluxo Principal Top-Down, x=0)
    % ==========================================
    \node (setpoint) at (0, 0) [align=center] {Setpoint ($S$)\\ \footnotesize(radianos)};
    \node [sum] (sum_e) at (0, -1.8) {$\Sigma$};
    \node [block] (error_calc) at (0, -3.6) {Cálculo Dinâmico\\ \footnotesize($dt \ge 20\text{ms}$)\\ $e(t)$ e $ve(t)$};
    \node [fuzzy] (fuzz) at (0, -5.4) {\textbf{Fuzzificação}\\ \footnotesize Erro (Trap/Trim)\\ Variação (Trap/Trim)};
    \node [fuzzy] (infer) at (0, -7.2) {\textbf{Inferência (Mamdani)}\\ \footnotesize Mínimo (AND)\\ 9 Regras de Decisão};
    \node [fuzzy] (defuzz) at (0, -9.0) {\textbf{Desfuzzificação}\\ \footnotesize Média Ponderada\\ $\Delta I$ (Singletons)};
    \node [sum] (sum_u) at (0, -10.8) {$\Sigma$};
    \node [safety] (sat) at (0, -12.6) {Saturação Física\\ \footnotesize $SAT_{MIN} \le I \le SAT_{MAX}$\\ \footnotesize(0.1A) \hspace{0.5cm} (5.0A)};
    \node [plant] (vesc) at (0, -14.4) {VESC \footnotesize (Driver CC)\\ \footnotesize Conexão UART (TX/RX)};
    \node [plant] (aeropendulo) at (0, -16.2) {Aeropêndulo\\ \footnotesize (Física do Sistema)};
    \node [block] (pcnt) at (0, -18.0) {Hardware PCNT\\ \footnotesize Decodificação Quadratura\\ \footnotesize ESP32 (Pinos 18/19)};

    % ==========================================
    % CORREDOR LATERAL DIREITO (Comandos e Bias, x=4.8)
    % ==========================================
    \node (serial_input) at (4.8, 0) [draw, dashed, fill=gray!5, align=center, minimum width=3.2cm] {Monitor Serial\\ \footnotesize (Sintonia Online)};
    \node [block, minimum width=3.2cm] (bias) at (4.8, -10.8) {Ponto de Op.\\ $I_{Bias}$ \footnotesize (Cmd B)};


    % ==========================================
    % ROTEAMENTO DAS LINHAS
    % ==========================================
    
    % Fluxo Central (Descida Direta)
    \draw [thick, ->] (setpoint) -- node[right] {$+$} (sum_e);
    \draw [thick, ->] (sum_e) -- node[right] {$e$} (error_calc);
    \draw [thick, ->] (error_calc) -- node[right] {$e, ve$} (fuzz);
    \draw [thick, ->] (fuzz) -- (infer);
    \draw [thick, ->] (infer) -- (defuzz);
    \draw [thick, ->] (defuzz) -- node[right] {$\Delta I$} (sum_u);
    \draw [thick, ->] (sum_u) -- node[right] {$u_{ideal}$} (sat);
    \draw [thick, ->] (sat) -- node[right] {$I_{current}$} (vesc);
    \draw [thick, ->] (vesc) -- (aeropendulo);
    \draw [thick, ->] (aeropendulo) -- (pcnt);

    % Fluxos do Corredor Direito (Entrando na malha central)
    \draw [thick, ->] (bias.west) -- node[above] {$I_{Bias}$} (sum_u.east);
    \draw [dashed, ->] (serial_input.west) -- (setpoint.east);
    \draw [dashed, ->] (serial_input.south) -- (bias.north);

    % Realimentação Limpa (Corredor Lateral Esquerdo x=-4.2)
    % A linha sai do sensor, vai totalmente para a esquerda, sobe reto e entra no somador inicial
    \draw [thick, ->] (pcnt.west) -- node[above, pos=0.5] {\footnotesize radianos} (-4.2, -18.0) -- (-4.2, -1.8) -- node[above, pos=0.85] {$-$} (sum_e.west);

    \end{tikzpicture}
    \caption{Diagrama de blocos vertical da malha fechada do controlador Fuzzy do Aeropêndulo.}
    \label{fig:diagrama_fuzzy_vertical}
\end{figure}



## ⚙️ Principais Características

* Controle em malha fechada com realimentação por encoder;
* Implementação embarcada em ESP32;
* Comunicação com o VESC via UART;
* Controlador Fuzzy Mamdani baseado em regras heurísticas;
* Compensação de perturbações através de corrente de equilíbrio e ação Fuzzy;
* Aplicação em um sistema não linear e instável.

---

## 🎯 Objetivos do Projeto

* Desenvolver uma plataforma experimental para estudos de controle inteligente;
* Aplicar técnicas de Lógica Fuzzy em sistemas dinâmicos reais;
* Avaliar o desempenho de estratégias de controle em um aeropêndulo;

---

## 👨‍💻 Tecnologias Utilizadas

* ESP32
* Arduino Framework
* Lógica Fuzzy Mamdani
* VESC UART
* Impressão 3D
* Controle Embarcado

---

## 📚 Contexto Acadêmico

Este trabalho foi desenvolvido como projeto final da disciplina de **Controle Inteligente** do curso de Engenharia de Controle e Automação do **Instituto Federal do Espírito Santo (IFES) – Campus Linhares**, servindo como plataforma de estudos em Sistemas Embarcados, Controle Inteligente e Lógica Fuzzy aplicada ao controle de sistemas dinâmicos.
