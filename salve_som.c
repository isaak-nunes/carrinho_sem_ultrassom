//com_ultrassom
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
#define max 875
#define min 200


#define TPM_IRQ_LINE TPM1_IRQn  // relaciona a interrupção ao timer TPM1
#define TPM_IRQ_PRIORITY 1      // define a prioridade da interrupção

#define TPM_MODULE 1000         // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))

volatile uint16_t captured= 0; 

/*
void tpm1_isr(void *arg)
{
       TPM1->STATUS |= TPM_STATUS_CH0F_MASK; // zerra a flag que gerou a interrupção

       captured = TPM1->CONTROLS[0].CnV; // coloca o valor atual do timer na variável "captured"
}
*/

void motor(int s)
{   
    //função pro motor ir pra frente
    pwm_tpm_CnV(TPM0, 1, 1000);        //led

    pwm_tpm_CnV(TPM0, 2, max * s +105);
    pwm_tpm_CnV(TPM1, 0, max * s);
    pwm_tpm_CnV(TPM0, 3, max * s);
    pwm_tpm_CnV(TPM1, 1, max * s);
    
}


int main (void)
{
    //iniciação do clock da porta (eu acho)
    pwm_tpm_Init(TPM0, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM1, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

        pwm_tpm_Ch_Init(TPM0, 1, TPM_PWM_L, GPIOD, 1); //led azul

        // o tempo de resposta do ultrassom
        pwm_tpm_Ch_Init(TPM0, 4, TPM_PWM_H, GPIOD, 4);

        //motores pra frente
        pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOD, 2);
        pwm_tpm_Ch_Init(TPM0, 3, TPM_PWM_H, GPIOD, 3); 

        //motores pra trás
        pwm_tpm_Ch_Init(TPM1, 0, TPM_PWM_L, GPIOA, 12);  
        pwm_tpm_Ch_Init(TPM1, 1, TPM_PWM_L, GPIOA, 13);  

    while (1) {
        pwm_tpm_CnV(TPM0, 4, 100);

        //vermelho, vulgo direita
        if (0 == 0)
        {
            motor(0);
        }
        else
        {
            motor(1); 
        }
    }
    k_msleep(20);
}