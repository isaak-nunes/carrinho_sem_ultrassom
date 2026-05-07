#include "sensor-ultrassonico.h"

volatile uint16_t tick_subida = 0;
volatile uint16_t tick_descida = 0;
volatile uint16_t tick = 0;
volatile float distancia = 0; // distanciaancia em cm

#define clock 65535
#define TPM_MODULE 2000
#define TPM_MOD TPM_MODULE/2
#define INPUT_PORT DT_NODELABEL(gpiob)
#define INPUT_PIN 0

__ramfunc void tpm1_isr(void *arg) {
    TPM1->STATUS |= TPM_STATUS_CH0F_MASK; // zerra a flag que gerou a interrupção
    if ((PTB->PDIR & 1) == 1) {
        tick_subida = TPM1->CONTROLS[0].CnV;
        return;
    }
    else {
        tick_descida = TPM1->CONTROLS[0].CnV;
        if (tick_descida > tick_subida) {
            tick = tick_descida - tick_subida;
            distancia = tick * 0.0057f;
        }
        else {
            tick = clock - (tick_subida - tick_descida);
            distancia = tick * 0.0057f;
        }
    }
}

void sensorUltrassonicoInit(void) {
    // Configura a porta B0 como input
    const struct device *input_dev;
    input_dev = DEVICE_DT_GET(INPUT_PORT);
    gpio_pin_configure(input_dev, INPUT_PIN, GPIO_INPUT);

    // Conecta a interrupção via Zephyr
    IRQ_CONNECT(TPM_IRQ_LINE, TPM_IRQ_PRIORITY, tpm1_isr, NULL, 0);
    irq_enable(TPM_IRQ_LINE);

    // Inicializa TPM1 e TPM0 com módulo e prescaler desejado
    pwm_tpm_Init(TPM1, TPM_PLLFLL, clock, TPM_CLK, PS_16, EDGE_PWM);
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_1, EDGE_PWM);

    // Configura TPM1_CH0 como input capture na borda de subida
    pwm_tpm_Ch_Init(TPM1, 0, TPM_INPUT_CAPTURE_BOTH | TPM_CHANNEL_INTERRUPT, GPIOB, 0);
    pwm_tpm_Ch_Init(TPM1, 1, TPM_PWM_H, GPIOB, 1);
    pwm_tpm_CnV(TPM1, 1, TPM_MOD);
}