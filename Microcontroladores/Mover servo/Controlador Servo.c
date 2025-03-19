// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"

// #define LED_BUILTIN 2  // GPIO del LED integrado
// #define BOTON 4

// void app_main(void) {  
//     gpio_reset_pin(LED_BUILTIN);
//     gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);

//     gpio_reset_pin(BOTON);
//     gpio_set_pull_mode(BOTON, GPIO_PULLUP_ONLY);
//     gpio_set_direction(BOTON, GPIO_MODE_INPUT);

//     bool estadoled = false;  // Estado del LED
//     bool estadoAnteriorBoton = false;  // Para detectar cambio de estado del botón

//     while (1) {
//         int estadoBoton = gpio_get_level(BOTON); // Leer el estado del botón

//         // Detectar flanco de bajada (cuando el botón es presionado)
//         if (estadoBoton == 0 && estadoAnteriorBoton == 1) {
//             estadoled = !estadoled;  // Alternar el estado del LED
//             gpio_set_level(LED_BUILTIN, estadoled); 
//         }

//         estadoAnteriorBoton = estadoBoton; // Guardar el estado del botón

//         vTaskDelay(50 / portTICK_PERIOD_MS);  // Pequeño retardo para evitar rebotes
//     }
// }

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

#define SERVO_PIN 5          // Pin del servo
#define BOTON_DERECHA 4      // Botón para girar a la derecha
#define BOTON_IZQUIERDA 18   // Botón para girar a la izquierda
#define BOTON_VELOCIDAD_AUMENTAR 19  // Botón para aumentar la velocidad
#define BOTON_VELOCIDAD_DISMINUIR 23  // Botón para disminuir la velocidad
#define PWM_FREQUENCY 50     // Frecuencia de 50Hz para servos
#define PWM_CHANNEL LEDC_CHANNEL_0
#define PWM_RESOLUTION LEDC_TIMER_16_BIT  // Resolución de 16 bits

int angulo = 180;  // Posición inicial del servo en el centro
int velocidad = 2; // Velocidad inicial (cuántos grados se mueve por pulsación)

// lave abierta 180 ,, llave cerrada Ángulo: 84, Ciclo de trabajo PWM: 5570



void moverServo(int angulo) {
    int dutyCycle = (angulo * (8191 - 3277)) / 180 + 3277; // Mapear ángulo (0° a 180°) a PWM
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, dutyCycle);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL);
    printf("Ángulo: %d, Ciclo de trabajo PWM: %d\n", angulo, dutyCycle);  // Imprimir valores en el serial
}

void app_main() {
    // Inicializar la comunicación serial
    printf("Iniciando el control del servo...\n");

    // Configurar el temporizador para el PWM
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQUENCY
    };
    ledc_timer_config(&timer);

    // Configurar el canal PWM
    ledc_channel_config_t channel = {
        .gpio_num = SERVO_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };
    ledc_channel_config(&channel);

    // Configurar los botones como entradas con resistencia pull-up
    gpio_reset_pin(BOTON_DERECHA);
    gpio_set_pull_mode(BOTON_DERECHA, GPIO_PULLUP_ONLY);
    gpio_set_direction(BOTON_DERECHA, GPIO_MODE_INPUT);

    gpio_reset_pin(BOTON_IZQUIERDA);
    gpio_set_pull_mode(BOTON_IZQUIERDA, GPIO_PULLUP_ONLY);
    gpio_set_direction(BOTON_IZQUIERDA, GPIO_MODE_INPUT);

    gpio_reset_pin(BOTON_VELOCIDAD_AUMENTAR);
    gpio_set_pull_mode(BOTON_VELOCIDAD_AUMENTAR, GPIO_PULLUP_ONLY);
    gpio_set_direction(BOTON_VELOCIDAD_AUMENTAR, GPIO_MODE_INPUT);

    gpio_reset_pin(BOTON_VELOCIDAD_DISMINUIR);
    gpio_set_pull_mode(BOTON_VELOCIDAD_DISMINUIR, GPIO_PULLUP_ONLY);
    gpio_set_direction(BOTON_VELOCIDAD_DISMINUIR, GPIO_MODE_INPUT);

    // Mover el servo a la posición inicial (90°) inmediatamente
    moverServo(angulo);

    while (1) {
        // Leer el estado de los botones para mover el servo
        if (gpio_get_level(BOTON_DERECHA) == 0) {  // Si se presiona el botón derecha
            if (angulo < 180) {
                angulo += velocidad;  // Incrementar ángulo por la velocidad definida
                if (angulo > 180) angulo = 180; // Limitar a 180°
                moverServo(angulo);
                vTaskDelay(pdMS_TO_TICKS(100));  // Pequeño retraso para suavizar movimiento
            }
        }

        if (gpio_get_level(BOTON_IZQUIERDA) == 0) {  // Si se presiona el botón izquierda
            if (angulo > 0) {
                angulo -= velocidad;  // Decrementar ángulo por la velocidad definida
                if (angulo < 0) angulo = 0; // Limitar a 0°
                moverServo(angulo);
                vTaskDelay(pdMS_TO_TICKS(100));  // Pequeño retraso para suavizar movimiento
            }
        }

        // Leer los botones para cambiar la velocidad
        if (gpio_get_level(BOTON_VELOCIDAD_AUMENTAR) == 0) {  // Si se presiona el botón para aumentar la velocidad
            if (velocidad < 10) {  // Limitar la velocidad máxima
                velocidad++;
                printf("Velocidad aumentada: %d\n", velocidad);
                vTaskDelay(pdMS_TO_TICKS(200));  // Retraso para evitar lecturas múltiples
            }
        }

        if (gpio_get_level(BOTON_VELOCIDAD_DISMINUIR) == 0) {  // Si se presiona el botón para disminuir la velocidad
            if (velocidad > 1) {  // Limitar la velocidad mínima
                velocidad--;
                printf("Velocidad disminuida: %d\n", velocidad);
                vTaskDelay(pdMS_TO_TICKS(200));  // Retraso para evitar lecturas múltiples
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));  // Pequeño retraso para evitar rebotes
    }
}

