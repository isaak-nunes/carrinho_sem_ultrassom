#include <zephyr/kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <zephyr/device.h>             // API para obter e utilizar dispositivos do sistema
#include <zephyr/drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <pwm_z42.h>                // Biblioteca personalizada com funções de controle do TPM (Timer/PWM Module)

#define SLEEP_TIME_MS 500

//34464 é o valor maximo
#define TPM_MODULE 1000         // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))
// Valores de duty cycle correspondentes a diferentes larguras de pulso
uint16_t duty_50  = TPM_MODULE/2;       // 50% de duty cycle (meio brilho)
uint16_t duty_0  = TPM_MODULE;
uint16_t duty_100  = 0;


enum estado_led {
    VERMELHO,
    AMARELO,
    AZUL
};

void cor_led(enum estado_led state) {

    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18);
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19);
    pwm_tpm_Ch_Init(TPM0, 1, TPM_PWM_H, GPIOD, 1);
    pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOD, 2);
     pwm_tpm_Ch_Init(TPM0, 3, TPM_PWM_H, GPIOD, 3);

    switch (state) {
        case VERMELHO:
            pwm_tpm_CnV(TPM2, 0, duty_100);
            pwm_tpm_CnV(TPM2, 1, duty_0);
            pwm_tpm_CnV(TPM0, 1, duty_0);
            pwm_tpm_CnV(TPM0, 2, duty_100);
            pwm_tpm_CnV(TPM0, 3, duty_100);
            break;
        case AMARELO:
            pwm_tpm_CnV(TPM2, 0, duty_100);
            pwm_tpm_CnV(TPM2, 1, duty_50);
            pwm_tpm_CnV(TPM0, 1, duty_0);
            pwm_tpm_CnV(TPM0, 2, duty_50);
            pwm_tpm_CnV(TPM0, 3, duty_50);
            break;
        case AZUL:
            pwm_tpm_CnV(TPM2, 0, duty_0);
            pwm_tpm_CnV(TPM2, 1, duty_0);
            pwm_tpm_CnV(TPM0, 1, duty_100);
            pwm_tpm_CnV(TPM0, 2, duty_0);
            pwm_tpm_CnV(TPM0, 3, duty_0);
            break;
    }
}

void pisca(int tpisca){
		for (int i = 0; i < 3; i++) {
		k_msleep(tpisca);
		 pwm_tpm_CnV(TPM2, 0, duty_100);
         pwm_tpm_CnV(TPM2, 1, duty_100);
        k_msleep(tpisca);
		 pwm_tpm_CnV(TPM2, 0, duty_0);
         pwm_tpm_CnV(TPM2, 1, duty_0);
		}

}


int main(void) {
        pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
        pwm_tpm_Init(TPM0, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    enum estado_led current_state = AZUL; // Initial state

    while (1) { 
        cor_led(current_state);

        switch (current_state) {
            case VERMELHO:
                k_sleep(K_SECONDS(5));
                current_state = AZUL;
                break;

            case AZUL:
                k_sleep(K_SECONDS(7)); 
                current_state = AMARELO; 
                break;

            case AMARELO:
                k_sleep(K_SECONDS(3)); 
                pisca(300);
                k_msleep(30);
                current_state = VERMELHO;
                break;
        }
    }
}