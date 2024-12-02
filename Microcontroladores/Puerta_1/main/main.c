#include <stdio.h>
#include<stdlib.h>

// librerias esp-32
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define ESTADO_INIT 0
#define ESTADO_CERRADO 1
#define ESTADO_ABIERTO 2
#define ESTADO_CERRANDO 3
#define ESTADO_ABRIENDO 4 
#define ESTADO_ERROR 5 
#define TRUE 1
#define FALSE 0 
#define RT_MAX 180 // tiempo limite del contador 
#define ERROR_LS 1
#define ERROR_RT 2


// Asignarle pines de salida al esp 32 // 
#define limit_shwitch_abierto 26
#define limit_shwitch_cerrado 5
#define MOTOR_ABRIR 2
#define MOTOR_CERRAR 4
#define BOTON 34
#define LED_ABRIENDO
#define LED_CERRANDO 
#define LED_ERROR 15



int ESTADO_SIGUIENTE    = ESTADO_INIT;
int ESTADO_ACTUAL       = ESTADO_INIT;
int ESTADO_ANTERIOR     = ESTADO_INIT; 

struct DATA_IO  //ESTRUCTURA DE ENTRADA DE DATOS , en este caso los sensores 
{
   // Unsigned int con :1 especificando que esta esperando un dato de un solo bit sin signo 
   ///////////////////////////////////////////////////////////////////////////////////////////

   unsigned int LSC : 1; // Limit switch Cerrado 
   unsigned int LSA : 1; // limit switch Abierto 
   unsigned int SPP : 1; // comando pulso - pulso 
   unsigned int MA : 1; // Motor abre 
   unsigned int MC : 1;// Motor cierra 
   unsigned int cont_RT ; // contador de runtime en segundos 
   unsigned int led_A : 1;
   unsigned int led_C : 1;
   unsigned int led_ERR : 1;
  // unsigned int COD_ERR;
   //unsigned int DATOS_READY : 1; // aqui se indica si los datos son leidos correctamente 


} data_io;

