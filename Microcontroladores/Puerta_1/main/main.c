#include <stdio.h>
#include<stdlib.h>

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

int ESTADO_SIGUIENTE    = ESTADO_INIT;
int ESTADO_ACTUAL       = ESTADO_INIT ;
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

   delay();
    data_io.led_A = FALSE;
   data_io.led_C = FALSE;
   data_io.led_ERR = FALSE;
 // data_io.COD_ERR = FALSE;
  data_io.cont_RT = 0;
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

{ 
   ESTADO_ANTERIOR = ESTADO_ACTUAL;
   ESTADO_ACTUAL = ESTADO_ERROR;
   data_io.MC = FALSE;
   data_io.MA = FALSE;
    data_io.led_A= FALSE;
   data_io.led_C = FALSE;
   data_io.led_ERR = TRUE;
}







