#include <zephyr/kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <zephyr/device.h>             // API para obter e utilizar dispositivos do sistema
#include <zephyr/drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <pwm_z42.h>                // Biblioteca personalizada com funções de controle do TPM (Timer/PWM Module)

#define TPM_IRQ_LINE TPM1_IRQn  // relaciona a interrupção ao timer TPM1
#define TPM_IRQ_PRIORITY 1      // define a prioridade da interrupção

extern volatile uint16_t tick_subida;
extern volatile uint16_t tick_descida;
extern volatile uint16_t tick;
extern volatile float t; // tempo que o pulso ficou ativo em microssegundos
extern volatile float distancia; // distanciaancia em cm

__ramfunc void tpm1_isr(void *arg);
void sensorUltrassonicoInit(void);