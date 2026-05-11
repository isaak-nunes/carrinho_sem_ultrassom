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

volatile uint8_t err = 0;   // variavel relacionada a possiveis falhas do ultrassom do carrinho
volatile uint8_t gus= 0;    // variavel relacionada ao sensor direito do carrinho  
volatile uint8_t tavo= 0;   // variavel relacionada ao sensor esquerdo do carrinho
volatile uint8_t reh= 0;    // variavel relacionada ao sensor central do carrinho

// variaveis para o controle PID
volatile uint8_t Kp= 10;    // peso da proporção
volatile uint8_t Ki= 0;    // peso da integral
volatile uint8_t Kd= 0;    // peso da derivada

volatile uint8_t P = 0, I = 0, D = 0, PID = 0;    // -\(^-^)/-

volatile uint8_t Spl= 0, SpR = 0;    // peso da derivada

volatile uint8_t erro = 0, erro_anterior = 0;    // peso da derivada

int calcula_pid(void){
    if(erro == 0){
        I = 0;
    }
    P = erro;
    I += erro;

    if (I > 255){
        I = 255;
    }
    else if (I < -255){
        I = -255;
    }

    D = erro - erro_anterior;
    PID = (Kp * P) + (Ki * I) + (Kd * D);
    erro_anterior = erro;
}

void motor_dir(int v){       //função pro motor direito    
    pwm_tpm_CnV(TPM0, 2, v);     
    pwm_tpm_CnV(TPM0, 0, tpm);   
}

void motor_esq(int v)        //função pro motor esquerdo   
{
    pwm_tpm_CnV(TPM0, 3, v);
    pwm_tpm_CnV(TPM0, 4, tpm);
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

int calcula_erro(int Sd, int Sm, int Se){
    if(Sd == Sm && Sd == Se && Sm == Se){ erro = 0;}
    else if(Sd == 1 && Sm == 0 && Se == 1){ erro = 0;}  // linha esta no meio

    else if(Sd == 0 && Sm == 0 && Se == 1){ erro = 1;}  // linha esta entre a direita e o meio
    else if(Sd == 1 && Sm == 0 && Se == 0){ erro = -1;}  // linha esta entre a esqeuerda e o meio

    else if(Sd == 0 && Sm == 1 && Se == 1){ erro = 2;}  // linha esta na direita
    else if(Sd == 1 && Sm == 1 && Se == 0){ erro = -2;}  // linha esta na esquerda
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
        reh = gpio_pin_get(input_dev, INPUT_PIN4);
        calcula_pid();
        calcula_erro(gus, reh, tavo);
       
        if(PID >= 0){
            motor_dir(tpm - erro * 100);
            motor_esq(tpm);
        }
        else{
            motor_dir(tpm);
            motor_esq(tpm - erro * 100);
        }

        k_msleep(20);
   }
}