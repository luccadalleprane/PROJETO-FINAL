/*
   Name:        Aeropendulo_Controle_Fuzzy_PCNT.ino
   Description: Integração do controle do VESC via UART + Encoder PCNT usando Lógica Fuzzy.
                Permite alterar I_Bias, Setpoint e acionar PARADA DE EMERGÊNCIA (E).
*/

#include <Arduino.h>
#include <VescUart.h>
#include "driver/pulse_cnt.h"

// ==========================================
// CONFIGURAÇÕES DO VESC (UART)
// ==========================================
HardwareSerial VescSerial(2); 
VescUart UART;

#define RX_PIN 16
#define TX_PIN 17

float current = 0.0; 

// ==========================================
// CONFIGURAÇÕES DO ENCODER (PCNT - v2 do ESP32)
// ==========================================
#define ENCODER_A 18
#define ENCODER_B 19

const float COUNTS_PER_REV = 600.0;

pcnt_unit_handle_t pcnt_unit = NULL;
int16_t raw_count = 0;
int64_t acc_count = 0;
int64_t last_acc_count = 999999;

float graus = 0.0f;
float radianos = 0.0f;

// ==========================================
// VARIÁVEIS DO CONTROLADOR FUZZY (DINÂMICAS)
// ==========================================
float setpoint = 1.57f;      // Alvo inicial em radianos (~45 graus)
float i_bias = 3.4f;           // Ponto de operação (corrente mínima para flutuar no alvo)

float last_erro = 0.0f;        // Armazena o erro anterior para o cálculo da derivada
unsigned long last_time = 0; 

const float SAT_MAX = 5.0f;    // Limite superior de corrente para o VESC
const float SAT_MIN = 0.1f;    // Limite inferior (não zera para manter empuxo e estabilidade)

// Flag de Segurança (Bloqueio do Motor)
bool parada_emergencia = false;

// ==========================================
// FUNÇÕES AUXILIARES DE PERTINÊNCIA FUZZY
// ==========================================
float trimf(float x, float a, float b, float c) {
    if (x <= a || x >= c) return 0.0f;
    if (x == b) return 1.0f;
    if (x > a && x < b) return (x - a) / (b - a);
    if (x > b && x < c) return (c - x) / (c - b);
    return 0.0f;
}

float trap_left(float x, float a, float b) {
    if (x <= a) return 1.0f;
    if (x >= b) return 0.0f;
    return (b - x) / (b - a);
}

float trap_right(float x, float a, float b) {
    if (x <= a) return 0.0f;
    if (x >= b) return 1.0f;
    return (x - a) / (b - a);
}

// ==========================================
// MOTOR DE INFERÊNCIA FUZZY (MAMDANI)
// ==========================================
float calcularFuzzyDeltaI(float e, float ve) {
    // 1. Fuzzificação do Erro (Alvo - Atual)
    float e_N = trap_left(e, -1.57f, 0.0f);
    float e_Z = trimf(e, -0.2f, 0.0f, 0.2f);
    float e_P = trap_right(e, 0.0f, 1.57f);

    // 2. Fuzzificação da Variação do Erro (Velocidade)
    float ve_N = trap_left(ve, -4.0f, 0.0f);
    float ve_Z = trimf(ve, -0.5f, 0.0f, 0.5f);
    float ve_P = trap_right(ve, 0.0f, 4.0f);

    // Singletons de saída (Delta I mapeado de forma assimétrica)
    const float GN = -0.6f; // Reduz consideravelmente, mas sem cortar o motor todo
    const float MN = -0.2f; // Redução suave
    const float Z  =  0.0f;
    const float MP =  0.8f;
    const float GP =  2.5f;

    float somaPesos = 0.0f;
    float somaNumerador = 0.0f;
    float r;

    // 3. Avaliação das Regras (Operador AND via Mínimo)
    // Erro Negativo (Aeropêndulo subiu demais)
    r = min(e_N, ve_N); somaNumerador += r * GN; somaPesos += r; // Regra 1
    r = min(e_N, ve_Z); somaNumerador += r * MN; somaPesos += r; // Regra 2
    r = min(e_N, ve_P); somaNumerador += r * Z;  somaPesos += r; // Regra 3

    // Erro Zero (Aeropêndulo no alvo)
    r = min(e_Z, ve_N); somaNumerador += r * MN; somaPesos += r; // Regra 4
    r = min(e_Z, ve_Z); somaNumerador += r * Z;  somaPesos += r; // Regra 5 (Mantém o Bias)
    r = min(e_Z, ve_P); somaNumerador += r * MP; somaPesos += r; // Regra 6

    // Erro Positivo (Aeropêndulo caindo / abaixo do alvo)
    r = min(e_P, ve_N); somaNumerador += r * Z;  somaPesos += r; // Regra 7
    r = min(e_P, ve_Z); somaNumerador += r * MP; somaPesos += r; // Regra 8
    r = min(e_P, ve_P); somaNumerador += r * GP; somaPesos += r; // Regra 9

    // 4. Desfuzzificação por Média Ponderada
    if (somaPesos > 0.0001f) {
        return somaNumerador / somaPesos;
    }
    return 0.0f;
}

