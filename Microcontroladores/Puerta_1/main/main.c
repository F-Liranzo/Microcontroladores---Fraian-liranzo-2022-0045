#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>  // Incluir la biblioteca de Paho MQTT

#define ESTADO_INIT 0
#define ESTADO_CERRADO 1
#define ESTADO_ABIERTO 2
#define ESTADO_CERRANDO 3
#define ESTADO_ABRIENDO 4
#define ESTADO_ERROR 5
#define TRUE 1
#define FALSE 0

int ESTADO_SIGUIENTE = ESTADO_INIT;
int ESTADO_ACTUAL = ESTADO_INIT;
int ESTADO_ANTERIOR = ESTADO_INIT;

struct DATA_IO
{
   unsigned int LSC : 1; // Limit switch Cerrado 
   unsigned int LSA : 1; // limit switch Abierto 
   unsigned int SPP : 1; // Pulsador de apertura o cierre
} data_io;

struct mosquitto *mqtt_client;  // Cliente MQTT

void mqtt_connect_callback(struct mosquitto *client, void *obj, int rc);
void mqtt_publish_state(int estado);

int main()
{
   // Inicializar la biblioteca MQTT
   mosquitto_lib_init();

   mqtt_client = mosquitto_new(NULL, true, NULL);
   if (!mqtt_client)
   {
      fprintf(stderr, "Error al crear el cliente MQTT.\n");
      return 1;
   }

   // Configurar el callback de conexión
   mosquitto_connect_callback_set(mqtt_client, mqtt_connect_callback);

   // Conectar al servidor MQTT
   if (mosquitto_connect(mqtt_client, "localhost", 1883, 60) != MOSQ_ERR_SUCCESS)
   {
      fprintf(stderr, "Error al conectar al servidor MQTT.\n");
      return 1;
   }

   // Bucle principal de la máquina de estados
   for (;;)
   {
      if (ESTADO_SIGUIENTE == ESTADO_INIT)
      {
         ESTADO_SIGUIENTE = Func_INIT();
      }

      if (ESTADO_SIGUIENTE == ESTADO_CERRADO)
      {
         ESTADO_SIGUIENTE = Func_CERRADO();
      }
      if (ESTADO_SIGUIENTE == ESTADO_ABIERTO)
      {
         ESTADO_SIGUIENTE = Func_ABIERTO();
      }
      if (ESTADO_SIGUIENTE == ESTADO_CERRANDO)
      {
         ESTADO_SIGUIENTE = Func_CERRANDO();
      }
      if (ESTADO_SIGUIENTE == ESTADO_ABRIENDO)
      {
         ESTADO_SIGUIENTE = Func_ABRIENDO();
      }
      if (ESTADO_SIGUIENTE == ESTADO_ERROR)
      {
         ESTADO_SIGUIENTE = Func_ERROR();
      }

      // Publicar el estado cada vez que cambie
      if (ESTADO_SIGUIENTE != ESTADO_ACTUAL)
      {
         mqtt_publish_state(ESTADO_SIGUIENTE);
         ESTADO_ACTUAL = ESTADO_SIGUIENTE;
      }

      mosquitto_loop(mqtt_client, -1, 1);  // Mantener el loop MQTT
   }

   // Limpiar y cerrar el cliente MQTT
   mosquitto_destroy(mqtt_client);
   mosquitto_lib_cleanup();
   return 0;
}

void mqtt_connect_callback(struct mosquitto *client, void *obj, int rc)
{
   if (rc != MOSQ_ERR_SUCCESS)
   {
      printf("Conexión fallida con código: %d\n", rc);
   }
   else
   {
      printf("Conectado al servidor MQTT\n");
   }
}

void mqtt_publish_state(int estado)
{
   char payload[50];

   // Convertir el estado a un string
   switch (estado)
   {
      case ESTADO_INIT:   snprintf(payload, sizeof(payload), "Estado: INICIAL"); break;
      case ESTADO_CERRADO: snprintf(payload, sizeof(payload), "Estado: CERRADO"); break;
      case ESTADO_ABIERTO: snprintf(payload, sizeof(payload), "Estado: ABIERTO"); break;
      case ESTADO_CERRANDO: snprintf(payload, sizeof(payload), "Estado: CERRANDO"); break;
      case ESTADO_ABRIENDO: snprintf(payload, sizeof(payload), "Estado: ABRIENDO"); break;
      case ESTADO_ERROR:   snprintf(payload, sizeof(payload), "Estado: ERROR"); break;
      default:             snprintf(payload, sizeof(payload), "Estado desconocido");
   }

   // Publicar el estado en el topic "puerta/estado"
   mosquitto_publish(mqtt_client, NULL, "puerta/estado", strlen(payload), payload, 0, false);
}

int Func_INIT(void)
{
   printf("Estado Inicial\n");
   if (data_io.LSC == TRUE && data_io.LSA == FALSE)
   {
      return ESTADO_CERRADO;
   }
   if (data_io.LSC == TRUE && data_io.LSA == TRUE)
   {
      return ESTADO_ERROR;
   }
   if (data_io.LSC == FALSE && data_io.LSA == FALSE)
   {
      return ESTADO_CERRANDO;
   }
   if (data_io.LSC == FALSE && data_io.LSA == TRUE)
   {
      return ESTADO_ABRIENDO;
   }
   return ESTADO_INIT;
}

int Func_CERRADO(void)
{
   ESTADO_ANTERIOR = ESTADO_ACTUAL;
   ESTADO_ACTUAL = ESTADO_CERRADO;
   printf("Estado Cerrado\n");

   if (data_io.SPP == TRUE)
   {
      return ESTADO_ABRIENDO;
   }
   return ESTADO_CERRADO;
}

int Func_ABIERTO(void)
{
   ESTADO_ANTERIOR = ESTADO_ACTUAL;
   ESTADO_ACTUAL = ESTADO_ABIERTO;
   printf("Estado Abierto\n");

   if (data_io.SPP == TRUE)
   {
      return ESTADO_CERRANDO;
   }
   return ESTADO_ABIERTO;
}

int Func_CERRANDO(void)
{
   ESTADO_ANTERIOR = ESTADO_ACTUAL;
   ESTADO_ACTUAL = ESTADO_CERRANDO;
   printf("Estado Cerrando\n");

   if (data_io.LSC == TRUE)
   {
      return ESTADO_CERRADO;
   }

   if (data_io.LSA == TRUE)
   {
      return ESTADO_ERROR;
   }

   return ESTADO_CERRANDO;
}

int Func_ABRIENDO(void)
{
   ESTADO_ANTERIOR = ESTADO_ACTUAL;
   ESTADO_ACTUAL = ESTADO_ABRIENDO;
   printf("Estado Abriendo\n");

   if (data_io.LSA == TRUE)
   {
      return ESTADO_ABIERTO;
   }

   if (data_io.LSC == TRUE)
   {
      return ESTADO_ERROR;
   }

   return ESTADO_ABRIENDO;
}

int Func_ERROR(void)
{
   ESTADO_ANTERIOR = ESTADO_ACTUAL;
   ESTADO_ACTUAL = ESTADO_ERROR;
   printf("Estado de Error\n");

   if (data_io.SPP == TRUE)
   {
      return ESTADO_INIT;
   }

   return ESTADO_ERROR;
}
