//versão 4.2
//:)

#include <zephyr/kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <zephyr/device.h>             // API para obter e utilizar dispositivos do sistema
#include <zephyr/drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <pwm_z42.h>   

#define INPUT_PORT      DT_NODELABEL(gpioa)  // Porta E = GPIO_4 no seu .dts
#define INPUT_PIN       1         // PTE20
#define INPUT_PORT2     DT_NODELABEL(gpioa)  // Porta E = GPIO_4 no seu .dts
#define INPUT_PIN2      2         // PTE20
#define max 500
#define min 200
#define start 1000

#define TPM_MODULE 1000         // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))

void main (void)
{
    const struct device *input_dev;
    const struct device *input_dev2;
    
    int ret, val, ret2, val2;
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM0, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    input_dev = DEVICE_DT_GET(INPUT_PORT);
    input_dev2 = DEVICE_DT_GET(INPUT_PORT2);

    ret = gpio_pin_configure(input_dev, INPUT_PIN, GPIO_INPUT);
    ret2 = gpio_pin_configure(input_dev2, INPUT_PIN2, GPIO_INPUT);

        pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18);
        pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19);
        pwm_tpm_Ch_Init(TPM0, 1, TPM_PWM_H, GPIOD, 1);

        pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOD, 2);
        pwm_tpm_Ch_Init(TPM0, 3, TPM_PWM_H, GPIOD, 3);

        pwm_tpm_CnV(TPM0, 1, 0);
        pwm_tpm_CnV(TPM0, 2, start);
        pwm_tpm_CnV(TPM0, 3, start);
        k_msleep(500);

    while (1) {
        pwm_tpm_CnV(TPM0, 1, 1000);
        val = gpio_pin_get(input_dev, INPUT_PIN);
        val2 = gpio_pin_get(input_dev2, INPUT_PIN2);

        //vermelho
        {if (val == 0)
        {
            pwm_tpm_CnV(TPM2, 0, 0);        //led
            pwm_tpm_CnV(TPM0, 2, max);
        }
        else
        {
            pwm_tpm_CnV(TPM0, 1, 1000);
            pwm_tpm_CnV(TPM2, 0, 1000);
            pwm_tpm_CnV(TPM0, 2, min);
        }
    }
        
        //verde
        {if (val2 != 1)
        {
            pwm_tpm_CnV(TPM2, 1, 0);        //led
            pwm_tpm_CnV(TPM0, 3, max);
        }
        else
        {
            pwm_tpm_CnV(TPM2, 1, 1000);
            pwm_tpm_CnV(TPM0, 3, min);
        }
    }
    k_msleep(30);
    }
}