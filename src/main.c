/*versão 4.2
versão do carrinho que:
-caso ambos os sensores deem BRANCO, ele siga em frente e o led liga amarelo
-caso um dos sensores deem preto, ele curve para um lado e o led liga,
 ou vermelho (com curva pra direita), ou verde (com curva pra esquerda)
-caso ambos os sensores deem preto, ele siga em frente e o led NÃO liga
*/
//:)

#include <zephyr/kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <zephyr/device.h>             // API para obter e utilizar dispositivos do sistema
#include <zephyr/drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <pwm_z42.h>   

#define INPUT_IF      DT_NODELABEL(gpioa)  // Porta A = GPIO_0 no seu .dts
#define INPUT_US      DT_NODELABEL(gpiod)  // Porta D = GPIO_3 no seu .dts
#define INPUT_PIN1      1         // input do infravermelho no pino PTA1
#define INPUT_PIN2      2         // input do infravermelho no pino PTA2
#define INPUT_PIN0      0         // input do ultrassom no pino PTD0
#define max 875
#define min 200

#define TPM_MODULE 1000         // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))

void mtr_Vrd_f(void)
{   
    //função pro motor verde ir pra frente
    pwm_tpm_CnV(TPM0, 2, max);
    pwm_tpm_CnV(TPM1, 0, max);
}

//função pro motor verde ir pra trás
void mtr_Vrd_t(void)             
{
        pwm_tpm_CnV(TPM0, 2, 0);
        pwm_tpm_CnV(TPM1, 0, min);
}

void mtr_Vrm_f(void)             //função pro motor vermelho ir pra frente
{
        pwm_tpm_CnV(TPM0, 3, max);
        pwm_tpm_CnV(TPM1, 1, max);
}

void mtr_Vrm_t(void)             //função pro motor vermelho ir pra trás
{
        pwm_tpm_CnV(TPM0, 3, 0);
        pwm_tpm_CnV(TPM1, 1, min);
}

int main (void)
{
    //isso aqui faz algo importante 
    const struct device *input_dev, *input_dev2;
    input_dev = DEVICE_DT_GET(INPUT_IF);
    input_dev2 = DEVICE_DT_GET(INPUT_US);
    int infra_vrm, val, infra_vrd, val2, us, val3;

    //iniciação do clock da porta (eu acho)
    pwm_tpm_Init(TPM0, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM1, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

        //habilita o pino como input, dos sensores infravermelho e do ultrassom
        infra_vrm = gpio_pin_configure(input_dev, INPUT_PIN1, GPIO_INPUT);
        infra_vrd = gpio_pin_configure(input_dev, INPUT_PIN2, GPIO_INPUT);
        us = gpio_pin_configure(input_dev2, INPUT_PIN0, GPIO_INPUT);

        pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18); //led vermelho
        pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19); //led verde

        //motores pra frente
        pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOD, 2);
        pwm_tpm_Ch_Init(TPM0, 3, TPM_PWM_H, GPIOD, 3); 

        //motores pra trás
        pwm_tpm_Ch_Init(TPM1, 0, TPM_PWM_L, GPIOA, 12);  
        pwm_tpm_Ch_Init(TPM1, 1, TPM_PWM_L, GPIOA, 13);  

    while (1) {
        val = gpio_pin_get(input_dev, INPUT_PIN1);
        val2 = gpio_pin_get(input_dev2, INPUT_PIN0);
        val3 = gpio_pin_get(input_dev2, INPUT_PIN0);

        //vermelho, vulgo direita
        if (val == 0)
        {
            pwm_tpm_CnV(TPM2, 0, 0);        //led

            mtr_Vrd_f();

        }
        else
        {
            pwm_tpm_CnV(TPM2, 0, 1000);

            /*esse if existe para que caso ambas
            as variaveis resultem em um, o carrinho
            contiue a seguir em frente*/
            if (val2 != 1)
            {
                mtr_Vrd_t();
            }
            else
            {
                 mtr_Vrd_f();
            }
        }
    
        
        //verde, vulgo esquerda
        if (val2 == 0)
        {
            pwm_tpm_CnV(TPM2, 1, 0);        //led

            mtr_Vrm_f();
        }
        else
        {
            pwm_tpm_CnV(TPM2, 1, 1000);
            
            /*esse if existe para que caso ambas
            as variaveis resultem em um, o carrinho
            contiue a seguir em frente*/
            if (val != 1)
            {
                mtr_Vrm_t();
            }
            else{
                 mtr_Vrm_f();
            }
        }
   
    }
    k_msleep(20);
}