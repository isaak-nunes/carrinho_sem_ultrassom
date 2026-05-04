//sem-ultrassom

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

#include <math.h>                      //livraria de operações matematicas padrão
#include <pwm_z42.h>                   //biblioteca do pulse width module
#include <sensor-ultrassonico.h>       //biblioteca do ultrassom 

#define INPUT_PORT      DT_NODELABEL(gpioa)  // Porta A = GPIO_0 no seu .dts
#define INPUT_PIN       1         // PTA1, vulgo, porta do sensor vermalho
#define INPUT_PIN2      2         // PTA2, vulgo, porta do sensor verde

#define TPM_MODULE 2000                 // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))

#define max TPM_MODULE * 80 / 100       // Velocidade máxima proporcional ao TPM_MODULE
#define min TPM_MODULE - max    // Velocidade mínima proporcionao ao TPM_MODULE

void mtr_Vrd_f(void)    //função pro motor verde ir pra frente      
{   
    pwm_tpm_CnV(TPM0, 2, max + 75);
    pwm_tpm_CnV(TPM0, 0, max);
}

void mtr_Vrd_t(int v)    //função pro motor verde ir pra trás        
{
        pwm_tpm_CnV(TPM0, 2, 0);
        pwm_tpm_CnV(TPM0, 0, min - v);
}

void mtr_Vrm_f(void)    //função pro motor vermelho ir pra frente   
{
        pwm_tpm_CnV(TPM0, 3, max);
        pwm_tpm_CnV(TPM0, 4, max);
}

void mtr_Vrm_t(int v)    //função pro motor vermelho ir pra trás     
{
        pwm_tpm_CnV(TPM0, 3, 0);
        pwm_tpm_CnV(TPM0, 4, min - v);
}

void quierio(void){
    pwm_tpm_CnV(TPM0, 2, 0);
    pwm_tpm_CnV(TPM0, 0, TPM_MODULE);
    pwm_tpm_CnV(TPM0, 3, 0);
    pwm_tpm_CnV(TPM0, 4, TPM_MODULE);
    printk("%.f\n",distancia);
}

int main (void)
{
    sensorUltrassonicoInit();
    //isso aqui faz algo importante 
    const struct device *input_dev;
    input_dev = DEVICE_DT_GET(INPUT_PORT);
    int gus, tavo;

    //iniciação do clock da porta (eu acho)
    pwm_tpm_Init(TPM0, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

        //habilita o pino como input, dos sensores infravermelho e do ultrassom
        gpio_pin_configure(input_dev, INPUT_PIN, GPIO_INPUT);
        gpio_pin_configure(input_dev, INPUT_PIN2, GPIO_INPUT);

        pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18); //led vermelho
        pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19); //led verde

        //motores pra frente
        pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOD, 2);
        pwm_tpm_Ch_Init(TPM0, 3, TPM_PWM_H, GPIOD, 3); 

        //motores pra trás
        pwm_tpm_Ch_Init(TPM0, 0, TPM_PWM_L, GPIOD, 0);  
        pwm_tpm_Ch_Init(TPM0, 4, TPM_PWM_L, GPIOD, 4);  

    float redher = distancia;
    while (1) {
        redher = (redher * 2 + distancia)/3;
        gus = gpio_pin_get(input_dev, INPUT_PIN);
        tavo = gpio_pin_get(input_dev, INPUT_PIN2);

        //vermelho, vulgo direita
        if (gus == 0)
        {
            pwm_tpm_CnV(TPM2, 0, 0);        //led

            mtr_Vrd_f();

        }
        else
        {
            pwm_tpm_CnV(TPM2, 0, TPM_MODULE);

            /*esse if existe para que caso ambas
            as variaveis resultem em um, o carrinho
            contiue a seguir em frente*/
            if (tavo != 1)
            {
                mtr_Vrd_t(30);
            }
            else
            {
                mtr_Vrd_f();
            }
        }
    
        
        //verde, vulgo esquerda
        if (tavo == 0)
        {
            pwm_tpm_CnV(TPM2, 1, 0);        //led

            mtr_Vrm_f();
        }
        else
        {
            pwm_tpm_CnV(TPM2, 1, TPM_MODULE);
            
            /*esse if existe para que caso ambas
            as variaveis resultem em um, o carrinho
            contiue a seguir em frente*/
            if (gus != 1)
            {
                mtr_Vrm_t(30);
            }
            else
            {
                mtr_Vrm_f();
            }
        }
   
        while(floor(redher) < 22){
            redher = (redher * 2 + distancia)/3;
            quierio();
        }
    }
    k_msleep(20);
}