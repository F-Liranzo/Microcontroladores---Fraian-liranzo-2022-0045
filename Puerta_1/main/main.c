#include <stdio.h>
#include <stdlib.h>

#define ESTADO_INIT 0
#define ESTADO_CERRADO 1
#define ESTADO_ABIERTO 2
#define ESTADO_CERRANDO 3
#define ESTADO_ABRIENDO 4
#define ESTADO_ERROR 5 
#define TRUE 1
#define FALSE 0

int ESTADO_SIGUIENTE    = ESTADO_INIT;
int ESTADO_ACTUAL       = ESTADO_INIT;
int ESTADO_ANTERIOR     = ESTADO_INIT;



int main ()
{

for (;;)
{
 if (ESTADO_SIGUIENTE == ESTADO_INIT)
 {
    ESTADO_SIGUIENTE = Func_INIT();
 }

 if (ESTADO_SIGUIENTE == ESTADO_CERRADO )
 {
    ESTADO_SIGUIENTE = Func_CERRADO();

 }
 if (ESTADO_SIGUIENTE == ESTADO_ABIERTO )
 {
    ESTADO_SIGUIENTE = Func_ABIERTO();
 }
 if (ESTADO_SIGUIENTE == ESTADO_CERRANDO )
 {
    ESTADO_SIGUIENTE = Func_CERRANDO();
 }
 if (ESTADO_SIGUIENTE == ESTADO_ABRIENDO )
 {
    ESTADO_SIGUIENTE = Func_ABRIENDO();
 }
 if (ESTADO_SIGUIENTE == ESTADO_ERROR )
 {
    ESTADO_SIGUIENTE = Func_ERROR();
 }

    
}
return 0;
}
int Func_INIT (void)
{
    
}
int Func_CERRADO (void)
{
    
}
int Func_ABIERTO (void)
{
    
}
int Func_CERRANDO (void)
{
    
}
int Func_ABRIENDO (void)
{
    
}
int Func_ERROR (void)
{
    
}