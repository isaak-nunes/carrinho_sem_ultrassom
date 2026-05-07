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
#define INPUT_PIN       1        // PTA1, vulgo, porta do sensor vermalho
#define INPUT_PIN2      2        // PTA2, vulgo, porta do sensor verde

#define r 2
#define tpm 1000 * r             // Define a frequência do PWM fpwm = (TPM_CLK / (tpm * PS))

#define max tpm * 60 / 100       // Velocidade máxima proporcional ao tpm
#define min tpm * 30 / 100        // Velocidade mínima proporcionao ao tpm

#define paro 26

volatile uint16_t gus= 0;
volatile uint16_t tavo= 0;

void mtr_Vrd_f(int v)       //função pro motor verde ir pra frente      
{   
    pwm_tpm_CnV(TPM0, 2, max + 60 + v);     //Quanto MAIOR, mais rápido
    pwm_tpm_CnV(TPM0, 0, tpm);          //Quanto MAIOR, menos rápido
}

void mtr_Vrd_t(void)        //função pro motor verde ir pra trás        
{
    pwm_tpm_CnV(TPM0, 2, 0);
    pwm_tpm_CnV(TPM0, 0, min);
}

void mtr_Vrm_f(int v)       //função pro motor vermelho ir pra frente   
{
    pwm_tpm_CnV(TPM0, 3, max + v);      //Quanto MAIOR, mais rápido
    pwm_tpm_CnV(TPM0, 4, tpm);      //Quanto MAIOR, menos rápido
}

void mtr_Vrm_t(void)        //função pro motor vermelho ir pra trás     
{
    pwm_tpm_CnV(TPM0, 3, 0);
    pwm_tpm_CnV(TPM0, 4, min);
}

void quierio(void)          // quierio                                  
{
    pwm_tpm_CnV(TPM0, 2, 0);
    pwm_tpm_CnV(TPM0, 0, tpm);
    pwm_tpm_CnV(TPM0, 3, 0);
    pwm_tpm_CnV(TPM0, 4, tpm);
}

void setup(void) { //pois quanto menor o main, melhor!
    sensorUltrassonicoInit();

    //iniciação do clock da porta (eu acho)
    pwm_tpm_Init(TPM0, TPM_PLLFLL, tpm, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM2, TPM_PLLFLL, tpm, TPM_CLK, PS_128, EDGE_PWM);

        pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18); //led vermelho
        pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19); //led verde

        //motores pra frente
        pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOD, 2);  //led vermelho
        pwm_tpm_Ch_Init(TPM0, 3, TPM_PWM_H, GPIOD, 3);  //led verde

        //motores pra trás
        pwm_tpm_Ch_Init(TPM0, 0, TPM_PWM_L, GPIOD, 0);  //led vermelho
        pwm_tpm_Ch_Init(TPM0, 4, TPM_PWM_L, GPIOD, 4);  //led verde
}
int main (void)
{
    setup();
    //isso aqui faz algo importante 
    const struct device *input_dev;
    input_dev = DEVICE_DT_GET(INPUT_PORT);

    //habilita o pino como input, dos sensores infravermelho e do ultrassom
    gpio_pin_configure(input_dev, INPUT_PIN, GPIO_INPUT);
    gpio_pin_configure(input_dev, INPUT_PIN2, GPIO_INPUT);
  
    int yu = tpm - max;
    while (1) {
        gus = gpio_pin_get(input_dev, INPUT_PIN);
        tavo = gpio_pin_get(input_dev, INPUT_PIN2);

        //vermelho, vulgo direita
        if (gus == 0 && tavo == 1)
        {
            pwm_tpm_CnV(TPM2, 0, 0);        //led vermelho, ligado
            pwm_tpm_CnV(TPM2, 1, tpm);      //led verde, desligado

            mtr_Vrd_f(0);
            mtr_Vrm_t();
        }

        //verde, vulgo esquerda
        if (gus == 1 && tavo == 0)
        {
            pwm_tpm_CnV(TPM2, 0, tpm);    //led vermelho, desligado
            pwm_tpm_CnV(TPM2, 1, 0);      //led verde, ligado

            mtr_Vrd_t();
            mtr_Vrm_f(0);
        }

        //frente
        if (gus == tavo)
        {
            pwm_tpm_CnV(TPM2, 0, tpm);       //led vermelho, ligado
            pwm_tpm_CnV(TPM2, 1, tpm);       //led verde, ligado

            mtr_Vrd_f(yu);
            mtr_Vrm_f(yu);
        }
        k_msleep(20);
        float aux = distancia;
        while(aux < paro){
            aux = (aux * 2 + distancia)/3;
            quierio();
        }
    }
}