// ==========================================
// FUNÇÃO DE CONFIGURAÇÃO DO ENCODER
// ==========================================
pcnt_unit_handle_t configurarEncoder(int pinA, int pinB) {
    pcnt_unit_config_t unit_config = {
        .low_limit = -30000,
        .high_limit = 30000,
    };
    pcnt_unit_handle_t unit = NULL;
    pcnt_new_unit(&unit_config, &unit);

    pcnt_glitch_filter_config_t filter_config = { .max_glitch_ns = 1000 };
    pcnt_unit_set_glitch_filter(unit, &filter_config);

    pcnt_chan_config_t chan_a_config = { .edge_gpio_num = pinA, .level_gpio_num = pinB };
    pcnt_channel_handle_t chan_a = NULL;
    pcnt_new_channel(unit, &chan_a_config, &chan_a);

    pcnt_chan_config_t chan_b_config = { .edge_gpio_num = pinB, .level_gpio_num = pinA };
    pcnt_channel_handle_t chan_b = NULL;
    pcnt_new_channel(unit, &chan_b_config, &chan_b);

    pcnt_channel_set_edge_action(chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    pcnt_channel_set_level_action(chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    pcnt_channel_set_edge_action(chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    pcnt_channel_set_level_action(chan_b, PCNT_CHANNEL_LEVEL_ACTION_INVERSE, PCNT_CHANNEL_LEVEL_ACTION_KEEP);

    pcnt_unit_enable(unit);
    pcnt_unit_clear_count(unit);
    pcnt_unit_start(unit);

    return unit;
}

void lerEncoderHardware() {
    int val = 0;
    pcnt_unit_get_count(pcnt_unit, &val);
    
    acc_count += (int16_t)(val - raw_count); 
    raw_count = (int16_t)val;
}

// ==========================================
// SETUP
// ==========================================
void setup() {
    Serial.begin(115200); 
    
    VescSerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN); 
    while (!VescSerial) { ; }
    UART.setSerialPort(&VescSerial);

    pcnt_unit = configurarEncoder(ENCODER_A, ENCODER_B);
    
    Serial.println("\n--- SISTEMA AEROPÊNDULO FUZZY INICIALIZADO ---");
    Serial.println("Comandos aceitos no Monitor Serial:");
    Serial.println("  B[valor] -> Define Corrente de Bias (Ex: B1.8)");
    Serial.println("  S[valor] -> Define Setpoint em radianos (Ex: S0.788)");
    Serial.println("  E        -> [EMERGÊNCIA] Desliga o motor imediatamente!");
    Serial.println("  R        -> [RESUME] Libera o motor e reinicia o controle.");
    Serial.println("----------------------------------------\n");
    
    last_time = millis(); 
}

// ==========================================
// LOOP PRINCIPAL
// ==========================================
void loop() {
    // 1. LEITURA E CONVERSÃO DO ENCODER (Escala Real de 360° e 2*PI)
    lerEncoderHardware();

    // Multiplica por 4.0f devido à decodificação por quadratura completa do PCNT
    graus = (360.0f * (float)acc_count) / (COUNTS_PER_REV * 4.0f);
    radianos = (2.0f * PI * (float)acc_count) / (COUNTS_PER_REV * 4.0f);
    if (acc_count != last_acc_count) {
        Serial.print("Angulo: ");
        Serial.print(graus, 2);
        Serial.print("° | Rad: ");
        Serial.print(radianos, 4);
        Serial.print(" rad | I_Bias: ");
        Serial.print(i_bias, 2);
        
        if (parada_emergencia) {
            Serial.println(" | [EMERGÊNCIA ATIVA - MOTOR BLOQUEADO]");
        } else {
            Serial.print(" | Setpoint: ");
            Serial.print(setpoint, 4);
            Serial.print(" | I_Current: ");
            Serial.print(current);
            Serial.println(" A");
        }

        last_acc_count = acc_count;
    }

    // 2. PARSER DO MONITOR SERIAL (Sintonização Dinâmica e Segurança)
    if (Serial.available() > 0) {
        String inputString = Serial.readStringUntil('\n');
        inputString.trim(); 
        
        if (inputString.length() > 0) {
            char comando = inputString.charAt(0);          
            String valorString = inputString.substring(1); 
            float valor = valorString.toFloat();
            
            // COMANDO DE EMERGÊNCIA
            if (comando == 'E' || comando == 'e') {
                parada_emergencia = true;
                current = 0.0f;
                Serial.println("\n################################################");
                Serial.println("[PERIGO] PARADA DE EMERGÊNCIA SOLICITADA!");
                Serial.println("Motor cortado para 0A. Controle Fuzzy desativado.");
                Serial.println("Para liberar o sistema, digite 'R' e pressione Enter.");
                Serial.println("################################################\n");
            } 
            // COMANDO PARA REASSUMIR O SISTEMA
            else if (comando == 'R' || comando == 'r') {
                parada_emergencia = false;
                last_time = millis(); 
                Serial.println("\n>>> [SISTEMA LIBERADO] Reiniciando controle Fuzzy... <<<\n");
            }
            // ALTERAÇÃO DE PARÂMETROS
            else if (!parada_emergencia) {
                if (comando == 'B' || comando == 'b') {
                    i_bias = valor;
                    Serial.print("\n>>> [CONFIG] Nova Corrente de Bias definida: ");
                    Serial.println(i_bias, 2);
                } 
                else if (comando == 'S' || comando == 's') {
                    setpoint = valor;
                    Serial.print("\n>>> [CONFIG] Novo Setpoint definido: ");
                    Serial.print(setpoint, 4);
                    Serial.println(" rad\n");
                } 
                else {
                    Serial.println("\n[ERRO] Comando desconhecido. Use B, S, E ou R.");
                }
            } else {
                Serial.println("\n[AVISO] Sistema em modo de EMERGÊNCIA. Comandos B e S bloqueados. Digite 'R' para liberar.");
            }
        }
    }

    // ==========================================
    // MALHA DE CONTROLE FUZZY
// ==========================================
    unsigned long current_time = millis();
    float dt = (current_time - last_time) / 1000.0f;

    // Garante a execução da malha de controle a uma taxa estável de amostragem (~20ms = 50Hz)
    if (dt >= 0.02f) {
        last_time = current_time;

        if (parada_emergencia) {
            current = 0.0f;
        } 
        else {
            // 1. Calcula o Erro atual e a variação do erro (Velocidade)
            float erro = setpoint - radianos;
            float variacao_erro = (erro - last_erro) / dt;
            last_erro = erro;

            // 2. Processa o Motor Fuzzy para obter o Delta I
            float delta_i = calcularFuzzyDeltaI(erro, variacao_erro);

            // 3. Aplica o Bias (Ponto de flutuação)
            float u_ideal = i_bias + delta_i;

            // 4. Saturação Física Antiqueda / Antiqueima
            if (u_ideal > SAT_MAX) {
                current = SAT_MAX;
            } else if (u_ideal < SAT_MIN) {
                current = SAT_MIN; // Mantém a hélice girando minimamente para o braço não desabar
            } else {
                current = u_ideal;
            }
        }

        // ==========================================
        // 3. ENVIO DE COMANDO PARA O VESC
        // ==========================================
        UART.setCurrent(current);
    }
    
    delay(1); 
}