#include <zephyr/kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <zephyr/device.h>             // API para obter e utilizar dispositivos do sistema
#include <zephyr/drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <math.h>
#include <pwm_z42.h>   

#define INPUT_PORT      DT_NODELABEL(gpioa)  // Porta E = GPIO_4 no seu .dts
#define INPUT_PIN       1         // PTE20
#define INPUT_PORT2     DT_NODELABEL(gpioa)  // Porta E = GPIO_4 no seu .dts
#define INPUT_PIN2      2         // PTE20

#define wait 50


//34464 é o valor maximo
#define TPM_MODULE 1024        // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))
// Valores de duty cycle correspondentes a diferentes larguras de pulso
uint16_t duty_0  = 0;
uint16_t duty_5  = TPM_MODULE/4;        // 50% de duty cycle (meio brilho)             
uint16_t duty_10  = TPM_MODULE;         // 100% de duty cycle (led desligado)
//IGNORE OS COMENTARIOS, O LED TA LIGADO NO ZERO E DESLIGADO DO 1024!!!!

enum estado_led {
    VERMELHO,
    AMARELO,
    VERDE,
    AZUL
    
};

//função que facilita a seleção de cor do led imbutido :)
void Led_rgb (int LED_VERMELHO, int LED_VERDE, int LED_AZUL){
    //multiplo pra facilitar o processo pra ficar bonito
    int m = floor(TPM_MODULE/256);
        pwm_tpm_CnV(TPM2, 0, TPM_MODULE - LED_VERMELHO * m);
        pwm_tpm_CnV(TPM2, 1, TPM_MODULE - LED_VERDE * m);
        pwm_tpm_CnV(TPM0, 1, TPM_MODULE - LED_AZUL * m);
}

//maquina de estado baseado na cor do lado
void cor_led(enum estado_led state) {
    //Iniciação das pinos de led
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18);     // Led vermelho
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19);     // Led verde
    pwm_tpm_Ch_Init(TPM0, 1, TPM_PWM_H, GPIOD, 1);      // Led azul

    //Iniciação dos pinos de uso geral
    pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOD, 2);      // porta de uso geral
    pwm_tpm_Ch_Init(TPM0, 3, TPM_PWM_H, GPIOD, 3);      // porta de uso geral

    switch (state) {
        
         case AZUL:
        //configuração dos pinos de led
        Led_rgb(0,0,255);

        //configuração dos pinos de uso geral
            pwm_tpm_CnV(TPM0, 2, duty_0);
            pwm_tpm_CnV(TPM0, 3, duty_0);
            break;

        case VERMELHO:
        //configuração dos pinos de led
        Led_rgb(255,0 ,0);
        //configuração dos pinos de uso geral
            pwm_tpm_CnV(TPM0, 2, duty_10);
            pwm_tpm_CnV(TPM0, 3, duty_5);
            break;

        case AMARELO:
        //configuração dos pinos de led
        Led_rgb(255,255,0);

        //configuração dos pinos de uso geral
            pwm_tpm_CnV(TPM0, 2, duty_10);
            pwm_tpm_CnV(TPM0, 3, duty_10);
            break;

        case VERDE:
        //configuração dos pinos de led
        Led_rgb(0,255,0);

        //configuração dos pinos de uso geral
            pwm_tpm_CnV(TPM0, 2, duty_5);
            pwm_tpm_CnV(TPM0, 3, duty_10);
            break;
    }
}

int main(void) {
    //iniciação das portas
        pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
        pwm_tpm_Init(TPM0, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    //estado inical
    enum estado_led current_state = AZUL; // Initial state
    const struct device *input_dev;
    const struct device *input_dev2;
    
    int ret, val, ret2, val2;
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    input_dev = DEVICE_DT_GET(INPUT_PORT);     
    input_dev2 = DEVICE_DT_GET(INPUT_PORT2);   

    ret = gpio_pin_configure(input_dev, INPUT_PIN, GPIO_INPUT);     //habilita o pino como input
    ret2 = gpio_pin_configure(input_dev2, INPUT_PIN2, GPIO_INPUT);  //habilita o pino2 como input


    while (1) { 
        cor_led(current_state);



        switch (current_state) {

            case AZUL:
            k_msleep(4000);
        val = gpio_pin_get(input_dev, INPUT_PIN);
        val2 = gpio_pin_get(input_dev2, INPUT_PIN2);
            current_state = AMARELO;
                break;

            case AMARELO:
            k_msleep(wait);
        while(val + val2 == 0)
        {
        val = gpio_pin_get(input_dev, INPUT_PIN);
        val2 = gpio_pin_get(input_dev2, INPUT_PIN2);
        }
            current_state = VERDE;
            break;

            case VERDE:
            k_msleep(wait);
            val = gpio_pin_get(input_dev, INPUT_PIN);
            val2 = gpio_pin_get(input_dev2, INPUT_PIN2);
            current_state = VERMELHO;
            break;

            case VERMELHO:
            k_msleep(wait);
            while(val2 != 1){
             val2 = gpio_pin_get(input_dev2, INPUT_PIN2);
            }
            current_state = AMARELO;
            break;
        }
    }
}