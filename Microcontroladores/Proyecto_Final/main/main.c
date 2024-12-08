#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <string.h>
#include "mqtt_client.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

//************************* Configuración Global ****************************
#define LOGICA_NEGATIVA "Negativa"
#define LOGICA_POSITIVA "Positiva"

#define LOGICA_ACTUAL LOGICA_NEGATIVA

uint8_t LOGIC_TRUE;
uint8_t LOGIC_FALSE;

// GPIOs configurados
#define GPIO_BUTTON 5
#define GPIO_LED 2

uint8_t buttonPressed = 0;
uint8_t buttonMQTT = 0;

// Estados de la máquina
uint8_t currentState = 0;
uint8_t previousState = 99;

// Wi-Fi
#define WIFI_SSID "Liranzo"
#define WIFI_PASSWORD "1234567f"
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t wifi_event_group;
static const char *TAG_WIFI = "WiFi";
static const char *TAG_MAIN = "Main";

// MQTT
#define MQTT_BROKER_URI "mqtt://broker.hivemq.com"

//************************* Declaración de Funciones ************************
void initializeGPIO(void);
void initializeWiFi(void);
void startMQTT(void);
void taskStateMachine(void *pvParameters);
void taskSerialInfo(void *pvParameters);
void taskLEDBlink(void *pvParameters);
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

//************************* Implementación Principal ************************
void app_main()
{
    initializeGPIO();

    // Inicializar NVS
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_LOGI(TAG_MAIN, "Inicializando Wi-Fi...");
    initializeWiFi();

    ESP_LOGI(TAG_MAIN, "Inicializando MQTT...");
    startMQTT();

    // Crear tareas FreeRTOS
    xTaskCreate(taskStateMachine, "State Machine", 2048, NULL, 5, NULL);
    xTaskCreate(taskSerialInfo, "Serial Info", 2048, NULL, 5, NULL);
    xTaskCreate(taskLEDBlink, "LED Blink", 2048, NULL, 5, NULL);
}

//************************* Inicialización de GPIO **************************
void initializeGPIO()
{
    if (strcmp(LOGICA_ACTUAL, LOGICA_NEGATIVA) == 0)
    {
        LOGIC_TRUE = 0;
        LOGIC_FALSE = 1;
        ESP_LOGI("GPIO", "Lógica Negativa configurada.");
    }
    else if (strcmp(LOGICA_ACTUAL, LOGICA_POSITIVA) == 0)
    {
        LOGIC_TRUE = 1;
        LOGIC_FALSE = 0;
        ESP_LOGI("GPIO", "Lógica Positiva configurada.");
    }
    else
    {
        ESP_LOGE("GPIO", "Error: lógica desconocida.");
    }

    // Configuración del botón
    gpio_reset_pin(GPIO_BUTTON);
    gpio_set_direction(GPIO_BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_BUTTON, GPIO_PULLUP_ONLY);

    // Configuración del LED
    gpio_reset_pin(GPIO_LED);
    gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);

    ESP_LOGI("GPIO", "GPIO inicializado correctamente.");
}

//************************* Manejo de Wi-Fi *********************************
void initializeWiFi()
{
    wifi_event_group = xEventGroupCreate();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG_WIFI, "Conectando a Wi-Fi...");
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG_WIFI, "Conexión fallida, intentando reconectar...");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_WIFI, "Conectado con IP:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

//************************* Manejo de MQTT **********************************
void startMQTT()
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .session.protocol_ver = MQTT_PROTOCOL_V_5,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        esp_mqtt_client_subscribe(event->client, "/Fraian Liranzo/SPP", 1);
        break;

    case MQTT_EVENT_DATA:
        if (strncmp(event->topic, "/Fraian Liranzo/SPP", event->topic_len) == 0)
        {
            char received_data[2];
            snprintf(received_data, sizeof(received_data), "%.*s", event->data_len, event->data);
            buttonMQTT = (strcmp(received_data, "1") == 0) ? 1 : 0;
        }
        break;

    default:
        break;
    }
}

//************************* Tareas FreeRTOS *********************************
void taskStateMachine(void *pvParameters)
{
    while (1)
    {
        if (!buttonPressed && gpio_get_level(GPIO_BUTTON) == LOGIC_TRUE)
        {
            buttonPressed = 1;
            currentState = (currentState < 4) ? currentState + 1 : 0;
        }
        else if (gpio_get_level(GPIO_BUTTON) == LOGIC_FALSE)
        {
            buttonPressed = 0;
        }

        if (buttonMQTT == 1)
        {
            buttonMQTT = 0;
            currentState = (currentState < 4) ? currentState + 1 : 0;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void taskSerialInfo(void *pvParameters)
{
    while (1)
    {
        if (previousState != currentState)
        {
            ESP_LOGI("Serial Info", "Estado = %d", currentState);
            previousState = currentState;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void taskLEDBlink(void *pvParameters)
{
    uint16_t contadorActual = 0;
    uint8_t ledLevel = 0;
    uint8_t estadoLedAnterior = 0; // Para detectar cambios de estado
    uint16_t contadorBarrido = 100; // Inicia en 100 ms
    uint16_t tiempoBarrido = 0;

    while (1)
    {
        // Detectar cambio de estado
        if (estadoLedAnterior != currentState)
        {
            estadoLedAnterior = currentState;
            contadorActual = 0; // Reiniciar contador
            tiempoBarrido = 0;
            contadorBarrido = 100; // Reiniciar a 100 ms
            ledLevel = 1;
            gpio_set_level(GPIO_LED, ledLevel); // Encender LED
        }

        // Lógica según el estado actual
        switch (currentState)
        {
        case 1:
            if (contadorActual >= 500) // Parpadeo cada 500 ms
            {
                contadorActual = 0;
                ledLevel = !ledLevel;
                gpio_set_level(GPIO_LED, ledLevel);
            }
            contadorActual += 10;
            break;

        case 2:
            if (contadorActual >= 100) // Parpadeo cada 100 ms
            {
                contadorActual = 0;
                ledLevel = !ledLevel;
                gpio_set_level(GPIO_LED, ledLevel);
            }
            contadorActual += 10;
            break;

        case 3:
            if (contadorActual >= 1000) // Parpadeo cada 1 segundo
            {
                contadorActual = 0;
                ledLevel = !ledLevel;
                gpio_set_level(GPIO_LED, ledLevel);
            }
            contadorActual += 10;
            break;

        case 4:
            // Incrementar el periodo cada 1 segundo
            if (tiempoBarrido >= 1000)
            {
                if (contadorBarrido > 1000) // Reinicia a 100 ms si supera 1 segundo
                {
                    contadorBarrido = 100;
                }
                else
                {
                    contadorBarrido += 100; // Incrementa 100 ms
                }

                tiempoBarrido = 0; // Reiniciar tiempo de barrido
            }

            // Parpadeo del LED con el periodo actual
            if (contadorActual >= contadorBarrido)
            {
                contadorActual = 0;
                ledLevel = !ledLevel;
                gpio_set_level(GPIO_LED, ledLevel);
            }

            // Incrementar contadores
            tiempoBarrido += 10;
            contadorActual += 10;
            break;

        default: // Estado 0 u otros
            gpio_set_level(GPIO_LED, 0); // LED apagado
            contadorActual = 0;
            break;
        }

        // Delay de 10 ms
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