void cofiguracion_salidas (void)
{
// Configurar LED como salida
    gpio_reset_pin(LED_ABRIENDO);
    gpio_set_direction(LED_ABRIENDO, GPIO_MODE_OUTPUT);

     gpio_reset_pin(LED_CERRANDO);
    gpio_set_direction(LED_CERRANDO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(MOTOR_ABRIR);
    gpio_set_direction(MOTOR_ABRIR, GPIO_MODE_OUTPUT);

    gpio_reset_pin(MOTOR_CERRAR);
    gpio_set_direction(MOTOR_CERRAR, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_ERROR);
    gpio_set_direction(LED_ERROR, GPIO_MODE_OUTPUT);


    // Configurar botón como entrada con pull-down
    gpio_reset_pin(BOTON);
    gpio_set_pull_mode(BOTON, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(BOTON, GPIO_MODE_INPUT);

     gpio_reset_pin(limit_shwitch_abierto);
    gpio_set_pull_mode(limit_shwitch_abierto, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(limit_shwitch_abierto, GPIO_MODE_INPUT);

     gpio_reset_pin(limit_shwitch_cerrado);
    gpio_set_pull_mode(limit_shwitch_cerrado, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(limit_shwitch_cerrado, GPIO_MODE_INPUT);
}

void actualizar_datos (void)
{
    //data_io.DATOS_READY = FALSE;
    vTaskDelay(10/portTICK_PERIOD_MS);

    data_io.LSA = gpio_get_level(limit_shwitch_abierto);
    data_io.LSC = gpio_get_level(limit_shwitch_cerrado);
    gpio_set_level(MOTOR_ABRIR, data_io.MA);
    gpio_set_level(MOTOR_CERRAR, data_io.MC);
    gpio_set_level(LED_ABRIENDO, data_io.Led_A);
    gpio_set_level(LED_CERRANDO, data_io.Led_C);
    gpio_set_level(LED_ERROR, data_io.led_ERR);
    //data_io.DATOS_READY = TRUE;
}


int main ()
{
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
    
   }
   return 0;
}



int Func_INIT(void)
{

   
   ESTADO_ANTERIOR = ESTADO_INIT ;
   ESTADO_ACTUAL = ESTADO_INIT ;
   data_io.MC = FALSE;
   data_io.MA = FALSE;

    data_io.led_A= TRUE;// al encender la maquina todos los led encienden 
   data_io.led_C = TRUE;
   data_io.led_ERR = TRUE;

   //delay();
    //data_io.led_A = FALSE;
   //data_io.led_C = FALSE;
   //data_io.led_ERR = FALSE;
 // data_io.COD_ERR = FALSE;
  //data_io.cont_RT = 0;
 // data_io.DATOS_READY = FALSE;

 // while (!data_io.DATOS_READY); // SI LOS DATOS NO SON LEIDOS EL PROGRAMA NO CORRE 
  
   for (;;)
   {
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
         return ESTADO_CERRANDO;
      }
   }
}


//////////////////////////////////////////////////




int Func_CERRADO(void)
{
 ESTADO_ANTERIOR = ESTADO_ACTUAL;
   ESTADO_ACTUAL = ESTADO_CERRADO;
   data_io.MC = FALSE;
   data_io.SPP = FALSE; // se asegura el estado poniendo la variable false 
   data_io.led_A= FALSE;
   data_io.led_C = FALSE;
   data_io.led_ERR = FALSE;
   printf("\n Cerrado\n");

   actualizar_datos ();

   for (;;)
   {
      if (data_io.SPP == TRUE)
      {
         data_io.SPP = FALSE; // asignandole el valor false a la señal pp
          return ESTADO_ABRIENDO;
      }
   }
   
   
}

int Func_ABIERTO(void)
{
  ESTADO_ANTERIOR = ESTADO_ACTUAL;
   ESTADO_ACTUAL = ESTADO_ABIERTO;
   data_io.MA = FALSE;
   data_io.SPP = FALSE; // se asegura el estado poniendo la variable false 
   data_io.led_A= FALSE;
   data_io.led_C = FALSE;
   data_io.led_ERR = FALSE;

    printf("\n abierto\n");

   actualizar_datos ();


   for (;;)
   {
      if (data_io.SPP == TRUE)
      {
         data_io.SPP = FALSE; // asignandole el valor false a la señal pp
          return ESTADO_CERRANDO;
      }
   }
     
  
}
int Func_CERRANDO(void)
{
   ESTADO_ANTERIOR = ESTADO_ACTUAL;
   ESTADO_ACTUAL = ESTADO_CERRANDO;
   data_io.MC = TRUE;
   data_io.cont_RT = 0; // Asigna el contador de runtime a 0 
    data_io.led_A= FALSE;
   data_io.led_C = TRUE;
   data_io.led_ERR = FALSE;

    printf("\n cerrando \n");

   actualizar_datos ();

   for (;;)
   {
      // Si el limit switch cerrado es verdadero , devuelve el estado cerrado 
      if (data_io.LSC == TRUE ) 
      {
         return ESTADO_CERRADO ;
      }

       if (data_io.cont_RT > RT_MAX ) // si el contador de runtime supera el limite de tiempo devuelve el estado de error 
      {
         return ESTADO_ERROR ;
      }
      
   }
   
}

int Func_ABRIENDO(void)
{
   ESTADO_ANTERIOR = ESTADO_ACTUAL;
   ESTADO_ACTUAL = ESTADO_ABRIENDO;
   data_io.MC = TRUE;
   data_io.cont_RT = 0; // Asigna el contador de runtime a 0 
    data_io.led_A= TRUE;
   data_io.led_C = FALSE;
   data_io.led_ERR = FALSE;

    printf("\n abriendo\n");

   actualizar_datos ();

   for (;;)
   {
      // Si el limit switch abierto es verdadero , devuelve el estado abierto 
      if (data_io.LSA == TRUE ) 
      {
         return ESTADO_ABIERTO ;
      }

       if (data_io.cont_RT > RT_MAX ) // si el contador de runtime supera el limite de tiempo devuelve el estado de error 
      {
         return ESTADO_ERROR ;
      }
      
   }
   
}
int Func_ERROR(void)                                   

 printf("\n error \n");

   actualizar_datos ();

{ 
   ESTADO_ANTERIOR = ESTADO_ACTUAL;
   ESTADO_ACTUAL = ESTADO_ERROR;
   data_io.MC = FALSE;
   data_io.MA = FALSE;
    data_io.led_A= FALSE;
   data_io.led_C = FALSE;
   data_io.led_ERR = TRUE;
}







