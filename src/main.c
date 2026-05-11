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
#define INPUT_PIN       5        // PTA5, vulgo, porta do sensor verde (esquerda)
#define INPUT_PIN2      4        // PTA4, vulgo, porta do sensor vermalho (direita)
#define INPUT_PIN4      1        // PTA13, vulgo, porta do sensor do meio 

#define tpm 1000                    // Define a frequência do PWM fpwm = (TPM_CLK / (tpm * PS))

#define frntG   900
#define trasG   200
#define frntS   600
#define trasS   400

#define paro 0                     // quantos cm pa para


volatile uint8_t err = 0;   // variavel relacionada a possiveis falhas do ultrassom do carrinho
volatile uint8_t gus= 0;    // variavel relacionada ao sensor direito do carrinho  
volatile uint8_t tavo= 0;   // variavel relacionada ao sensor esquerdo do carrinho
volatile uint8_t reh= 0;    // variavel relacionada ao sensor central do carrinho

// variaveis para o controle PID
volatile uint8_t Kp= 35;    // peso da proporção
volatile uint8_t Ki= 0;    // peso da integral
volatile uint8_t Kd= 35;    // peso da derivada

volatile uint8_t P = 0, I = 0, D = 0, PID = 0;    // -\(^-^)/-

void mtr_Vrd_f(int v){       //função pro motor verde (esquerdo) ir pra frente      
    pwm_tpm_CnV(TPM0, 2, v);     //Quanto MAIOR, mais rápido
    pwm_tpm_CnV(TPM0, 0, tpm);          //Quanto MAIOR, menos rápido
}

void mtr_Vrd_t(int d)        //função pro motor verde (esquerdo) ir pra trás        
{
    pwm_tpm_CnV(TPM0, 2, 0);
    pwm_tpm_CnV(TPM0, 0, d);
}

void mtr_Vrm_f(int v)       //função pro motor vermelho (direito) ir pra frente   
{
    pwm_tpm_CnV(TPM0, 3, v);      //Quanto MAIOR, mais rápido
    pwm_tpm_CnV(TPM0, 4, tpm);      //Quanto MAIOR, menos rápido
}

void mtr_Vrm_t(int d)        //função pro motor vermelho (direito) ir pra trás     
{
    pwm_tpm_CnV(TPM0, 3, 0);
    pwm_tpm_CnV(TPM0, 4, d);
}

void quierio(int e)          // quierio                                  
{
    if (e == 0){
    pwm_tpm_CnV(TPM0, 2, 0);
    pwm_tpm_CnV(TPM0, 0, tpm);
    pwm_tpm_CnV(TPM0, 3, 0);
    pwm_tpm_CnV(TPM0, 4, tpm);
    }
}

void setup(void) { //pois quanto menor o main, melhor!
    sensorUltrassonicoInit();

    //iniciação do clock da porta (eu acho)
    pwm_tpm_Init(TPM0, TPM_PLLFLL, tpm, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM2, TPM_PLLFLL, tpm, TPM_CLK, PS_128, EDGE_PWM);

        //pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18); //led vermelho
        //pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19); //led verde
        pwm_tpm_Ch_Init(TPM0, 1, TPM_PWM_H, GPIOD, 1);

        //motores pra frente
        pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOD, 2);  //lado direito (vermelho)
        pwm_tpm_Ch_Init(TPM0, 3, TPM_PWM_H, GPIOD, 3);  //lado esquerdo (verde)

        //motores pra trás
        pwm_tpm_Ch_Init(TPM0, 0, TPM_PWM_L, GPIOD, 0);  //lado direito (vermelho)
        pwm_tpm_Ch_Init(TPM0, 4, TPM_PWM_L, GPIOD, 4);  //lado esquerdo (verde)
}

int main (void)
{
    setup();
    //isso aqui faz algo importante 
    const struct device *input_dev;
    input_dev = DEVICE_DT_GET(INPUT_PORT);

    //habilita o pino como input, dos sensores infravermelho e do ultrassom
    gpio_pin_configure(input_dev, INPUT_PIN, GPIO_INPUT); //direito
    gpio_pin_configure(input_dev, INPUT_PIN2, GPIO_INPUT);//esquerdo
    gpio_pin_configure(input_dev, INPUT_PIN4, GPIO_INPUT);//central

    while (1) {
        gus  = gpio_pin_get(input_dev, INPUT_PIN);
        tavo = gpio_pin_get(input_dev, INPUT_PIN2);
        reh = gpio_pin_get(input_dev, INPUT_PIN4);;
        float aux;
        //giro suave
        if (reh == 1){
            pwm_tpm_CnV(TPM0, 1, 1000);
            //para esquerda
            if(gus == 1 && tavo == 0){
                mtr_Vrd_f(frntS); //esquerda para frente
                mtr_Vrm_t(trasS); //direita para atras
            }
            //para direita
            else if(gus == 0 && tavo == 1){
                mtr_Vrd_t(trasS); //esquerda para atras
                mtr_Vrm_f(frntS); //direita para frente
            }

            //caso contrario, va para frente!
        else{
            mtr_Vrd_f(frntS);
            mtr_Vrm_f(frntS);
        }
        }

        //giro grosseiro
        else if (reh == 0){
            pwm_tpm_CnV(TPM0, 1, 0);
            //para esquerda
            if(gus == 1 && tavo == 0){
                mtr_Vrd_f(frntG); //esquerda para frente
                mtr_Vrm_t(trasG);   //direita para atras
            }
            //para direita
            else if(gus == 0 && tavo == 1){
                mtr_Vrd_t(trasG);   //esquerda para atras
                mtr_Vrm_f(frntG); //direita para frente
            }

            //caso contrario, va para frente!
        else{
            mtr_Vrd_f(frntS);
            mtr_Vrm_f(frntS);
        }
        }
        k_msleep(20);
        aux = distancia;
        
        //ultrassom
        while(aux < paro){
            if(distancia - aux > 10){
                err = 1;
            }
            else{
                err = 0;
            }
            aux = (aux + distancia)/2;
            quierio(err);
        }
    
   }
}