/***********************************************
Definiciones Ethernet
modificado puerto de activacion desactivacion Sim 900  2/11/2012
configurado para puerto serie
se ha modificado input lector wiegand-
Se reemplazo el chip rs 232 y se colocaron resistencias
Se agregó en Seteo Equipo Nro Celular,Ip y Gateway
Se agregó WDG
Se cambió Lectura Wgd
Se cambió Web con Seguridad desactivada
Para Hard Viejo (cambiar entradas RFID y EE)
***********************************************/

#define TCPCONFIG 0
#define USE_ETHERNET 1
#define MY_PORT   23
#define DEFAULT_SOURCEIP      "192.168.1.98"
#define DEFAULT_NETMASK       "255.255.255.0"
#define DEFAULT_GATEWAY       "192.168.1.1"
#define DEFAULT_DNS           "192.168.1.1"
#memmap xmem
#use "dcrtcp.lib"
#use "http.lib"
#define DINBUFSIZE  255
#define DOUTBUFSIZE 511
#define REDIRECTOK      "/"
//#define REDIRECTTOK      "http://" MY_IP_ADDRESS "/index.shtml"
//#define REDIRECTTODOS "/dosboton.shtml"
//#define REDIRECTTODOS "/salidas.shtml"

//#ximport "index.html"   index_html
//#ximport  "rabbit1.gif"  rabbit1_gif
//#ximport "dosboton.shtml"  dosboton_html
#ximport "webcontrolbox.shtml"  webcontrolbox_html
//#ximport "entradas.shtml"  entradas_html
//#ximport "salidas1.shtml"  salidas1_html


typedef struct {
   longword src_ip;
   longword src_mask;
   longword def_gwy;
   longword my_dns;
} Config;

Config myconfig;
char my_ip[16],my_mask[16],my_gwy[16],my_dns[16];

enum {READ=0,WRITE,READED};
char bufferrx[1024];
char buffertx[1024];
int len;
enum{IDLE,INIT,LISTEN,OPEN,CLOSE,CLOSING};
int state,astate;
int stateold,astateold;
int leido;
int habilitar_telnet;
int reiniciar_tcp;


static char ANSW[500], *answ;     // Buffer de recepción
char STRING[200], *string;        // Buffer de transmision
char STRING_TX[400], *string_tx;  // Buffer de transmision hacia PC

#define SET_CONTROL   (4096*GetIDBlockSize()-0x300)// tomo 1036 para controler y offset
#define VERSION  "ControlBox14_WEB.c  12/01/2017\r\n"
#define PROGRAMADOR  "Beto"
#define Celular_Prueba  "+543413031303"

typedef struct {
char  lector;          //Nro Controlador ABC....
char  serie[6];        //Nro serie equipo
char  descripcion[20]; //Descripción Equipo
char  nro_celular[10]; //Numero de celular instalado
char  ip[15];          //IP del equipo
char  gateway[15];     //Gateway del equipo
}Controler;
Controler Equipo_Setup;

struct tm      rtc;

/***********************************************
Fin Definiciones Ethernet
***********************************************/

/*****************************************************
     Definición de los buffers del puerto serie.
******************************************************/
#define CINBUFSIZE  255
#define COUTBUFSIZE 512
#define BINBUFSIZE  255
#define BOUTBUFSIZE 512
#define  B_BAUDRATE  19200
/*****************************************************
     Definición de los buffers para almacenar en flash

******************************************************/

#define TEL_HAB   (4096*GetIDBlockSize()-0x5F0) // tomo 1024 para tel y offset
#define TRJ_HAB   (4096*GetIDBlockSize()-0x780) // 400 Bytes para Tarjetas
#define ENS_HAB   (4096*GetIDBlockSize()-0x8DE) // 350 Bytes relacionE/S
#define SMS_HAB   (4096*GetIDBlockSize()-0xA3C) // 350 Bytes para SMS
#define ENT_HAB   (4096*GetIDBlockSize()-0xDC0) // 900 Bytes SMS entradas(0/1)
#define EQP_HAB   (4096*GetIDBlockSize()-0xE56) // 150 Bytes Equipo/Variables
#define SAL_HAB   (4096*GetIDBlockSize()-0xFB4) // 350 Bytes Entradas
#define MAC_HAB   (4096*(GetIDBlockSize()-1)-0x0C8) // 150 Bytes Macros
#define CONFIG_IP (4096*(GetIDBlockSize()-1)-0x12C) // 50 Bytes configuracion IP
#define ALM_HAB 	(4096*(GetIDBlockSize()-1)-0x258) // 250 Bytes Tipos de activación

//void recept_time();
#define FIN    "\n"
#define SINAUTO    "\"\nNo esta autorizado a realizar la operacion solicitada.\n"
#define BLKL1    "                 #1 \r\n";    //Buffer TRJ1 BLK
char *const blkl1 = BLKL1;
#define BLKL2    "                 #2 \r\n";    //Buffer TRJ1 BLK
char *const blkl2 = BLKL2;
int v, j, i, h, x;
//char   answ[500];        // Buffer de recepción
char  Comando[5],*comando;
int count;
char SIM_on;             //Bandera Inicio SIM
char SIM_off;              //Bandera Apagado SIM
char SIM_OK;               //Bandera SIM Habilitado
char SIM_reset;            //Bandera de SIM reset - Apagado/Encendido
int Set_SIM;               //Indicador de seteo Sim
int Llamada_OK;            //Indicador de llamada de Alarma
int Llamada_SAL;           //Indicador de llamada saliente en curso
int EnvioSMS_ON;           //Indicador de envío SMS en curso
int EnvioSMS_ERROR;        //Contador errores de envío SMS


//char STRING[200], *string;
unsigned long t0, t1;     // usado para fecha y conversión de la hora
unsigned long tds;            //hora de disparo
unsigned long tcee;           //hora de corte energía
//unsigned int  counta, countinter, countintRX; //Contadores de interrup 0.051ms y 20ms
unsigned int countinter; //Contadores de interrup 0.051ms y 20ms

char con1,con2;               //contadores 0 y 1 tarjetas
char SMS_delay;               //indicador de envio SMS
char RING_value;              //Contador de ring's
static char NUMERO[14], *numero;    // String del numero a utilizar
static char NOMBRE[11], *nombre;    // String del nombre a utilizar
static char NOMBRE_ACT[11], *nombre_act;  //nombre de quien activó o desactivo alarma
char NUMERO_SMS[14],*numero_sms;   //Numero de tel que envio SMS a Control Box
static char TEXTO_SMS[31], *texto_sms;    // String del mensaje a utilizar
static char texto_bsq[31];    // String del mensaje a utilizar
static char *bsq,*bsqold;              // Puntero de busqueda


int c,l,n;
int k,id;
int intr;
//buffer de tarjetas
static char SERIAL1[22], *serial1;           // Buffer de Numero de Serie Tarjeta1
static char SERIAL2[22], *serial2;           // Buffer de Numero de Serie Tarjeta1


static char Buffer_ens[349], *buffer_ens;    // Buffer de relacionE/S
static char Buffer_sms[350], *buffer_sms;    // Buffer de Mensajes Habilitados

static char Buffer_sal[304], *buffer_sal;    // Buffer de Entradas
static char Buffer_mac[108], *buffer_mac;    // Buffer de Entradas
static char buffer[500];
static char bufferc[500];
static char Buffer_txc[100], *buffer_txc;
static char Buffer_alm[250], *buffer_alm;    //almac programacion tipo de alarmas
char CARACTER[6];
char SALIDA[16];
static char PORTERO1;              				// indicador de apertura portero lector1
static char PORTERO2;              				// indicador de apertura portero lector2
//defino la estructura de la info tarjetas
typedef struct {
unsigned long fcha;
char  estdo;
char trjta[6];
}BuffTarj;

BuffTarj buff_tarj1;      //lectura tarjeta Lector 1
BuffTarj buff_tarj2;      //lectura tarjeta Lector 2
BuffTarj buff_almac;       //Buffer para almacenamiento






//estructura envio de SMS
typedef struct {
char  numero_envio[14];
char  mensaje[250];
}SMS_envio;
SMS_envio  buff_envioSMS;    //buffer para envío SMS
SMS_envio  buff_almacSMS;    //buffer para almac SMS
#define LARGO_ENVIOSMS  20
char Buffer_envioSMS[sizeof(SMS_envio)*LARGO_ENVIOSMS],*buffer_envioSMS;  // Buffer de SMS a enviar
int puntin_SMS,puntout_SMS;         //punteros de buffer SMS leídas


//defino estructura de fecha
struct fecha{
char anio[4];
char mes[2];
char dia[2];
char hora[2];
char min[2];
char seg[2];
};
struct fecha fecha_actual;



int   Btn_Habilitado[9];     //estado salida web hab/deshab
char  Btn_Texto[9][26];      //nemonico de la Salida para Web
char  Btn_Color[9][12];       //Color del Boton
unsigned int   SalidaTiempo[8];        //Tiempo de activación de la Salida

char  Entrada_Nemonico[16][12];         //nemonico de la entrada
int   EntradaWeb[8];          //Info del estado Entrada para Web (o,1,2)

char  est_alrm_web[75];       //estado de la Alarma para la web
char  ent_sal_web[20];        //estado de la Alarma para la web
char  col_alrm_web[10];       //estado de la Alarma para la web
//defino estructura de equipo





//definiciones generales
char TH[30],*th;                 // Buffer conversión DDMMAAHHMMSS
unsigned long dirxmemo,dirxalrm; // punteros para escribir en Xmem
char  tv;               //estado Tarj Valida "0" ó "1"
char  tv1;              //estado Tarj1 Valida   "0" ó "1"
char  tv2;              //estado Tarj2 Valida   "0" ó "1"
char  lr1,lr2;          //Led 1 o 2  Valida  "0" ó "1"
int   timeon1,timeoff1,timeon2,timeoff2;  //tiempos de encendidos on-off
char  beep_desactivar;             //estado activacion alarma "0" Desact  "1" Activada
char  alrm_armd;        //estado alarma   "0" Desarmada  "1" Armada
#define TimeAct  12     //Tiempo para activar Alarma
#define TimeDesact  8   //Tiempo para desactivar Alarma

unsigned int PORTA, *porta;
char  OUTPUTS[9],*outputs; //String que almacena estado entradas
int  INPUT,*input;        //Variable que almacena estado entradas optoacopladas
char  INPUTB,*inputb;      //Variable que almacena estado entradas directas
char  INPUTEE,*inputee;    //Variable que almacena estado para EE
int  INPUT_OLD,*input_old;//Variable que almacena estado anterior entradas
int  INPUT_COMP,*input_comp;//Variable que almacena estado anterior entradas
//char  INPUTS[9],*inputs;   //String que almacena estado entradas


int  ent_operativas;             //Seteo de entradas operativas
int  ent_tipo;            //Seteo tipo de entradas
int  set_alrm;            //Seteo de entradas de alarma
int  set_alrm_sleep;      //Seteo de entradas de alarma nocturna
char  z1;                  //Variable para activación Nocturna lector1
char  z2;                  //Variable para activación Nocturna lector2
char	sensor;					//elección de sensor usado
int   alrm_set_flag;       //estado entradas alarma activada
int   est_alrm;            //estado entradas alarmas actualizadas
int   alrm_disp;           //alarma disparada
int   alrm_zona;           //zona disparada
int   alrm_flag;           //zona informada
int   time_act;            //tiempo armado alarma
int   time_act1;           //tiempo armado alarma Lector1
int   time_act2;           //tiempo armado alarma Lector2
int   time_desact;         //tiempo espera desactivación alarma
int   time_ONluces;        //Tiempo de encendico Luces de acceso
int   S1,S2,S3,S4,S5,S6;      //Estado de las salidas
unsigned int   TOnS1,TOnS2,TOnS3,TOnS4,TOnS5,TOnS6;       //Tiempo de activación
unsigned long TS[8];    //Tiempo de activación
int   beep_on;                            //Indicador de Beep Sirena
unsigned long T_beep1,T_beep2,T_beep3;    //Time beep sirena

#define LARGO_BUFFTRJ  290          //Largo buffer tarjetas
int puntin_tjtas,puntout_tjtas;     //punteros de buffer tarjetas leídas
int puntin_event,puntout_event;     //punteros de buffer tarjetas leídas
int   energia;                      //indicador de corte de energía
enum {off=0,on=1};

//variables interrupcion TimerB WGD
char  estado_rec;          //estado de recepción lectores
char  temp_inter;          //temporización interdigito
//static char INTER, *inter; //Variable para controlar func interrupcion TimerB
char  TARJ[7];
int tempo;
char dir_puerto;           //puerto en acción   0=serie  1=ethernet
//variables estado alarma
char estado_alrm;          //estado de la alarma
char alrm_bat;             //Alarma de batería baja
char alrm_ee;              //Alarma de energía eléctrica
enum {reposo=0,set_armado=1, armado=2, disparada=3};  //estados posibles

/*************************
funciones de la página Web
*************************/
char ent_sal;              //indicador de entradas / salidas para web
char web;               //indicador de entradas / salidas para web
char SIM_OFF_OK;        //Indicador de apagado correcto
char bateria;           //Indicador de bateria off

char  CambioEnt;			//Indicador de cambio de Entradas
int 	Hab_CambioEnt;		//Indicador de boton cambio entrada habilitado
int   Hab_Alarmas;		//Indicador de botones alarma

//char  Col_CambioEnt[12];	//Color del cambio de Entradas
char  Text_CambioEnt[40];  //Texto del cambio de entradas
char  caja_de_texto[40];  //Texto del cambio de entradas


SSPEC_MIMETABLE_START
   SSPEC_MIME_FUNC(".shtml", "text/html", shtml_handler),
   SSPEC_MIME(".html", "text/html"),
// SSPEC_MIME(".gif", "image/gif"),
   SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END

int myb1(HttpState* state);
int myb2(HttpState* state);
int myb3(HttpState* state);
int myb4(HttpState* state);
int myb5(HttpState* state);
int myb6(HttpState* state);
int myb7(HttpState* state);
int myb8(HttpState* state);
int myb9(HttpState* state); //Alimentacion/ activar-desactivar
int myb0(HttpState* state);
int myb12(HttpState* state);

int activar;            // indicador de activar vía web
int   ID;               //Id de Virtual WatchDog
char INI; 					//Indicador de reinicio del soft




// directorio del server, funciones y variables de SSI
SSPEC_RESOURCETABLE_START
   SSPEC_RESOURCE_XMEMFILE("/", webcontrolbox_html),
   //SSPEC_RESOURCE_XMEMFILE("/webcontrolbox.shtml", webcontrolbox_html),
// SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif),
   //SSPEC_RESOURCE_XMEMFILE("/salidas.shtml", salidas_html),
   //SSPEC_RESOURCE_XMEMFILE("/entradas.shtml", entradas_html),
   //SSPEC_RESOURCE_XMEMFILE("/salidas1.shtml", salidas1_html),

   //boton habilitado/ deshabilitado
   SSPEC_RESOURCE_ROOTVAR("es1", &Btn_Habilitado[0], INT16,"%d"),
   SSPEC_RESOURCE_ROOTVAR("es2", &Btn_Habilitado[1], INT16,"%d"),
   SSPEC_RESOURCE_ROOTVAR("es3", &Btn_Habilitado[2], INT16,"%d"),
   SSPEC_RESOURCE_ROOTVAR("es4", &Btn_Habilitado[3], INT16,"%d"),
   SSPEC_RESOURCE_ROOTVAR("es5", &Btn_Habilitado[4], INT16,"%d"),
   SSPEC_RESOURCE_ROOTVAR("es6", &Btn_Habilitado[5], INT16,"%d"),
   SSPEC_RESOURCE_ROOTVAR("es7", &Btn_Habilitado[6], INT16,"%d"),
   SSPEC_RESOURCE_ROOTVAR("es8", &Btn_Habilitado[7], INT16,"%d"),
   SSPEC_RESOURCE_ROOTVAR("es9", &Btn_Habilitado[8], INT16,"%d"),
   //variable para mostrar boton elegido
   SSPEC_RESOURCE_ROOTVAR("d3",  &Hab_CambioEnt, INT16,"%d"),//hab <> entradas
   SSPEC_RESOURCE_ROOTVAR("d4",  &Hab_Alarmas, INT16,"%d"),//hab <> alarmas

   //acá agrego variable de entrada al rabbit desde web

  	SSPEC_RESOURCE_ROOTVAR("kx",	&caja_de_texto, PTR16,"%s"),//texto de caja de texto

   SSPEC_RESOURCE_ROOTVAR("a1",  &est_alrm_web, PTR16,"%s"),//texto superior
   SSPEC_RESOURCE_ROOTVAR("c1",  &col_alrm_web, PTR16,"%s"),//color superior

   SSPEC_RESOURCE_ROOTVAR("a2",  &ent_sal_web, PTR16,"%s"), //texto boton E/S
   SSPEC_RESOURCE_ROOTVAR("a3",  &Text_CambioEnt, PTR16,"%s"),//texto EE
   //SSPEC_RESOURCE_ROOTVAR("c3",  &Col_CambioEnt, PTR16,"%s"),//color EE

   //Color de fondo
   SSPEC_RESOURCE_ROOTVAR("s1",  &Btn_Color[0][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("s2",  &Btn_Color[1][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("s3",  &Btn_Color[2][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("s4",  &Btn_Color[3][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("s5",  &Btn_Color[4][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("s6",  &Btn_Color[5][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("s7",  &Btn_Color[6][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("s8",  &Btn_Color[7][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("s9",  &Btn_Color[8][0], PTR16,"%s"),
   //texto del Boton
   //SSPEC_RESOURCE_ROOTVAR("t1",  &Btn_Texto[0][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("t1",  &Btn_Texto[0][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("t2",  &Btn_Texto[1][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("t3",  &Btn_Texto[2][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("t4",  &Btn_Texto[3][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("t5",  &Btn_Texto[4][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("t6",  &Btn_Texto[5][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("t7",  &Btn_Texto[6][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("t8",  &Btn_Texto[7][0], PTR16,"%s"),
   SSPEC_RESOURCE_ROOTVAR("t9",  &Btn_Texto[8][0], PTR16,"%s"),


   SSPEC_RESOURCE_FUNCTION("/b1.cgi", myb1),
   SSPEC_RESOURCE_FUNCTION("/b2.cgi", myb2),
   SSPEC_RESOURCE_FUNCTION("/b3.cgi", myb3),
   SSPEC_RESOURCE_FUNCTION("/b4.cgi", myb4),
   SSPEC_RESOURCE_FUNCTION("/b5.cgi", myb5),
   SSPEC_RESOURCE_FUNCTION("/b6.cgi", myb6),
   SSPEC_RESOURCE_FUNCTION("/b7.cgi", myb7),
   SSPEC_RESOURCE_FUNCTION("/b8.cgi", myb8),
   SSPEC_RESOURCE_FUNCTION("/b9.cgi", myb9),
   SSPEC_RESOURCE_FUNCTION("/b0.cgi", myb0),
   SSPEC_RESOURCE_FUNCTION("/b12.cgi", myb12)


SSPEC_RESOURCETABLE_END

//////////////////////////////////////
// Conversión de tiempo en TH[] Caracteres en ASCII sin separadores

void conversion_web (unsigned long tiempo)
{
   struct tm   thetm;
   mktm(&thetm, tiempo);

   sprintf(TH,"&nbsp;%02d/%02d&nbsp;%02d:%02d:%02d",thetm.tm_mday,thetm.tm_mon,
   		 thetm.tm_hour,thetm.tm_min,thetm.tm_sec);

   /*
   TH[0]=0;
   itoa(thetm.tm_mday,CARACTER);
   if(strlen(CARACTER)==1){
   strcat(TH,"0");
   }
   strcat(TH,CARACTER);
   strcat(TH,"/");

   itoa(thetm.tm_mon,CARACTER);
   if(strlen(CARACTER)==1){
   strcat(TH,"0");
   }
   strcat(TH,CARACTER);
   strcat(TH,"&#32&#32");
   itoa(thetm.tm_hour,CARACTER);
   if(strlen(CARACTER)==1){
   strcat(TH,"0");
   }
   strcat(TH,CARACTER);
   strcat(TH,":");
   itoa(thetm.tm_min,CARACTER);
   if(strlen(CARACTER)==1){
   strcat(TH,"0");
   }
   strcat(TH,CARACTER);
   strcat(TH,":");
   itoa(thetm.tm_sec,CARACTER);
   if(strlen(CARACTER)==1){
   strcat(TH,"0");
   }
   strcat(TH,CARACTER);
   //strcat(TH,"-");
   strcat(TH,"\0");  */
}

void Cambiar_Salidas(int h)
{
   if (BIT(porta,h)){
      //printf("desactivar");
      res(porta,h);
   }else{
      set(porta,h);
      TS[h]=t0+SalidaTiempo[h];
      //printf("activar");
   }
   web=1;
}

tcp_Socket socket;
/*
myb1(HttpState* state)
{
   Cambiar_Salidas(0);
   cgi_redirectto(state,REDIRECTOK);
   return(0);
} */


myb12(HttpState* state)

{
   if (CambioEnt){
   	CambioEnt=0;
      strcpy(Text_CambioEnt,"Ir&#32a&#32Entradas&#32&#49-8");
   }else{
   	CambioEnt=1;
      strcpy(Text_CambioEnt,"Ir&#32a&#32Entradas&#32&#57-16");
   }
  	web=1;
   cgi_redirectto(state,REDIRECTOK);
   return(0);
}


/***********************************
Inicio Funciones Control Box
***********************************************/
/***************************************************************
 // initialize_timerb();
  set up the Timer b with (perclk/2/8) and set interup level 1
  set initial match 0000.
****************************************************************/
void initialize_timerb()
{
#asm nodebug
   ld    a,0x0bh           ; (perclk/2/8) and set interup level 3
ioi   ld    (TBCR),a
   ld    (TBCRShadow), a   ; load value into shadow register
   xor   a                 ; port e bit 1..7 inputs, 0 output
ioi ld   (TBL1R),a         ; set match 0x00
ioi ld   (TBM1R),a
   ld    a,0x03            ; enable Timer B and B1 Match interrup
ioi ld   (TBCSR),a
#endasm
}

///
///Comienzo funciones Lectores
///
/**********************************************
Interrupcion de lectura lectores Wiegand
estado_rec  -> estado recepcion anterior si es = sigo
temp_inter  -> Contador de interrupcion Controla que el Wgd ande Bien
INTER       -> Controla que la interrupción se ejecute
Seriali[0]  -> Data1
Seriali[1]  -> Data2
Seriali[2]  -> Data3
Seriali[3]  -> Datos Temporales
Seriali[4]  -> Contador de Bits parciales
Seriali[5]  -> Contador de Bytes
Seriali[6]  -> Contador de Bits totales (wgd 26) //rutina anterior Seriali[4]
Seriali[7]  -> Contador interpulso
Seriali[8]  -> Byte de datos anteriores

**********************************************/
#define PORT_L 4
#define D0_1   1
#define D1_1   0
#define D0_2   3
#define D1_2   2
/*
#define D0_1   7        //Hard Viejo
#define D1_1   6
#define D0_2   5
#define D1_2   4
*/
static char INTER, *inter;

#asm nodebug
timerb_isr::
   push af
   push bc
   push de
   push ix
   push  hl
   ioi   ld hl,(TBCMR)
;
; Interrupción cada 0,052 Ms Adaptada para Lector de Tarjetas
;

   ld    hl,(countinter)      ;Atualización para próximo Match 11
   ld    bc,004fh             ;6   es el que va en el lector
   add   hl,bc                ;2
   ld    (countinter),hl      ;15
   ld    a,h                  ;
   rl    a                    ;2
   rl    a
   rl    a
   rl    a
   rl    a
   rl    a
   and   a,0C0h
   ioi   ld (TBM1R), a        ; set up next B1 match (at timer=0100h)
   ld    a,l                  ; 01h bit-mangled to for TBM1R layout
   ioi   ld (TBL1R), a        ; NOTE:  you _need_ to reload the match

   //toma de datos del puerto
   ioe   ld a,(PORT_L)
   ld    d,a                     //guardo en registro 'd' para analisis

/***********Tratamiento de lectores **********/

   ld    ix,(serial1)
//Analisis de dato recibido
   bit   D0_1,d                 //analizo si viene un cero
   jr    nz,rxlect1_01          //Entrada High - no hay 'ceros'
   bit   D0_1,(ix+8)            //analizo si es repetido - mismo pulso
   jp    nz,rxlect1_06          //Si es el mismo voy a prox lector
   set   D0_1,(ix+8)              //cambio de estado lo guardo
   res   D1_1,(ix+8)
   ld    b,0
   jr    rxlect1_02

rxlect1_01:
   bit   D1_1,d               //analizo si viene un uno
   jr    nz,rxlect1_05        //Entrada High - no hay 'unos'- contador interpulso
   bit   D1_1,(ix+8)          //analizo si es repetido - mismo pulso
   jp    nz,rxlect1_06        //Si es el mismo voy a prox lector
   set   D1_1,(ix+8)          //cambio de estado lo guardo
   res   D0_1,(ix+8)
   ld    b,1

rxlect1_02:
   ld    (ix+7),0                   //reseteo contador de tiempos
//almaceno datos
   inc   (ix+6)                     //incremento contador de bits
   ld    a,(ix+6)
   cp    1
   jr    nz,rxlect1_03               //es <> a 1 me voy a recibir datos
   xor   a
   ld    (ix),a
   ld    (ix+1),a
   ld    (ix+2),a
   ld    (ix+3),a
   ld    (ix+4),a
   ld    (ix+5),a
   jr    rxlect1_06   //Voy a proximo  lector

rxlect1_03:          //selecciono si es un 'uno' o un 'cero'
   bit   0,b
   jr    nz,rxlect1_04
   scf

rxlect1_04:
   rl    (ix+3)
   inc   (ix+4)      //contador de bits parciales
   ld    a,(ix+4)
   cp    8
   jr    nz,rxlect1_06
   ld    a,(ix+5)
   ld    c,a
   ld    b,00h
   ld    hl,ix
   add   hl,bc
   ld    a,(ix+3)    //cargo dato parcial
   ld    (hl),a      //almaceno dato parcial
   inc   (ix+5)      //Contador de Bytes
   xor   a
   ld    (ix+3),a
   ld    (ix+4),a
   jr    rxlect1_06

rxlect1_05:
   ld    (ix+8),0
   ld    a,(ix+6)    //verifico contador de bits recibidos <> 0
   cp    0
   jr    z,rxlect1_06      //voy a próximo lector
   inc   (ix+7)
   bit   6,(ix+7)
   jr    z,rxlect1_06      //voy a próximo lector
   ld    (ix+6),26   //

rxlect1_06:

/***********Segundo Lector       ***************/
   ld    ix,(serial2)
//Analisis de dato recibido
   bit   D0_2,d
   jr    nz,rxlect2_01          //Entrada High - no hay 'ceros'
   bit   D0_2,(ix+8)
   jp    nz,rxlect2_06
   set   D0_2,(ix+8)              //cambio de estado lo guardo
   res   D1_2,(ix+8)
   ld    b,0
   jr    rxlect2_02

rxlect2_01:
   bit   D1_2,d
   jr    nz,rxlect2_05    //Entrada High - no hay 'unos'
   bit   D1_2,(ix+8)
   jp    nz,rxlect2_06
   set   D1_2,(ix+8)              //cambio de estado lo guardo
   res   D0_2,(ix+8)
   ld    b,1

rxlect2_02:
   ld    (ix+7),0             //reseteo contador de tiempos
//almaceno datos
   inc   (ix+6)                     //incremento contador de bits
   ld    a,(ix+6)
   cp    1
   jr    nz,rxlect2_03                //es =/ a 1 me voy a recibir datos
   xor   a
   ld    (ix),a
   ld    (ix+1),a
   ld    (ix+2),a
   ld    (ix+3),a
   ld    (ix+4),a
   ld    (ix+5),a
   jr    rxlect2_06

rxlect2_03:          //selecciono si es un 'uno' o un 'cero'
   bit   0,b
   jr    nz,rxlect2_04
   scf

rxlect2_04:
   rl    (ix+3)
   inc   (ix+4)      //contador de bits parciales
   ld    a,(ix+4)
   cp    8
   jr    nz,rxlect2_06
   ld    a,(ix+5)
   ld    c,a
   ld    b,00h
   ld    hl,ix
   add   hl,bc
   ld    a,(ix+3)    //cargo dato parcial
   ld    (hl),a      //almaceno dato parcial
   inc   (ix+5)      //Contador de Bytes
   xor   a
   ld    (ix+3),a
   ld    (ix+4),a
   jr    rxlect2_06

rxlect2_05:

   ld    (ix+8),0
   ld    a,(ix+6)    //verifico contador de bits recibidos <> 0
   cp    0
   jr    z,rxlect2_06      //si es igual a cero no hago nada
   inc   (ix+7)
   bit   6,(ix+7)
   jr    z,rxlect2_06
   ld    (ix+6),26   //

rxlect2_06:
//rxlect4_06:
/*******Fin Proceso   **************************/
   xor   a
   ld    (INTER),a
   pop hl
   pop ix
   pop de
   pop bc
   ioi   ld a, (TBCSR)        ; load B1, B2 interrupt flags (clears flag)
   pop   af
   ipres
   ret
#endasm

/***************************************************************
initialize_ports();
set up the parallel ports so PORTA are outputs and PORTE bit 0
is and output.
****************************************************************/
void initialize_ports()
{
#asm nodebug
   ld    a,0x84            ; Port A all outputs
ioi ld   (SPCR),a
   ld    (SPCRShadow), a   ; load value into shadow register
//seteo port E
   ld    hl,PEDDRShadow
   ld    de,PEDDR
   set   4,(hl)            ;E4 salida
   set   7,(hl)            ;E7 salida
ioi   ldd
   ld    hl,PEFRShadow
   ld    de,PEFR
   res   4,(hl)            ;E4 salida
   res   7,(hl)            ;E7 salida
ioi   ldd
   xor   a
ioi ld   (PECR),a
   ld    (PECRShadow), a
//seteo port D
// ld    hl,PDDDRShadow
//   ld    de,PDDDR
//   set 3,(hl)            ;D3 salida
//ioi ldd
#endasm
}

/************************************************
Definición de la estructura de las Activaciones
************************************************/
typedef struct{
char  nemonico[15];      //nemonico de la activacion
char  descripcion[25];   //descripcion de la activacion
char  entrada0;      //Entrada 0 	1/0
char  entrada1;      //Entrada 1 	1/0
char  entrada2;      //Entrada 2 	1/0
char  entrada3;      //Entrada 3 	1/0
char  entrada4;      //Entrada 4 	1/0
char  entrada5;      //Entrada 5 	1/0
char  entrada6;      //Entrada 6		1/0
char  entrada7;      //Entrada 7 	1/0
char  entrada8;      //Entrada 8 	1/0
char  entrada9;      //Entrada 9 	1/0
char  entrada10;     //Entrada 10 	1/0
char  entrada11;     //Entrada 11 	1/0
char  entrada12;     //Entrada 12 	1/0
char  entrada13;     //Entrada 13 	1/0
char  entrada14;     //Entrada 14 	1/0
char  entrada15;     //Entrada 15 	1/0
int	programacion;  //máscara de las entradas utilizadas
char  sms;  			//Envia o no SMS S/N
char 	ring;				//realiza el ring en alarma
char	sirena;    		//Tipo de sirena utilizada
}Activacion;                 //Bytes de Entrada =   61

Activacion myActivacion;      //buffer descripcion tipos activacion
const static Activacion myActivacion_default =
				{"Total","Activacion Total",1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0x0000,'S','S','0'};

int myMasc_activacion[4];		//Máscara de Activación
int myNemo_activacion[4][16];		//Nemonico de Activación
int myDesc_activacion[4][26];		//Descripcion de Activación

unsigned long t_activacion;     // usado para fecha y hora de activacion
char flag_activar;			//indicador de activacion Web en marcha
int 	tipo_alarma;			//indicador de tipo de alarma activada
/************************************************
 Función que Transmite la configuración de la Alarma.
comando !staA1
************************************************/
nodebug void tx_alarma()
{
      bsqold=answ+3;
      if (*bsqold!='A')
         goto error_alineamiento;
      bsq=answ+4;
      i=*bsq-49;
      if (i<0||i>3)
         goto error_alineamiento;
      readUserBlock(&myActivacion,ALM_HAB+(i*sizeof(Activacion)),sizeof(Activacion));   //Recupero Activaciones
      //memcpy(&myActivacion,buffer_alm +(i*sizeof(Activacion)),sizeof(Activacion));
      sprintf(STRING_TX,"A%d,%s,%s,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%d%s",i+1,
            myActivacion.nemonico,myActivacion.descripcion,myActivacion.entrada0,
            myActivacion.entrada1,myActivacion.entrada2,myActivacion.entrada3,
            myActivacion.entrada4,myActivacion.entrada5,myActivacion.entrada6,
            myActivacion.entrada7,myActivacion.entrada8,myActivacion.entrada9,
            myActivacion.entrada10,myActivacion.entrada11,myActivacion.entrada12,
            myActivacion.entrada13,myActivacion.entrada14,myActivacion.entrada15,
            myActivacion.sms,myActivacion.ring,myActivacion.sirena,FIN);
   return;
error_alineamiento:
   strcpy(STRING_TX,"Error Alineamiento LF\r\n");      //serCputs(STRING);
}

/************************************************
Funcion que recepciona el seteo de la Activacion
comando !alaA1,Alrm,Alarma total,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,S,S,0\n
************************************************/

nodebug void rx_alarma()
{
   bsqold=answ+3;
   //printf ("seteo Alarma:\t%s\r\n",answ);
  /* if(!strchr(bsqold,'\n')){        //busco el LF final
      strcpy(STRING_TX,"Error Alineamiento Entradas\r\n");    //mensaje de error
      return;
   }*/
   if(bsq=strtok(bsqold,",")){
      if (*(bsq)!='A'){               //controlo J
         strcpy(STRING_TX,"Error Falta 'A'\r\n");    //mensaje de error
         return;
         }
   }
   bsqold=bsq+1;
   i=atoi(bsqold);
   i=i-1;
   //printf("Indice Alarma%d\r\n",i);

   myActivacion.programacion=0x0000;   //pongo todas las entradas desasociadas
   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      strcpy(myActivacion.nemonico,bsq);
   }
   if (bsq=strtok(NULL,",")){      //Descripcion Entrada
     strcpy(myActivacion.descripcion,bsq);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 1 Habilitada
      myActivacion.entrada0=*bsq;
      if (myActivacion.entrada0>48)
      	SET(&myActivacion.programacion,0);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 2 Habilitada
      myActivacion.entrada1=*bsq;
      if (myActivacion.entrada1>48)
      	SET(&myActivacion.programacion,1);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 3 Habilitada
      myActivacion.entrada2=*bsq;
      if (myActivacion.entrada2>48)
      	SET(&myActivacion.programacion,2);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 4 Habilitada
      myActivacion.entrada3=*bsq;
      if (myActivacion.entrada3>48)
      	SET(&myActivacion.programacion,3);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 5 Habilitada
      myActivacion.entrada4=*bsq;
      if (myActivacion.entrada4>48)
      	SET(&myActivacion.programacion,4);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 6 Habilitada
      myActivacion.entrada5=*bsq;
      if (myActivacion.entrada5>48)
      	SET(&myActivacion.programacion,5);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 7 Habilitada
      myActivacion.entrada6=*bsq;
      if (myActivacion.entrada6>48)
      	SET(&myActivacion.programacion,6);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 8 Habilitada
      myActivacion.entrada7=*bsq;
      if (myActivacion.entrada7>48)
      	SET(&myActivacion.programacion,7);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 9 Habilitada
      myActivacion.entrada8=*bsq;
      if (myActivacion.entrada8>48)
      	SET(&myActivacion.programacion,8);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 10 Habilitada
      myActivacion.entrada9=*bsq;
      if (myActivacion.entrada9>48)
      	SET(&myActivacion.programacion,9);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 11 Habilitada
      myActivacion.entrada10=*bsq;
      if (myActivacion.entrada10>48)
      	SET(&myActivacion.programacion,10);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 12 Habilitada
      myActivacion.entrada11=*bsq;
      if (myActivacion.entrada11>48)
      	SET(&myActivacion.programacion,11);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 13 Habilitada
      myActivacion.entrada12=*bsq;
      if (myActivacion.entrada12>48)
      	SET(&myActivacion.programacion,12);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 14 Habilitada
      myActivacion.entrada13=*bsq;
      if (myActivacion.entrada13>48)
      	SET(&myActivacion.programacion,13);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 15 Habilitada
      myActivacion.entrada14=*bsq;
      if (myActivacion.entrada14>48)
      	SET(&myActivacion.programacion,14);
   }
   if (bsq=strtok(NULL,",")){      //Entrada 16 Habilitada
      myActivacion.entrada15=*bsq;
      if (myActivacion.entrada15>48)
      	SET(&myActivacion.programacion,15);
   }
   if (bsq=strtok(NULL,",")){      //SMS Habilitado
      myActivacion.sms=*bsq;
   }
   if (bsq=strtok(NULL,",")){      //Ring Habilitado
      myActivacion.ring=*bsq;
   }
   if (bsq=strtok(NULL,"\n")){      //Tipo de Sirena Utilizado
      myActivacion.sirena=atoi(bsq);
      memcpy(buffer_alm+(i*sizeof(Activacion)),&myActivacion, sizeof(Activacion));
      writeUserBlock(ALM_HAB+(i*sizeof(Activacion)),&myActivacion, sizeof(Activacion));
      myMasc_activacion[i] = myActivacion.programacion;
      strcpy(myNemo_activacion[i],myActivacion.nemonico);
      //printf( "Nemonico %s\r\n", myNemo_activacion[i]);
      strcpy(myDesc_activacion[i],myActivacion.descripcion);
      strcpy(STRING_TX,"OK.La Activacion se ha Modificado Correctamente");
   }else{
      strcpy(STRING_TX,"Error Alineamiento LF\r\n");
      readUserBlock(&Buffer_alm,ALM_HAB,249);   //Recupero Activaciones
   }
}


/************************************************
Defino estructura de equipo
************************************************/
typedef struct{
int  ent_operativas;           //Entradas Operativas(Están Conectadas)
int  ent_no_asociada;          //Entradas habilitadas para la alarma
int  entalrmsleep;     //Entradas habilitadas para la alarma sleep
int  enttipo;          //Tipo de Entradas NA/NC
char  serie[7];        //Número de serie del equipo
char  descripcion[11];  //Descripción de instalación
unsigned int   tON_luces;        //tiempo encendido luces con un ring
unsigned int   t_activar;        //Tiempo de espera Activación Alarma
unsigned int   t_desactivar;     //Tiempo de espera Activación Alarma
char  cia;              //Compañía de Celular
char  nro_celular[11]; //Numero de celular instalado
char  ip[16];          //IP del equipo
char  gateway[16];     //Gateway del equipo
char  programador[6];  //Identificador de programa
char  tipo_lector;     //Lector 1 Interior/exterior
char  hab_ping;         //habilito ping de control
char  hab_aviso;       //Habilito aviso de alarma no activada en det Horario
char  hora_aviso[5];    //horario de aviso de alarma no habilitada
char  hora_ini_entsal[5];  //horario inicio validez  E/S
char  hora_fin_entsal[5];  //horario fin validez  E/S
}Hard;
Hard myHard;
int  ent_no_asociada;
const static Hard myHard_default = {0xFFFF,0xFFFF,0x0000,0x0000,"000001","ControlBox",
											5,12,12,'P',"3416403413",DEFAULT_SOURCEIP,
                                  DEFAULT_GATEWAY, PROGRAMADOR,'I','N',
                                  'N',"2100","2100","0800"};


/*************************************************************************
Funcion que transmite el seteo de los datos del equipo
comando recibido !stj
numeserie(6Ch),descrip(10Ch),tonLuces(2ch),Tact(2ch),Tdesac(2ch),cia(1ch)
*************************************************************************/
char T,P;
nodebug void tx_equip()
{

  if (myHard.tipo_lector==1){
      T='I';
  }else{
      T='E';
  }
  if (myHard.hab_ping==1){
      P='S';
  }else{
      P='N';
  }
  sprintf(STRING_TX,"J,%d,%d,%d,%c,%c,%s,%s,%s,%s,%s,%c,%c,%s,%s,%s%s",myHard.t_activar,
            myHard.t_desactivar,myHard.tON_luces,T,myHard.cia,
            myHard.serie,myHard.descripcion,myHard.nro_celular,myHard.ip,
            myHard.gateway,P,myHard.hab_aviso,
            myHard.hora_aviso,myHard.hora_ini_entsal,myHard.hora_fin_entsal,FIN);
  //printf(STRING_TX);
}


/************************************************
Funcion que recepciona el seteo de los datos del equipo
!alj numeserie(6Ch),descrip(10Ch),tonLuces(2ch),Tact(2ch),Tdesac(2ch),cia(1ch)
************************************************/
nodebug int rx_equip()
{
   bsqold=answ+3;
   strcpy(STRING_TX,"Error Alineamiento Equipo\r\n");    //mensaje de error
   j=1;
   if(!strchr(bsqold,'\n')){        //busco el LF final
      return(0);
   }
   if(bsq=strtok(bsqold,",")){
      if (*bsq!='J'){               //controlo J
         strcpy(STRING_TX,"Error Falta 'J'\r\n");    //mensaje de error
         return(0);
         }
   }
   if (bsq=strtok(NULL,",")){      //Tiempo de activación
      myHard.t_activar=atoi(bsq);
   }
   if (bsq=strtok(NULL,",")){      //Tiempo de desactivación
      myHard.t_desactivar=atoi(bsq);
   }
   if (bsq=strtok(NULL,",")){      //Tiempo de ON luces default
      myHard.tON_luces=atoi(bsq);
   }
   if (bsq=strtok(NULL,",")){      //Tipo lector
      if (*bsq=='I'){              //Interior
         myHard.tipo_lector=1;
      }else{
         myHard.tipo_lector=0;
      }
   }
   if (bsq=strtok(NULL,",")){      //compania del celular
      myHard.cia=*bsq;
   }
   if (bsq=strtok(NULL,",")){      //numero serie equipo
      strcpy(myHard.serie,bsq);
      myHard.serie[6]=0;
   }
   if (bsq=strtok(NULL,",")){      //descripcion equipo
      strcpy(myHard.descripcion,bsq);
      myHard.descripcion[10]=0;
   }
   if (bsq=strtok(NULL,",")){      //nro de celular
      strcpy(myHard.nro_celular,bsq);
      myHard.nro_celular[10]=0;
   }
   if (bsq=strtok(NULL,",")){      //IP equipo
      strcpy(&SALIDA,bsq);
      if(!strstr(SALIDA,myHard.ip)){
       strcpy(myHard.ip,bsq);
       j=2;
      }
   }
   if (bsq=strtok(NULL,",")){      //Gateway sistema
      strcpy(myHard.gateway,bsq);
   }
   if (bsq=strtok(NULL,",")){      //Habilito Ping
      if (*bsq=='S'){              //Si
         myHard.hab_ping=1;
      }else{
         myHard.hab_ping=0;
      }
   }
   if (bsq=strtok(NULL,",")){      //Habilito Aviso activacion
      myHard.hab_aviso=*bsq;
   }
   if (bsq=strtok(NULL,",")){      //Hora activacion
      strcpy(myHard.hora_aviso,bsq);
      myHard.hora_aviso[4]=0;
   }
   if (bsq=strtok(NULL,",")){      //Hora Inicio E/S
      strcpy(myHard.hora_ini_entsal,bsq);
      myHard.hora_ini_entsal[4]=0;
   }
   if (bsq=strtok(NULL,"\n")){      //Hora FIN E/S
      strcpy(myHard.hora_fin_entsal,bsq);
      myHard.hora_fin_entsal[4]=0;
   }else{
      readUserBlock(&myHard,EQP_HAB,sizeof(myHard));   //recupero programacion
      return(0);
   }
//grabo la nueva programacion
   writeUserBlock(EQP_HAB,&myHard, sizeof(myHard)); //grabo en flash
   time_act1=myHard.t_activar;    //tiempo de activacion alarma
   time_act2=myHard.t_desactivar;    //tiempo de activacion alarma
   strcpy(STRING_TX,"OK.El Equipo se ha Modificado Correctamente\r\n");
   return(j);
}

/************************************************
Definición de la estructura de las entradas
************************************************/
typedef struct{
char  numero;              // numero de entrada
char  asociada;            //habilitada para alarma
char  operativa;           //operativa para utilizar
char  tipo;           		//tipo de entrada NA/NC (A/C)
int   time_alarma;         //Tiempo de espera para alarma
char  nemonico[10];
char  descripcion[25];
}Entrada;                 //Bytes de Entrada =   41

Entrada myEntrada;      //buffer descripcion entradas SMS(On/Off)
const static Entrada myEntrada_default = {"1","S","S","A",10,"Ent","Entrada"};
static char Buffer_ent[900], *buffer_ent;   // Buffer de Entradas
/************************************************
 Función que Transmite la configuración entradas.
comando !steE1\n
************************************************/
nodebug void tx_ent()
{
      bsqold=answ+3;
      if (*bsqold!='E')
         goto error_alineamiento;
      bsqold=answ+4;
      bsq=strchr(bsqold,'\n');
   	if(bsq){
   		*bsq='\0';
      //printf("no hay Error%s\r\n",bsqold);
      }else{
         strcpy(STRING_TX,"Error Alineamiento Entradas\r\n");    //mensaje de error
      	return;
      }
      i=atoi(bsqold);
      i=i-1;
      //printf("Indice%d\r\n",i);
      if (i<0||i>15)
         goto error_alineamiento;
      memcpy(&myEntrada,buffer_ent+(i*sizeof(Entrada)),sizeof(Entrada));

      sprintf(STRING_TX,"E%02d,%c,%c,%c,%s,%s,%d%s",myEntrada.numero,
      		myEntrada.asociada, myEntrada.operativa, myEntrada.tipo,
            myEntrada.nemonico,myEntrada.descripcion,myEntrada.time_alarma,FIN);
  		//  printf("Tx Entradas:\t%s\r\n",STRING_TX);
   return;
error_alineamiento:
   strcpy(STRING_TX,"Error Alineamiento LF\r\n");      //serCputs(STRING);
}

/************************************************
Funcion que recepciona el seteo de todas las entradas
comando !stlE0
************************************************/

nodebug void rx_ent()
{
   bsqold=answ+3;
   if(bsq=strtok(bsqold,",")){
      if (*(bsq)!='E'){               //controlo J
         strcpy(STRING_TX,"Error Falta 'E'\r\n");    //mensaje de error
         return;
         }
   }
   bsqold=bsq+1;
   i=atoi(bsqold);
   myEntrada.numero=i;
   i=i-1;
   if (bsq=strtok(NULL,",")){      //Entrada asociada a Seguridad
      if (*bsq=='N'){
         SET(&ent_no_asociada,i);      //Asociada a Alarma
         //printf("Asociada\r\n");
      }else{
         RES(&ent_no_asociada,i);      //NO Asociada a Alarma
         //printf("NO Asociada\r\n");
      }
      myEntrada.asociada=*bsq;	//Guardo si está asociada
   }
   if (bsq=strtok(NULL,",")){      //Entrada Operativa
      if (*bsq=='S'){
         SET(&ent_operativas,i);   //Entrada Operativa
         //printf("Habilitada\r\n");
      }else{
         RES(&ent_operativas,i);   //Entrada NO Operativa
         //printf("NO Habilitada\r\n");
      }
      myEntrada.operativa=*bsq;	//Guardo si está Operativa
   }
   if (bsq=strtok(NULL,",")){      //Tipo Entrada
      if (*bsq=='A'){
         SET(&ent_tipo,i);   //Entrada Normal Abierto
         //printf("Normal Abierto\r\n");
      }else{
         RES(&ent_tipo,i);   //Entrada Normal Cerrada
         //printf("Normal cerrado\r\n");
      }
      myEntrada.tipo=*bsq;	//Guardo el tipo
   }
   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      strcpy(myEntrada.nemonico,bsq);
      //printf("nemonico\t %s\r\n",myEntrada.nemonico);
   }

   if (bsq=strtok(NULL,",")){      //Descripcion Entrada
      strcpy(myEntrada.descripcion,bsq);
      //printf("descripcion\t %s\r\n",myEntrada.descripcion);
   }
  if (bsq=strtok(NULL,"\n")){      //Gateway sistema
      myEntrada.time_alarma=atoi(bsq);
      //printf("tiempo alarma\t %d\r\n",myEntrada.time_alarma);
      //Guardo seteo entradas
      memcpy(buffer_ent+(i*sizeof(Entrada)),&myEntrada, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(i*sizeof(Entrada)),&myEntrada,sizeof(Entrada));
         /*****guardo seteo entradas en flash Equipo ***/
      myHard.ent_no_asociada=ent_no_asociada;
      //myHard.entalrmsleep=set_alrm_sleep;
      myHard.ent_operativas=ent_operativas;     //entradas habilitadas
      myHard.enttipo=ent_tipo;      //tipo de entradas NA/NC
      strcpy(Entrada_Nemonico[i],myEntrada.nemonico);
      writeUserBlock(EQP_HAB,&myHard, sizeof(myHard));
      strcpy(STRING_TX,"OK.La Entrada se ha Modificado Correctamente");
      web=1;
   }else{
      strcpy(STRING_TX,"Error Alineamiento LF\r\n");
      readUserBlock(Buffer_ent,ENT_HAB,800);         //Recupero Entradas
      readUserBlock(&myHard,EQP_HAB,sizeof(myHard));  //recupèro prog de entradas
      ent_operativas=myHard.ent_operativas;     //entradas habilitadas
      ent_no_asociada=myHard.ent_no_asociada;   //entradas de alarma
      set_alrm_sleep=myHard.entalrmsleep; //entradas de alarma noct
      ent_tipo=myHard.enttipo;      //tipo de entradas NA/NC
   }
}

/************************************************
Definición de la estructura de las entradas/salidas
************************************************/
typedef struct{
int   time0;         //Tiempo de activacion salida0
int   time1;         //Tiempo de activacion salida1
int   time2;         //Tiempo de activacion salida2
int   time3;         //Tiempo de activacion salida3
int   time4;         //Tiempo de activacion salida4
int   time5;         //Tiempo de activacion salida5
int   time6;         //Tiempo de activacion salida6
int   time7;         //Tiempo de activacion salida7
char  programacion;  //Byte de mascara
char 	horario;			//Actua por horario o no
char	activacion;    //Actua pòr activacion o no
}Entrada_Salida;                 //Bytes de Entrada =   39

Entrada_Salida myEntrada_Salida;      //buffer descripcion entradas SMS(On/Off)
const static Entrada_Salida myEntrada_Salida_default =
				{0,0,0,0,0,0,0,0,0x00,'H','D'};


/************************************************
 Función que Transmite la configuración Entrada/Salidas.
comando !strE1
************************************************/
nodebug void tx_ent_sal()
{
      bsqold=answ+3;
      if (*bsqold!='R')
         goto error_alineamiento;
      bsqold=answ+4;
      bsq=strchr(bsqold,'\n');
   	if(bsq){
   		*bsq='\0';
      }else{
         strcpy(STRING_TX,"Error Alineamiento Entradas/Salidas\r\n");    //mensaje de error
      	return;
      }
      i=atoi(bsqold);
      i=i-1;
      if (i<0||i>15)
         goto error_alineamiento;
      memcpy(&myEntrada_Salida,buffer_ens+(i*sizeof(Entrada_Salida)),sizeof(Entrada_Salida));
      sprintf(STRING_TX,"R%02d,%d,%d,%d,%d,%d,%d,%d,%d,%c,%c%s",i+1,
            myEntrada_Salida.time0,myEntrada_Salida.time1,myEntrada_Salida.time2,
            myEntrada_Salida.time3,myEntrada_Salida.time4,myEntrada_Salida.time5,
            myEntrada_Salida.time6,myEntrada_Salida.time7,myEntrada_Salida.horario,
            myEntrada_Salida.activacion,FIN);
   return;
error_alineamiento:
   strcpy(STRING_TX,"Error Alineamiento\n\r");      //serCputs(STRING);
}

/************************************************
Funcion que recepciona el seteo de Entradas/Salidas
comando !alrE1
************************************************/

nodebug void rx_ent_sal()
{
   bsqold=answ+3;
   //printf ("seteo SMS_entradas:\t%s\r\n",answ);
  /* if(!strchr(bsqold,'\n')){        //busco el LF final
      strcpy(STRING_TX,"Error Alineamiento Entradas\r\n");    //mensaje de error
      return;
   }*/
   if(bsq=strtok(bsqold,",")){
      if (*(bsq)!='R'){               //controlo J
         strcpy(STRING_TX,"Error Falta 'R'\r\n");    //mensaje de error
         return;
         }
   }
   bsqold=bsq+1;
   i=atoi(bsqold);
   j=i-1;
   //printf("Indice%d\r\n",i);

   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      i=atoi(bsq);
      myEntrada_Salida.time0=i;
   }
   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      i=atoi(bsq);
      myEntrada_Salida.time1=i;
   }
   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      i=atoi(bsq);
      myEntrada_Salida.time2=i;
   }
   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      i=atoi(bsq);
      myEntrada_Salida.time3=i;
   }
   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      i=atoi(bsq);
      myEntrada_Salida.time4=i;
   }
   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      i=atoi(bsq);
      myEntrada_Salida.time5=i;
   }
   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      i=atoi(bsq);
      myEntrada_Salida.time6=i;
   }
   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      i=atoi(bsq);
      myEntrada_Salida.time7=i;
   }
   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      myEntrada_Salida.horario=*bsq;
   }
   if (bsq=strtok(NULL,"\n")){      //Gateway sistema
      myEntrada_Salida.activacion=*bsq;
      memcpy(buffer_ens+(j*sizeof(Entrada_Salida)),&myEntrada_Salida, sizeof(Entrada_Salida));
      writeUserBlock(ENS_HAB+(j*sizeof(Entrada_Salida)),&myEntrada_Salida, sizeof(Entrada_Salida));
      strcpy(STRING_TX,"OK.Las Entrada/Salidas se ha Modificado Correctamente");

   }else{
      strcpy(STRING_TX,"Error Alineamiento LF\r\n");
      readUserBlock(Buffer_ens,ENS_HAB,349);   //Recupero SMS_Entradas
   }
}


/************************************************
Definición de la estructura de las Macros
************************************************/
typedef struct{
int   indice;
char  estado;
char  nemonico[10];
char  descripcion[25];
unsigned int   t0;
unsigned int   t1;
unsigned int   t2;
unsigned int   t3;
unsigned int   t4;
unsigned int   t5;
unsigned int   t6;
unsigned int   t7;
}MACRO;
MACRO MyMacro;
const static MACRO myMacro_default = {0,'S',"MAC1","MACRO UNO",60,0,0,0,0,0,0,0};
const static MACRO myMacro_default1 = {1,'S',"EMERG","LUCES EMERGENCIA",0,1200,0,0,0,0,0,0};

/****************************************
Función que transmite las macros
!stmM1
****************************************/
nodebug void tx_mac()
{
   bsqold=answ+3;
   if (*bsqold!='M')
      goto error_alineamiento;
   bsq=answ+4;
   i=*bsq-49;
   if (i<0||i>1)
      goto error_alineamiento;
   memcpy(&MyMacro,Buffer_mac+(i*sizeof(MyMacro)),sizeof(MyMacro));
   sprintf(STRING_TX,"M%d,S,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d%s",i+1,MyMacro.nemonico,
   			MyMacro.descripcion, MyMacro.t0, MyMacro.t1, MyMacro.t2, MyMacro.t3,
            MyMacro.t4, MyMacro.t5,MyMacro.t5, MyMacro.t6,FIN);
   return;
error_alineamiento:
   strcpy(STRING_TX,"Er. Ali. Solicitud MAC\r\n");
}

/********************************************
Funcion que recepciona el seteo de las MAC
!stmM1,
********************************************/
nodebug void rx_mac()
{
   bsqold=answ+3;
   if (*bsqold!='M')
      goto error_alineamiento;
   bsq=answ+4;
   //printf("RX Macro: %s\r\n",bsq);

   i=*bsq-49;
   if (i<0||i>7)
      goto error_alineamiento;
   bsq=strchr(bsqold,',');    //busco la primera coma
   if (bsq==0) goto error_alineamiento;
   bsqold=bsq+1;
   bsq=strchr(bsqold,',');    //busco la segunda coma
   if (bsq==0) goto error_alineamiento;
   bsqold=bsq+1;
   bsq=strchr(bsqold,',');    //busco la tercera coma
   if (bsq==0) goto error_alineamiento;
   j=bsq-bsqold;              //largo dato
   if (j==0){
      MyMacro.nemonico[j]=0;
   }else{
      if(j>9)
         j=9;
      strncpy(MyMacro.nemonico,bsqold,j);
      MyMacro.nemonico[j]=0;
   }
   bsqold=bsq+1;
   bsq=strchr(bsqold,',');    //busco la cuarta coma
   if (bsq==0) goto error_alineamiento;
   j=bsq-bsqold;
   if (j==0){
      MyMacro.descripcion[j]=0;
   }else{
      if(j>24)
         j=24;
      strncpy(MyMacro.descripcion,bsqold,j);
      MyMacro.descripcion[j]=0;
   }
   bsq=bsq+1;
   bsqold=bsq;

   bsq=strchr(bsqold,',');   //busco la quinta coma
   if (bsq==0) goto error_alineamiento;
   j=bsq-bsqold;
   if (j==0){
      MyMacro.t0=0;
   }else{
      if(j>5)
         j=5;
      strncpy(CARACTER,bsqold,j);
      CARACTER[j]=0;
      MyMacro.t0=atoi(CARACTER);
   }

   bsq=bsq+1;
   bsqold=bsq;
   bsq=strchr(bsqold,',');   //busco la sexta coma
   if (bsq==0) goto error_alineamiento;
   j=bsq-bsqold;
   if (j==0){
      MyMacro.t1=0;
   }else{
      if(j>5)
         j=5;
      strncpy(CARACTER,bsqold,j);
      CARACTER[j]=0;
      MyMacro.t1=atoi(CARACTER);
   }
   bsq=bsq+1;
   bsqold=bsq;
   bsq=strchr(bsqold,',');   //busco la septima coma
   if (bsq==0) goto error_alineamiento;
   j=bsq-bsqold;
   if (j==0){
      MyMacro.t2=0;
   }else{
      if(j>5)
         j=5;
      strncpy(CARACTER,bsqold,j);
      CARACTER[j]=0;
      MyMacro.t2=atoi(CARACTER);
   }
   bsq=bsq+1;
   bsqold=bsq;
   bsq=strchr(bsqold,',');   //busco la octava coma
   if (bsq==0) goto error_alineamiento;
   j=bsq-bsqold;
   if (j==0){
      MyMacro.t3=0;
   }else{
      if(j>5)
         j=5;
      strncpy(CARACTER,bsqold,j);
      CARACTER[j]=0;
      MyMacro.t3=atoi(CARACTER);
   }
   bsq=bsq+1;
   bsqold=bsq;
   bsq=strchr(bsqold,',');   //busco la novena coma
   if (bsq==0) goto error_alineamiento;
   j=bsq-bsqold;
   if (j==0){
      MyMacro.t4=0;
   }else{
      if(j>5)
         j=5;
      strncpy(CARACTER,bsqold,j);
      CARACTER[j]=0;
      MyMacro.t4=atoi(CARACTER);
   }
   bsq=bsq+1;
   bsqold=bsq;
   bsq=strchr(bsqold,',');   //busco la decima coma
   if (bsq==0) goto error_alineamiento;
   j=bsq-bsqold;
   if (j==0){
      MyMacro.t5=0;
   }else{
      if(j>5)
         j=5;
   strncpy(CARACTER,bsqold,j);
   CARACTER[j]=0;
   MyMacro.t5=atoi(CARACTER);
   }
   bsq=bsq+1;
   bsqold=bsq;
   bsq=strchr(bsqold,',');   //busco la undecima coma
   if (bsq==0) goto error_alineamiento;
   j=bsq-bsqold;
   if (j==0){
      MyMacro.t6=0;
   }else{
      if(j>5)
         j=5;
      strncpy(CARACTER,bsqold,j);
      CARACTER[j]=0;
      MyMacro.t6=atoi(CARACTER);
   }
   bsq=bsq+1;
   bsqold=bsq;
   bsq=strchr(bsqold,'\n');   //busco LF
   if (bsq==0) goto error_alineamiento;
   j=bsq-bsqold;
   if (j==0){
      MyMacro.t7=0;
   }else{
      if(j>5)
         j=5;
      strncpy(CARACTER,bsqold,j);
      CARACTER[j]=0;
      MyMacro.t7=atoi(CARACTER);
   }
   /*printf("Macro:\r\n%d,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d\r\n",i+1,MyMacro.nemonico,
   			MyMacro.descripcion, MyMacro.t0, MyMacro.t1, MyMacro.t2, MyMacro.t3,
            MyMacro.t4, MyMacro.t5,MyMacro.t5, MyMacro.t6); */
   memcpy(Buffer_mac+(i*sizeof(MyMacro)),&MyMacro, sizeof(MyMacro));
   writeUserBlock(MAC_HAB+(i*sizeof(MyMacro)),&MyMacro,sizeof(MyMacro));
   strcpy(STRING_TX,"OK.La Macro se ha Modificado Correctamente\r\n");
   return;
error_alineamiento:
   strcpy(STRING_TX,"Er Ali Recepcion MAC\r\n");
}

/************************************************
Defino la estructura de las tarjetas RFID
************************************************/
typedef struct {
char  numero[8];         //tengo el ";" agregado
char  categoria;
char  nombre[10];
}Tarjetas;              //Bytes de tarjetas 19

Tarjetas buff_trj;     //buffer tarjetas
Tarjetas myTarjeta;     //buffer tarjetas
const static Tarjetas myTarjetas_default = {";471F22",'A',"BetoLietti"};

static char Buffer_trj[381], *buffer_trj;    // Buffer de Tarjetas Habilitados
/************************************************
 Función que Transmite todas las tarjetas
comando !sttT01
************************************************/
nodebug void tx_trj()
{
   bsqold=answ+3;
    if(bsq=strtok(bsqold,",")){
      if (*(bsq)!='T'){               //controlo J
         strcpy(STRING_TX,"Error Falta 'T'\r\n");    //mensaje de error
         return;
       }
    }
    bsqold=bsq+1;
    i=atoi(bsqold);
       if (i>20)
          return;
   j=i-1;
   //printf ("ID Tjta=%02d\r\n",i);
   memcpy(&myTarjeta,Buffer_trj+(j*sizeof(myTarjeta)),sizeof(myTarjeta));
   sprintf(STRING_TX,"T%02d,%s,%c,%s%s",i,myTarjeta.numero,
   			myTarjeta.categoria, myTarjeta.nombre,FIN);

}

/************************************************
 Función que carga todas las tarjetas en Control Box desde PC
comando !altT01,;471F22,0,Beto\n
************************************************/
nodebug void rx_trj()
{
   bsqold=answ+3;
    if(bsq=strtok(bsqold,",")){
      if (*(bsq)!='T'){               //controlo J
         strcpy(STRING_TX,"Error Falta 'T'\r\n");    //mensaje de error
         return;
       }
    }
    bsqold=bsq+1;
    i=atoi(bsqold);
       if (i>20)
          return;
    j=i-1;         //j es el numero de ubicación celular
    if (bsq=strtok(NULL,",")){      //Numero de celular
       strcpy(myTarjeta.numero,bsq);
    }
    if (bsq=strtok(NULL,",")){      //categoría de celular
       myTarjeta.categoria=*bsq;
    }

    if (bsq=strtok(NULL,"\n")){      //Nombre del titular celular
       strcpy(myTarjeta.nombre,bsq);
       memcpy(buffer_trj+(j*sizeof(myTarjeta)),&myTarjeta, sizeof(myTarjeta));
       writeUserBlock(TRJ_HAB+(j*sizeof(myTarjeta)),&myTarjeta, sizeof(myTarjeta));
       strcpy(STRING_TX,"OK.OK. Tarjetas cargadas Correctamente\n\r");
    }else{
       strcpy(STRING_TX,"Error Alineamiento LF\r\n");
       readUserBlock(TRJ_HAB+(j*sizeof(myTarjeta)),&myTarjeta, sizeof(myTarjeta));   //Recupero SMS_Entradas
    }

}

/************************************************
Defino la estructura de los numeros telefónicos
************************************************/
typedef struct{
int id;
char  numero[14];
char  categoria;
char  nombre[10];
}Telefono;           //Total Bytes 27

Telefono myTelefono;  //buffer telefonos
const static Telefono myTelefono_default = {1,"+543416403413",'1',"BetoLietti"};
const static Telefono myTelefono_default0 = {1,"             ",' ',""};
char  Tel_nombre[11];          //Nombre del teléfono
int   Tel_categoria;        //categoría del teléfono

static char Buffer_tel[270], *buffer_tel;    // Buffer de Teléfonos Habilitados

/*************************************************
Función que Transmite la lista de numeros telefónicos
comando !stnC0
*************************************************/
nodebug void tx_telef()
{
   bsqold=answ+3;
   if (*bsqold!='C')
      return;
   bsq=answ+4;
   i=atoi(bsq)-1;
   memcpy(&myTelefono,buffer_tel+(i*sizeof(myTelefono)),sizeof(myTelefono));
   sprintf(STRING_TX,"C%02d,%s,%c,%s%s",myTelefono.id,
   		  myTelefono.numero, myTelefono.categoria, myTelefono.nombre,FIN);


}

/************************************************
 Función que da de alta la lista de telefonos en el buffer
comando !alnC0,+543416403413,1,AlbertoPab\n
************************************************/
nodebug void rx_telef()
{
     bsqold=answ+3;
     //	printf("%s\r\n",bsqold);
      if (*bsqold!='C')
         goto error_alineamiento;
      if(bsq=strtok(bsqold,",")){
      if (*(bsq)!='C'){               //controlo J
         strcpy(STRING_TX,"Error Falta 'C'\r\n");    //mensaje de error
         return;
         }
   	}
   	bsqold=bsq+1;
   	i=atoi(bsqold);
      	if (i>10)
         	return;
      myTelefono.id=i;
      j=i-1; 			//j es el numero de ubicación celular
      if (bsq=strtok(NULL,",")){      //Numero de celular
         strcpy(myTelefono.numero,bsq);
      }
      if (bsq=strtok(NULL,",")){      //categoría de celular
         myTelefono.categoria=*bsq;
      }

      if (bsq=strtok(NULL,"\n")){      //Nombre del titular celular
       	strcpy(myTelefono.nombre,bsq);
         memcpy(buffer_tel+(j*sizeof(myTelefono)),&myTelefono, sizeof(myTelefono));
      	writeUserBlock(TEL_HAB+(j*sizeof(myTelefono)),&myTelefono, sizeof(myTelefono));
      	strcpy(STRING_TX,"OK.La activacion del Celular se ha realizado correctamente\n\r");
   	}else{
      	strcpy(STRING_TX,"Error Alineamiento LF\r\n");
      	readUserBlock(TEL_HAB+(j*sizeof(myTelefono)),&myTelefono, sizeof(myTelefono));   //Recupero SMS_Entradas
   	}
}

/************************************************
Defino la estructura de Salidas
************************************************/
typedef struct{
char  operativa;
char  nemonico[10];
char  descripcion[25];
int   tiempo;
}OUTPUT;
OUTPUT salidas;
const static OUTPUT mySalidas_default = {'A',"Sal","Salida",60};

char  Salida_Operativa[8];        //estado de la salida= Habilitado manual/No Hab
char   Salida_Nemonico[8][12];         //nemonico de la Salida
/************************************************
 Función que Transmite la configuración Salidas.
comando !stsS1
************************************************/
nodebug void tx_sal()
{

      bsqold=answ+3;
      if (*bsqold!='S')
         goto error_alineamiento;
      bsq=answ+4;
      i=*bsq-49;
      if (i<0||i>7)
         goto error_alineamiento;
      memcpy(&salidas,buffer_sal+(i*38),38);  //copio en salidas los datos
      STRING_TX[0]=  'S';
      STRING_TX[1]=  i+49;
      STRING_TX[2]=  0;
      strcat(STRING_TX,",N,");
      STRING_TX[3]=salidas.operativa;
      strncat(STRING_TX,salidas.nemonico,sizeof(salidas.nemonico));
      strcat(STRING_TX,",");
      strncat(STRING_TX,salidas.descripcion,sizeof(salidas.descripcion));
      strcat(STRING_TX,",");
      utoa(salidas.tiempo,CARACTER);
      strncat(STRING_TX,CARACTER,strlen(CARACTER));
      strcat(STRING_TX,FIN);

return;
error_alineamiento:
   strcpy(STRING_TX,"Error Alineamiento\r\n");
}

/************************************************
Funcion que recepciona el seteo de todas las Salidas
************************************************/
nodebug void rx_sal()
{

   bsqold=answ+3;
   if (*bsqold!='S')
      goto error_alineamiento;
   bsq=answ+4;
   i=*bsq-49;
   if (i<0||i>7)
      goto error_alineamiento;
      memcpy(&salidas,buffer_sal+(i*38),38);
   bsq=strchr(bsqold,',');    //busco la primera coma
   if (bsq==0) goto grabar;
   bsqold=bsq-1;
   bsqold=bsq+1;
   if (*bsqold=='S'){         //Estado de la salida Operativa/No Operativa
      salidas.operativa='S';
      Btn_Habilitado[i]=0;
   }else{
      salidas.operativa='N';
      Btn_Habilitado[i]=1;
   }
   bsqold++;
   bsqold++;
   bsq=strchr(bsqold,',');    //busco la segunda coma
   if (bsq==0) goto grabar;
   j=bsq-bsqold;
   if (j==0){                 //Nombre corto de la salida
      salidas.nemonico[j]=0;
   }else{
      if(j>9)
         j=9;
      strncpy(salidas.nemonico,bsqold,j);
      salidas.nemonico[j]=0;
   }
   bsqold=bsq+1;
   bsq=strchr(bsqold,',');    //busco la quinta coma
   if (bsq==0) goto grabar;
   j=bsq-bsqold;
   if (j==0){                 //Descripcion de la salida
      salidas.descripcion[j]=0;
   }else{
      if(j>24)
         j=24;
      strncpy(salidas.descripcion,bsqold,j);
      salidas.descripcion[j]=0;
   }
   bsq=bsq+1;
   bsqold=bsq;
   bsq=strchr(bsqold,'\n');   //busco el LF
   if (bsq==0) goto grabar;
   j=bsq-bsqold;
   if (j==0){                 //tiempo de activación Salida
      salidas.tiempo=time_ONluces;
      //CARACTER[j]=0;
   }else{
      if(j>5)
         j=5;
      strncpy(CARACTER,bsqold,j);
      CARACTER[j]=0;
      salidas.tiempo=atoi(CARACTER);
   }

   //almaceno en buffer_sal la salida
grabar:
   memcpy(buffer_sal+(i*38),&salidas, 38);
   strcpy(Salida_Nemonico[i],salidas.nemonico);
   Salida_Operativa[i]=salidas.operativa;     //salida con activa Manual/no activ
   SalidaTiempo[i]=salidas.tiempo;     //tiempo de activacion salidas
   writeUserBlock(SAL_HAB+(i*38),&salidas,38);
   strcpy(STRING_TX,"OK.La Salida se ha Modificado Correctamente\r\n");
   web=1;
   return;
error_alineamiento:
   strcpy(STRING_TX,"Error Alineamiento\r\n");
}

/************************************************
Definición de la estructura de las SMS_entradas
************************************************/
typedef struct{
char  numero;              // numero de entrada
char  activacion[40];
char  desactivacion[40];
}SMS_Entrada;                 //Bytes de Entrada =   81

SMS_Entrada mySMS_Entrada;      //buffer de SMS para activacion/desactivacion entradas
const static SMS_Entrada mySMS_Entrada_default = {0,"Activacion entrada","Desactivacion Entrada"};

/************************************************
 Función que Transmite la configuración SMS_entradas.
comando !stxX1
************************************************/
nodebug void tx_SMS_ent()
{
 	 bsqold=answ+3;
    if(bsq=strtok(bsqold,",")){
      if (*(bsq)!='X'){               //controlo J
         strcpy(STRING_TX,"Error Falta 'X'\r\n");    //mensaje de error
         return;
       }
    }
    bsqold=bsq+1;
    i=atoi(bsqold);
    i=i-1;
    if(i>3)
       return;

    memcpy(&mySMS_Entrada,buffer_sms+(i*sizeof(SMS_Entrada)),sizeof(SMS_Entrada));
    sprintf(STRING_TX,"X%d,%d,%s,%s%s",i+1,mySMS_Entrada.numero,
    			mySMS_Entrada.activacion,mySMS_Entrada.desactivacion,FIN);
    //printf("mensaje%s\r\n", STRING_TX);
   return;
}

/************************************************
Funcion que recepciona el seteo de todas los SMS entradas
El número de mensaje va desde 1 a 4
comando !alxX1,5,Activación Salida5,Desactivacion5
************************************************/

nodebug void rx_SMS_ent()
{
int s;
   bsqold=answ+3;
  	//printf ("seteo SMS_entradas:\t%s\r\n",answ);
   if(bsq=strtok(bsqold,",")){
      if (*(bsq)!='X'){               //controlo J
         strcpy(STRING_TX,"Error Falta 'X'\r\n");    //mensaje de error
         return;
         }
   }
   bsqold=bsq+1;
   s=atoi(bsqold);
   if ((s<1)||(s>4)){
   	strcpy(STRING_TX,"Error Numero SMS \r\n");
      return;
	}
   s=s-1;          //acomodo el numero de mensaje 0-3

   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      mySMS_Entrada.numero=atoi(bsq);
   }
   if (bsq=strtok(NULL,",")){      //Nemonico Entrada
      strcpy(mySMS_Entrada.activacion,bsq);
   }
  if (bsq=strtok(NULL,"\n")){      //Gateway sistema
      strcpy(mySMS_Entrada.desactivacion,bsq);
      //Guardo seteo entradas
      memcpy(buffer_sms+(s*sizeof(SMS_Entrada)),&mySMS_Entrada, sizeof(SMS_Entrada));
      writeUserBlock(SMS_HAB+(s*sizeof(SMS_Entrada)),&mySMS_Entrada,sizeof(SMS_Entrada));
      strcpy(STRING_TX,"OK.Los SMS_Entrada se ha Modificado Correctamente");
   }else{
      strcpy(STRING_TX,"Error Alineamiento LF\r\n");
      readUserBlock(Buffer_sms,SMS_HAB,350);   //Recupero SMS_Entradas
   }
}

/************************************************
Funcion que busca si hay un SMS asociado
Devuelve 1 existe y 0 no existe
en mySMS_Entrada estructura de los mensajes
************************************************/
nodebug int fn_SMS_asociado(int n)
{
int s;
	for(s=0;s<4;s++){
   	memcpy(&mySMS_Entrada, buffer_sms+(s*sizeof(SMS_Entrada)), sizeof(SMS_Entrada));
   	if (n == mySMS_Entrada.numero)
      	return(1);
   }
   return(0);
}


/*****************************************
Recepción de hora "yymmddhhmmss"
!horyymmddhhmmss
*****************************************/
nodebug void recept_time()
{
   struct tm      rtc;                 // time struct
   strncpy (th,answ+3,12);
   memset(answ,0x00,20);   //pongo en cero el buffer BLNK
   //serCputs("recepcion\r\n");
   //serCputs(TH);
   v = TH[11] + ((TH[10]-48)*10)-48;
   rtc.tm_sec = v;
   v = TH[9] + ((TH[8]-48)*10)-48;
   rtc.tm_min = v;
   v = TH[7] + ((TH[6]-48)*10)-48;
   rtc.tm_hour = v;
   v = TH[1] + ((TH[0]-48)*10)-48;
   rtc.tm_year = v+100;
   v = TH[3] + ((TH[2]-48)*10)-48;
   rtc.tm_mon = v;
   v = TH[5] + ((TH[4]-48)*10)-48;
   rtc.tm_mday = v;
   tm_wr(&rtc);                  // set clock
   strcpy(STRING_TX,"Actualizacion Hora OK\n\r");
}


/************************************************
Función que lee los parámetros almacenados en Flash
************************************************/
nodebug void leer_flash()
{
      readUserBlock(Buffer_tel,TEL_HAB,270);
      readUserBlock(Buffer_trj,TRJ_HAB,380);
      readUserBlock(Buffer_ens,ENS_HAB,349);
      readUserBlock(Buffer_sms,SMS_HAB,350);
      readUserBlock(Buffer_ent,ENT_HAB,800);
      readUserBlock(Buffer_sal,SAL_HAB,304);
      readUserBlock(Buffer_alm,ALM_HAB,249);



//Guardo las principales variables de las salidas
      for(i=0; i<8; i++){
         memcpy(&salidas,buffer_sal+(i*38),38);  //leo de a una las salidas
         strcpy(Salida_Nemonico[i],salidas.nemonico);
         Salida_Operativa[i]=salidas.operativa;
         SalidaTiempo[i]=salidas.tiempo;     //tiempo de activacion Salida
         if (Salida_Operativa[i]=='N')
            Btn_Habilitado[i]=1;
         else
            Btn_Habilitado[i]=0;
      }
//Guardo los nemonicos de las Entradas
      for(i=0; i<16; i++){
         memcpy(&myEntrada,buffer_ent+(i*sizeof(Entrada)),sizeof(Entrada));  //leo de a una las entradas
         strcpy(Entrada_Nemonico[i],myEntrada.nemonico);
      }
//Guardo las máscaras de activaciones
      for(i=0; i<4; i++){
         memcpy(&myActivacion,buffer_alm+(i*sizeof(myActivacion)),sizeof(myActivacion));  //leo de a una las entradas
         myMasc_activacion[i] = myActivacion.programacion;
         strcpy(myNemo_activacion[i],myActivacion.nemonico);
      	strcpy(myDesc_activacion[i],myActivacion.descripcion);
      }

      readUserBlock(Buffer_mac,MAC_HAB,108);
      readUserBlock(&myHard,EQP_HAB,sizeof(myHard));
      ent_operativas=myHard.ent_operativas;     //entradas habilitadas
      ent_no_asociada=myHard.ent_no_asociada;   //entradas de alarma
      //set_alrm_sleep=myHard.entalrmsleep; //entradas de alarma noct
      ent_tipo=myHard.enttipo;      //tipo de entradas NA/NC
      time_ONluces=myHard.tON_luces;//tiempo de encendido luces
      time_act1=myHard.t_activar;    //tiempo de activacion alarma
      time_act2=myHard.t_desactivar;    //tiempo de desactivacion alarma
}

/**********************************************
Función que inicializa el stack TCP y Telnet
**********************************************/
nodebug void inicio_IP()
{

/************Leo Datos de myconfig**************/
   readUserBlock(&myconfig,CONFIG_IP,sizeof(Config));
   sock_init();
   http_init();
   tcp_reserveport(80);

   ifconfig(IF_ETH0,                   //cambio la configuracion de IP
            IFS_DOWN,
            IFS_IPADDR, myconfig.src_ip,
            IFS_NETMASK,myconfig.src_mask,
            IFS_ROUTER_SET, myconfig.def_gwy,
            IFS_NAMESERVER_SET,myconfig.my_dns,
            IFS_UP,
            IFS_END);
   tcp_reserveport(MY_PORT);
   state=INIT;
   stateold=INIT;
}

/**********************************************
Función que convierte y almacena los datos de IP
**********************************************/
nodebug void set_IP()
{
   readUserBlock(&myHard,EQP_HAB,sizeof(myHard));
   myconfig.src_ip=inet_addr(myHard.ip);
   myconfig.src_mask=inet_addr(DEFAULT_NETMASK);
   myconfig.def_gwy=inet_addr(myHard.gateway);
   myconfig.my_dns=inet_addr(myHard.gateway);
   writeUserBlock(CONFIG_IP,&myconfig,sizeof(Config));
   inicio_IP();
}

/*
Actualizar Datos Flash
*/
//#nodebug
/************************************************
Conversión de tiempo en TH[] Caracteres en ASCII sin separadores
************************************************/
nodebug void conversion (unsigned long tiempo)
{
   struct tm   thetm;
   mktm(&thetm, tiempo);
   TH[0]=0;
   itoa(thetm.tm_hour,CARACTER);
   if(strlen(CARACTER)==1){
   strcat(TH,"0");
   }
   strcat(TH,CARACTER);
   strcat(TH,":");
   itoa(thetm.tm_min,CARACTER);
   if(strlen(CARACTER)==1){
   strcat(TH,"0");
   }
   strcat(TH,CARACTER);
   strcat(TH,":");
   itoa(thetm.tm_sec,CARACTER);
   if(strlen(CARACTER)==1){
   strcat(TH,"0");
   }
   strcat(TH,CARACTER);
   strcat(TH," ");

   itoa(thetm.tm_mday,CARACTER);
   if(strlen(CARACTER)==1){
   strcat(TH,"0");
   }
   strcat(TH,CARACTER);
   strcat(TH,"/");

   itoa(thetm.tm_mon,CARACTER);
   if(strlen(CARACTER)==1){
   strcat(TH,"0");
   }
   strcat(TH,CARACTER);
   strcat(TH,"\0");
}
/************************************************
 Funcion de almacenamiento de eventos
************************************************/

nodebug void almacenar_event(BuffTarj *buffer)
{
	root2xmem(dirxalrm + puntin_event * sizeof(BuffTarj), buffer, sizeof(BuffTarj));
	puntin_event++;
	if (puntin_event > LARGO_BUFFTRJ){
		puntin_event=0;
	}
	if (puntin_event == puntout_event){
		puntout_event++;
	   if (puntout_event > LARGO_BUFFTRJ){
	   	puntout_event=0;
	   }
	}
}
/************************************************
 Funcion de envio de eventos a PC
************************************************/
void buscar_event()
{
	if(puntout_event==puntin_event){
   	strcpy(STRING_TX,"No hay mas eventos almacenados \r\n");
      return;
   }
   xmem2root(&buff_almac,dirxalrm + puntout_event * sizeof(BuffTarj), sizeof(BuffTarj));      // guardo buffer1 en Xmem
   puntout_event++;
   if (puntout_event > LARGO_BUFFTRJ){
   	puntout_event=0;
   }
   conversion(buff_almac.fcha);
   sprintf(STRING_TX,"Resultado:%c\tTipo:%.6s\tFecha:%s\r\n",
      			buff_almac.estdo,buff_almac.trjta,th);

}
/************************************************
 Funcion de encendido SIM
************************************************/
cofunc SIM_ON()
{
   if (BitRdPortI(PBDR,0)){
   	t1=read_rtc();
      SIM_OK=off;
      WrPortI(PBDR, NULL, 0x00);
      count=serBread(bufferc,sizeof(bufferc)-1,300);
      waitfor(DelayMs(1500)||!BitRdPortI(PBDR,0));
      WrPortI(PBDR, NULL, 0x80);
      //printf("PASE\r\n");
      for(j=0; j<6; j++){
         count=0;
         memset(bufferc,0x00,10);
         waitfor(DelaySec(20)||(count=serBread(bufferc,sizeof(bufferc)-1,300))); // net confirm
         //printf("PASE%d\r\n",j);
         if(strstr(bufferc,"Call Ready")){
            buff_almac.fcha=t1;
            buff_almac.estdo='O';
      		strcpy(buff_almac.trjta,"CalRdy");
            almacenar_event(& buff_almac);
            SIM_OK=on;
            return(1);
         }
      }
      //printf("PASE8\r\n");

      buff_almac.estdo='E';
      strcpy(buff_almac.trjta,"CalRdy");
   }else{
      SIM_OK=on;
      buff_almac.estdo='O';
      strcpy(buff_almac.trjta,"SIMRdy");
   }
   //printf("PASE9\r\n");
   buff_almac.fcha=t1;
   almacenar_event(& buff_almac);
}

/********************************************
Funcion de apagado del SIM por puerto de hardware
********************************************/
cofunc SIM_OFF(){
//printf("proceso de apagado SIM \r\n");
   SIM_OK=off;       //Deshabilito funciones del SIM
   WrPortI(PBDR, NULL, 0x00);
   //printf("primer proceso de apagado SIM\t%u\r\n",MS_TIMER);
   waitfor(DelayMs(2500)||BitRdPortI(PBDR,0));      //750 MS de SIM apagado
   //printf("segundo proceso de apagado SIM\t%u\r\n",MS_TIMER);
   WrPortI(PBDR, NULL, 0x80);
   waitfor(DelayMs(1000)||BitRdPortI(PBDR,0));

   WrPortI(PBDR, NULL, 0x00);
   waitfor(DelayMs(1500)||BitRdPortI(PBDR,0));      //750 MS de SIM apagado
   //printf("Tercer proceso de apagado SIM\t%u\r\n",MS_TIMER);
   WrPortI(PBDR, NULL, 0x80);
   waitfor(DelayMs(6000)||BitRdPortI(PBDR,0));
   t1=read_rtc();
/******Almaceno resultado        ****************/
   buff_almac.fcha=t1;
   if(BitRdPortI(PBDR,0)){
      buff_almac.estdo='O';
      strcpy(buff_almac.trjta,"CelOFF");
      SIM_OFF_OK=1;
   }else{
      buff_almac.estdo='E';
      strcpy(buff_almac.trjta,"CelOFF");
      SIM_OFF_OK=0;
   }
   almacenar_event(& buff_almac);
/******Fin Almacenamiento        ****************/
   SIM_off=off;
}

/*************************************************
 Cofunción de seteo SIM
*************************************************/
cofunc seteoSim()
{
     SIM_OK=off;
      count=serBread(bufferc,sizeof(bufferc)-1,300);
      serBputs("ATE0\r\n"); //elimino el echo del puerto serie
      waitfor(DelaySec(5)||(count=serBread(bufferc,sizeof(bufferc)-1,300)));
      if(!strstr(bufferc,"OK")){
         serCputs("Error Echo SIM\r\n");
         goto fin_seteo_sim;
      }
      serBputs("AT+CMGF=1\r\n"); //Mensajes en modo Texto
      waitfor(DelaySec(5)||(count=serBread(bufferc,sizeof(bufferc)-1,300)));
      if(!strstr(bufferc,"OK")){
         serCputs("Error SMS SIM\r\n");
         goto fin_seteo_sim;
      }
      serBputs("AT+CNMI=2,2,0,0,0\r\n"); //Muestro el SMS directamente
      waitfor(DelaySec(5)||(count=serBread(bufferc,sizeof(bufferc)-1,300)));
        if(!strstr(bufferc,"OK")){
         serCputs("Error Mostrar SMS SIM\r\n");
         goto fin_seteo_sim;
      }
      serBputs("AT+COLP=1\r\n"); //Saber cuando B contesta
      waitfor(DelaySec(5)||(count=serBread(bufferc,sizeof(bufferc)-1,300)));
      if(!strstr(bufferc,"OK")){
         serCputs("Error B contesta SIM\r\n");
         goto fin_seteo_sim;
      }
      serBputs("AT+CLIP=1\r\n"); //Muestro el Caller id
      waitfor(DelaySec(5)||(count=serBread(bufferc,sizeof(bufferc)-1,300)));
      if(!strstr(bufferc,"OK")){
         serCputs("Error Caller ID SIM\r\n");
         goto fin_seteo_sim;
      }
      serBputs("AT+IPR=19200\r\n"); //Programo port serie a 19200
      waitfor(DelaySec(5)||(count=serBread(bufferc,sizeof(bufferc)-1,300)));
      if(!strstr(bufferc,"OK")){
         serCputs("Error Port Serie SIM\r\n");
         goto fin_seteo_sim;
      }
      serBputs("AT+VTD=3\r\n");
      waitfor(DelaySec(5)||(count=serBread(bufferc,sizeof(bufferc)-1,300)));
      if(!strstr(bufferc,"OK")){
         serCputs("Error Tonos SIM\r\n");
         goto fin_seteo_sim;
      }
      serBputs("AT&W\r\n"); //grabo config
      waitfor(DelaySec(5)||(count=serBread(bufferc,sizeof(bufferc)-1,300)));
      if(!strstr(bufferc,"OK")){
         serCputs("Error Grabado SIM\r\n");
         goto fin_seteo_sim;
      }
      serCputs("Seteo SIM OK\r\n");
fin_seteo_sim:
      SIM_OK=on;
}


/*
 llamada a un numero fijo
*/
cofunc llamar()
{
   costate{
      serBputs("ATD3413098825;\r");
      //printf("estoy en la rutina de llamda\r\n");      //espero que el buffer esté vacío
      waitfor (DelaySec(2));
   }
}


/*
 cortar llamada
*/
cofunc cortar()
{
   costate{
   serBputs("ATH0\r");
   waitfor(serBwrFree()!=BOUTBUFSIZE);    //espero que el buffer esté vacío
   waitfor (DelaySec(2));
   }
}


/******************************
Armar Mensaje de estado alarmapara web
******************************/
void Estado_Alarma_Web()
{
   memset(est_alrm_web,0x00,10);
   switch(estado_alrm){
      case armado:    //alarma Activada
         strcpy(col_alrm_web,"green");
         conversion_web(t_activacion);
         sprintf(est_alrm_web,"Seguridad&#32&#32%s&#32Activa&nbsp;%s&nbsp;%s",
         			myNemo_activacion[tipo_alarma],nombre_act,th);
         break;
      case disparada:    //alarma Disparada
         strcpy(col_alrm_web,"red");
         conversion_web(tds);
         sprintf(est_alrm_web,"Seguridad:&#32ALARMA&#32%s",th);
         break;
      default:    //alarma Desactivada
         strcpy(col_alrm_web,"black");
         conversion_web(t_activacion);
         sprintf(est_alrm_web,"Seguridad:&#32Desactivada&#32%s&#32%s",nombre_act,th);
         //strcat(est_alrm_web,nombre_act);
   }
   //strcat(est_alrm_web,"&nbsp;");
   //strcat(est_alrm_web,th);
   //printf("estado web %s",est_alrm_web);
}

/************************************************
Función que actualiza las ssalidas para la Web
************************************************/
/*
void actualizar_salidasWeb(int h)
{
  // printf ("salida estado1: %c\r\n",Salida_Operativa[h]);

   strcpy(Btn_Texto[h],Salida_Nemonico[h]); //Guardo en NameSal el nemonico
   if (Salida_Operativa[h]=='S'){  //habilitado/deshabilitado
      Btn_Habilitado[h]=0;
      if(strstr(Btn_Color[h],"red")){
         strcpy(Btn_Color[h],"green");
         res(porta,h);
         strcat(Btn_Texto[h],"&#10&#32OFF"); //Agrego a NameSalWeb OFF
      }else{
         strcpy(Btn_Color[h],"red");
         strcat(Btn_Texto[h],"&#10&#32ON");
         set(porta,h);
         TS[h]=t0+SalidaTiempo[h];
      }
   }else{
      Btn_Habilitado[h]=1;
      strcat(Btn_Texto[h],"&#10&#32NOP");
      strcpy(Btn_Color[h],"transparent");
   }
   Estado_Alarma_Web();
   //printf ("salida estado2: %c\r\n",Salida_Operativa[h]);
}


*/


char  Text_On[140];  //Texto de entradas ON
char  Text_Of[140];  //Texto de entradas OFF
char  Text_Al[140];  //Texto de entradas Alarma
/************************************************
Analiza estado entradas asociadas para la alarma.
Informa si la entrada se disparo o no, y si se armó
************************************************/
void Estado_Inputs_Alrm()
{
   Text_On[0]=0;
   Text_Of[0]=0;
	Text_Al[0]=0;
   for(n=0;n<16;n++){
      if(BIT(&set_alrm,n)){   //Entradas seteadas para la Alarma-Máscara de Activación
      //seleccion de estado ON / OFF / Disp.
         if(BIT(&alrm_set_flag,n)){  //Entradas Armadas para la Alarma
            if(BIT(&alrm_flag,n)){  //Entrada Armada/Disparada
               strcat(Text_Al,Entrada_Nemonico[n]);
               strcat(Text_Al,"-");

            }else{  //Entrada Armada/Normal
               strcat(Text_On,Entrada_Nemonico[n]);
               strcat(Text_On,"-");

            }
         }else{ //Entrada Off en el momento de armado alarma
          strcat(Text_Of,Entrada_Nemonico[n]);
          strcat(Text_Of,"-");

         }
      }
   }
   if (strlen(Text_Al)){
       strcat(buff_almacSMS.mensaje,"ALRM\n");
       Text_Al[strlen(Text_Al)-1]= 10;
       strcat(buff_almacSMS.mensaje,Text_Al);
   }
   if (strlen(Text_On)){
       strcat(buff_almacSMS.mensaje,"ON\n");
       Text_On[strlen(Text_On)-1]= 10;
       strcat(buff_almacSMS.mensaje,Text_On);
   }
   if (strlen(Text_Of)){
       strcat(buff_almacSMS.mensaje,"OFF\n");
       Text_Of[strlen(Text_Of)-1]= 10;
       strcat(buff_almacSMS.mensaje,Text_Of);
   }
   strcat(buff_almacSMS.mensaje,"\n");
   if (energia){
      strcat(buff_almacSMS.mensaje,"Red: OK\n");
   }else{
      strcat(buff_almacSMS.mensaje,"Red: Falla\n");
   }
}

/************************************************
Función que almacena en buff_almac estado de entradas
NO ASOCIADAS a la funcion de Alarma
************************************************/
void Estado_Inputs_NoAlarma()
{
    for(i=0; i<16; i++){
      if(BIT(&ent_operativas,i)){           //entradas habilitadas
         if(BIT(&ent_no_asociada,i)){       //entradas no asociadas para alarma
            //traer_ent(i);
            if(strlen(Entrada_Nemonico[i])){
               strcat(buff_almacSMS.mensaje,Entrada_Nemonico[i]);

               if (!BIT(&INPUT,i)){     // puerto entradas numero i
                  strcat(buff_almacSMS.mensaje," Normal\n");
               }else{
                  strcat(buff_almacSMS.mensaje," Falla\n");
               }
            }
         }
      }
   }
   if (energia){
      strcat(buff_almacSMS.mensaje,"Red: OK\n");
   }else{
      strcat(buff_almacSMS.mensaje,"Red: Falla\n");
   }

}


//////////////////////////////////////////////////

/************************************************
 Función que busca la descripción de entradas.
************************************************/
/*void traer_ent(int u)
{
     memcpy(&myEntrada,buffer_ent+(u*sizeof(Entrada)),sizeof(Entrada));
} */







////////////////////////////////////////////////////////////////////////////////
// prints out date and time handed to it in tm struct
/*
void print_time(unsigned long thetime)
{
   struct tm   thetm;
   mktm(&thetm, thetime);

   //printf("%02d/%02d/%04d %02d:%02d:%02d\n\n",
         thetm.tm_mon, thetm.tm_mday, 1900+thetm.tm_year,
         thetm.tm_hour, thetm.tm_min, thetm.tm_sec);

}

*/
//función que almacena en buff_almac estado de salidas
/*void string_output()
{
   strcpy(STRING_TX,"Salidas:");
   for(i=0; i<8; i++){
      if (BIT(porta,i)){
         OUTPUTS[i]='1';
      }else{
         OUTPUTS[i]='0';
      }
   }
   OUTPUTS[i]=0;
   strcat(STRING_TX,OUTPUTS);
   strcat(STRING_TX,"\n");

   strcat(STRING_TX,"Entradas:");
   for(i=0; i<8; i++){
      if (BIT(&INPUT,i)){
         OUTPUTS[i]='1';
      }else{
         OUTPUTS[i]='0';
      }
   }
   OUTPUTS[i]=0;
   strcat(STRING_TX,OUTPUTS);
   strcat(STRING_TX,"\n");
   //serCputs(OUTPUTS);
   //serCputs("SALIDAS \r\n");
} */

//función que almacena en Xmem la tarjeta
void almacenar_tarj(BuffTarj *buffer)
{
      root2xmem(dirxmemo + puntin_tjtas * 11, buffer, 11);
      puntin_tjtas++;
      if (puntin_tjtas > LARGO_BUFFTRJ){
             puntin_tjtas=0;
      }
      if (puntin_tjtas == puntout_tjtas){
             puntout_tjtas++;
         if (puntout_tjtas > LARGO_BUFFTRJ){
                puntout_tjtas=0;
         }
      }
}

//función que recupera de Xmem la tarjeta
void buscar_tarj()
{
      if(puntout_tjtas==puntin_tjtas){
         strcpy(STRING_TX,"No hay mas tarjetas almacenadas \r\n");
         return;
      }
      xmem2root(&buff_almac,dirxmemo + puntout_tjtas * 11, 11);      // guardo buffer1 en Xmem
      puntout_tjtas++;
      if (puntout_tjtas > LARGO_BUFFTRJ){
             puntout_tjtas=0;
      }
      conversion(buff_almac.fcha);
		sprintf(STRING_TX,"%cTarjeta: %.6s Fecha: %s\r\n",
      			buff_almac.estdo,buff_almac.trjta,th);
      //printf("%cTarjeta: %.6s Fecha: %s\r\n", buff_almac.estdo,buff_almac.trjta,th);

}


//función que recupera de Xmem los eventos


/************************************************
Función que almacena para ser envíado el SMS
************************************************/

nodebug void almacenar_SMS(SMS_envio *sms)
{
	//printf("\r\nFuncion Almacenamiento SMS \r\n%sX\r\n%sX\r\n",sms->numero_envio,sms->mensaje);
  	memcpy(buffer_envioSMS + (puntin_SMS * sizeof(SMS_envio)),sms,sizeof(SMS_envio));
   puntin_SMS++;
   if (puntin_SMS > LARGO_ENVIOSMS-1){
      puntin_SMS=0;
   }
   if (puntin_SMS == puntout_SMS){
      puntout_SMS++;
      if (puntout_SMS > LARGO_ENVIOSMS-1){
         puntout_SMS=0;
      }
   }
}

/*************************************************
 Función que agrega fecha/hora y ControlBox a Mensaje SMS
 y lo Almacena para su envío
*************************************************/
void armar_SMS(SMS_envio *armar_sms)
{
  conversion(t0);
  strcat(armar_sms->mensaje,"\n");
  strcat(armar_sms->mensaje,th);
  strcat(armar_sms->mensaje,"\nControlBox");
  almacenar_SMS(armar_sms);
}

/************************************************
 Función Almacena SMS para Celulares con una
 Categoria dada por u - mensaje en sms_cat
************************************************/
void fn_SMS_Cat(int u,SMS_envio *sms_cat)
{
  for(n=0; n<8; n++){
      if (traer_telef(n)<=u){ //Busco el Telefono con Cat 1
         strcpy(sms_cat->numero_envio, numero);
         almacenar_SMS(sms_cat);
      }
   }
}

/************************************************
 Función que Transmite la lista de nombres de numeros telefónicos por SMS
************************************************/

void sms_telef()
{
   strcpy(string,"Nombres Celulares:\n");
   for(i=0; i<10; i++){
      memcpy(&myTelefono,buffer_tel+(i*sizeof(myTelefono)),sizeof(myTelefono));
      if(strlen(myTelefono.nombre)){
         strncat(string,myTelefono.nombre,sizeof(myTelefono.nombre));
         strcat(string,"\n");
      }
   }
   strcat(string,"\n");
   strcpy(buff_almacSMS.mensaje,string);
   armar_SMS(&buff_almacSMS);
   serCputs (string);
}
/*
 Función que busca el telefono "u" del buffer
 Trae un dado telefono desde el buffer
*/
int traer_telef(int u)
{
int cat;
   memcpy(&myTelefono,buffer_tel+(u*sizeof(myTelefono)),sizeof(myTelefono));
   strncpy(NUMERO,myTelefono.numero,sizeof(myTelefono.numero));
   if (NUMERO[0]!='+') {return(9);}
   strncpy(Tel_nombre,myTelefono.nombre,sizeof(myTelefono.nombre));
   Tel_nombre[10]=0;
   cat= atoi(&myTelefono.categoria);
   return(cat);
}
/************************************************
 Busco si existe un numero de teléfono De Comando
 devuelvo 1 si encontré y 0 no encontrado
 Tel_nombre Nombre encontrado y Tel_categoría  categoría encontrada
************************************************/
int Verificar_Telefono_Comandos()
{
   /*Busco los telefonos de Comandos*/
   if(strstr("+543416403413",NUMERO)){
        strcpy(Tel_nombre,"C.Box");
        Tel_categoria=0;
        return(1);    //tel de comandos

   }
   if(strstr("+543413031303",NUMERO)){
        strcpy(Tel_nombre,"C.Box");
        Tel_categoria=0;
        return(1);    //tel de comandos
   }
   strcpy(Tel_nombre,"NoConocido");
   Tel_categoria=10;
   Tel_nombre[10]=0;
   return(0); // tel inexistente en lista
}





/*
 Busco si existe un numero de teléfono en el buffer
 devuelvo 1 si encontré y 0 no encontrado
 Tel_nombre Nombre encontrado y Tel_categoría  categoría encontrada
*/
int Verificar_Telefono()
{
   for(i=0; i<10; i++){
      memcpy(&myTelefono,buffer_tel+(i*sizeof(myTelefono)),sizeof(myTelefono));
      if(strstr(myTelefono.numero,NUMERO)){
        strncpy(Tel_nombre,myTelefono.nombre,10);     //guardo el nombre
        Tel_nombre[10]=0;
        Tel_categoria=atoi(&myTelefono.categoria);    //guardo la categoria
        return(1);    //tel encontrado
      }
   }

   /*Busco los telefonos de Comandos*/
   if(strstr("+543416403413",NUMERO)){
        strcpy(Tel_nombre,"C.Box");
        Tel_categoria=0;
        return(1);    //tel de comandos

   }
   if(strstr("+543413031303",NUMERO)){
        strcpy(Tel_nombre,"C.Box");
        Tel_categoria=0;
        return(1);    //tel de comandos
   }


   strcpy(Tel_nombre,"NoConocido");
   Tel_categoria=10;
   Tel_nombre[10]=0;
   return(0); // tel inexistente en lista
}




/*
 Función que busca un telefono del buffer
*/
/*void buscar_tel(int u)
{
     memcpy(&myTelefono,buffer_tel+(u*sizeof(myTelefono)),sizeof(myTelefono));
     strncpy(NUMERO,myTelefono.numero,sizeof(myTelefono.numero));
     //strncpy(Tel_nombre,buff_tel.nombre,sizeof(buff_tel.nombre));
     //Tel_nombre[10]=0;
}
  */

/*
 Función que Transmite la lista de nombres de tarjetas por SMS
*/

void sms_trj()
{
   strcpy(string,"Nombres Tarjetas:\n");
   for(i=0; i<10; i++){
      memcpy(&buff_trj,Buffer_trj+(i*sizeof(buff_trj)),sizeof(buff_trj));
      if (strlen(buff_trj.nombre)){
         strncat(string,buff_trj.nombre,sizeof(buff_trj.nombre));
         strcat(string,"\n");
      }
   }
   strcat(string,"\n");
   strcpy(buff_almacSMS.mensaje,string);
   armar_SMS(&buff_almacSMS);
   serCputs (string);
}



/**************************************************************
 Función que busca y valida un tarjeta en el buffer de Tarjetas
**************************************************************/
nodebug int validar_trj(char *f_tarjeta)
{
      //printf ("f_tarjeta:\t%s\t%d\r\n",f_tarjeta, strlen(f_tarjeta));
      for(i=0; i<20; i++){
      memcpy(&buff_trj,buffer_trj+(i*sizeof(buff_trj)),sizeof(buff_trj));
         if(strstr(buff_trj.numero,f_tarjeta)){
            strncpy(nombre,buff_trj.nombre,10); //Nombre del titular tarjeta
            nombre[10]=0;
            /**************voy a buscar nombre celular agregado21/01****/
            strcpy(ANSW,nombre);
            for(i=0; i< strlen(answ); i++){  //paso a minuscula
               answ[i]=tolower(answ[i]);
            }
            return(1);
         }
      }
       strcpy(buff_almac.trjta,f_tarjeta); //almaceno tarjeta
       buff_almac.fcha=t0;
       buff_almac.estdo='I';
       almacenar_tarj(&buff_almac);
       return(0);
}


/*
 Función que Transmite la por SMS configuración Salidas.
comando "salidas"
*/
void sms_sal()
{
   strcpy(string,"Salidas:\n");
   for(i=0; i<8; i++){
      memcpy(&salidas,buffer_sal+(i*38),38);
      if(strlen(salidas.nemonico)){
         strncat(string,salidas.nemonico,sizeof(salidas.nemonico));
         strcat(string,"\n");
      }
   }
   strcat(string,"\n");
   strcpy(buff_almacSMS.mensaje,string);
   armar_SMS(&buff_almacSMS);
   serCputs (string);
}



/*
 Función que Transmite las mensajes alarma por RS232
*/
/*void tx_sms()
{
   for(i=0; i<10; i++){
     Buffer_txc[0]='S';
     Buffer_txc[1]=i+48;
     Buffer_txc[2]=',';
     memcpy(buffer_txc+3,buffer_sms+(i*30),30);
     if (strlen(Buffer_txc)>=33){
        Buffer_txc[33]=0;
     }else{
        Buffer_txc[strlen(Buffer_txc)]=0;
     }
     strcat(buffer_txc,"\n");
     serCputs(Buffer_txc);
     //printf(buffer_txc);
   }
  serCputs (FIN);
} */

/*
 Función que da de alta un mensaje en el buffer

void set_asms()
{
      id=answ[3]-48;
      //printf("Largo %d mensaje %s\r\n",strlen(answ),answ);
      c=strlen(answ);
      if((c-4)>=30){
         c=30;
      }
      memcpy(buffer_sms+(id*30),answ+4,c);
      writeUserBlock(SMS_HAB,Buffer_sms,300);
      serCputs("OK\r\n");
}
*/

/*
 Función que busca coincidencias con SMS.
comando "salidas"
*/
/*void buscar_sal()
{
   for(i=0; i<8; i++){
     memcpy(&salidas,buffer_sal+(i*38),38);
     strcpy(texto_bsq,salidas.nemonico);
     texto_bsq[sizeof(salidas.nemonico)]=0;
     for(j=0; j< strlen(texto_bsq); j++){
               texto_bsq[j]=tolower(texto_bsq[j]);
            }
     if (strstr(ANSW,texto_bsq)) {
      //printf("exito busqueda %d %s\n", i,texto_bsq );
      strcat(texto_bsq," on\0");
      if (strstr(ANSW,texto_bsq)) {
         set(porta,i);
         strcpy(buff_almacSMS.mensaje,"Salida 3 Activada.\n");
         //printf("encender\n");
         return;
      }
      texto_bsq[strlen(texto_bsq)-1]='f';
      if (strstr(ANSW,texto_bsq)) {
         res(porta,i);
         //printf("apagar\n");
         return;
      }
      return;
     }
   }
} */

/************************************************
Función que busca coincidencias con nombres celulares.
comando "salidas"
************************************************/
int buscar_nom_cel(char *buscar_nom[11])
{
   char busq_nom[11],nom;
   char busq_nomOri[11];
   int largo_nom;
  	strcpy(busq_nom,buscar_nom);
   for(x=0; x< strlen(busq_nom); x++){  //paso todo a minúscula
               busq_nom[x]=tolower(busq_nom[x]);
   }
   for(i=0; i<10; i++){
     memcpy(&myTelefono,buffer_tel+(i*sizeof(myTelefono)),sizeof(myTelefono));
     strcpy(busq_nomOri,myTelefono.nombre);
     for(x=0; x< strlen(busq_nomOri); x++){  //paso todo a minúscula
               busq_nomOri[x]=tolower(busq_nomOri[x]);
     }
     if (strstr(busq_nomOri,busq_nom)) {
   //         printf("Categoria:%d\r\n",atoi(&myTelefono.categoria));
            return(atoi(&myTelefono.categoria));
     }
   }
   return(-1);
}

/*
Función que busca coincidencias con nombres tarjetas.
comando "salidas"
*/
int buscar_nom_tarj()
{
   for(i=0; i<20; i++){
     memcpy(&buff_trj,Buffer_trj+(i*sizeof(buff_trj)),sizeof(buff_trj));
     strcpy(texto_bsq,buff_trj.nombre);
     texto_bsq[sizeof(buff_trj.nombre)]=0;
     for(j=0; j< strlen(texto_bsq); j++){  //paso todo a minúscula
               texto_bsq[j]=tolower(texto_bsq[j]);
     }
     if (strstr(ANSW,texto_bsq)) {
         return(i);
     }
   }
   return(-1);
}

/************************************************
Funcion que inicializa las variables de Control Box
************************************************/
void inicio_cbox()
{
   th=&TH[0];
   intr=0;
   tv1=off;             //estado Tarj1 off
   tv2=off;             //estado Tarj2 off
   lr1=off;             //estado Led1 reset  off
   lr2=off;             //estado Led2 reset  off
   answ=&ANSW[0];
   ANSW[0]=0;
   comando=&Comando[0];
   Comando[0]=0;
   RING_value=0;
   alrm_disp=off;    // alarma no activa
   Llamada_OK=off;   //llamada no activa
   Llamada_SAL=on;   //llamada no activa
   EnvioSMS_ON=off;  // no hay envío SMS
   EnvioSMS_ERROR=0; //Pongo en cero contador de errores sms
   input = &INPUT;
   input_old = &INPUT_OLD;
   input_comp = &INPUT_COMP;
   porta = &PORTA;
   PORTA=0x00;
   PORTERO1=off;
   PORTERO2=off;
   bufferrx[0]=0;
   c = 0;
   serCopen(B_BAUDRATE);
   serBopen(B_BAUDRATE);
   serCputs("Inicio Variables\r\n");
   string=&STRING[0];
   buffer_tel=&Buffer_tel[0];
   buffer_trj=&Buffer_trj[0];
   buffer_ens=&Buffer_ens[0];
   buffer_sms=&Buffer_sms[0];
   buffer_ent=&Buffer_ent[0]; //buffer mensajes de aviso entradas
   buffer_txc=&Buffer_txc[0];
   buffer_sal=&Buffer_sal[0];
   buffer_alm=&Buffer_alm[0];
   buffer_envioSMS=&Buffer_envioSMS[0];
   numero=&NUMERO[0];
   numero_sms=&NUMERO_SMS[0];
   NUMERO[13]=0;
   nombre=&NOMBRE[0];
   NOMBRE[10]=0;
   nombre_act=&NOMBRE_ACT[0];    //nombre de quien activó o desactivo alarma
   NOMBRE_ACT[10]=0;
   texto_sms=&TEXTO_SMS[0];
   TEXTO_SMS[30]=0;
   Buffer_trj[380]=0;
   SIM_on=on;            //Encendido SIM
   Set_SIM=off;          //No voy a seter SIM
   SIM_off=off;          //Apagado SIM
   SIM_reset=off;       //no hago reset
   SIM_OK=off;           //SIM no Habilitado
   SMS_delay=1;
   alrm_bat=off;         //Borro alarma de batería baja
   INI=0;

   leer_flash();         //lee y actualiza las variables del equipo
   inicio_IP();         //Reinico los datos IP
   //estodebe hacerse en el reset
   beep_on=off;      //desactivo beep sirena
   state=INIT;
   serial1=&SERIAL1[0];
   serial2=&SERIAL2[0];
   memset(serial1,0x00,22);   //pongo en cero el buffer BLNK
   memset(serial2,0x00,22);   //pongo en cero el buffer BLNK
   timeon1=150;
   timeoff1=2500;
   timeon2=500;
   timeoff2=3000; //tiempos de encendidos on-off

}

/*
 Función que Resetea parámetros Control Box
*/
void res_cbox()
{
      inicio_cbox();
      //inicializo puntero de tarjetas
      puntin_tjtas=0;
      puntout_tjtas=0;
//inicializo puntero de eventos
      puntin_event=0;
      puntout_event=0;
//inicializo puntero de SMS
      puntin_SMS=0;
      puntout_SMS=0;
//inicializo Alarmas
      beep_desactivar=off;            //alarma  Desactivada
      alrm_armd=off;
      alrm_disp=off;
      ent_no_asociada=myHard.ent_no_asociada;   //tomo las ultimas cuatro entradas
      estado_alrm=reposo;
      strcpy(STRING_TX,"Reset OK\r\n");
}

/************************************************
 Función que realiza el Seteo Inicial de Control Box
 cargando los datos por Default
************************************************/
void set_cbox()
{

/***********Seteo datos equipo******************/
      writeUserBlock(EQP_HAB,&myHard_default,sizeof(myHard_default));  //guardo setup Eq
      set_IP();       //Configuro la ip del Equipo

/***********Seteo datos celulares **************/
      memset(buffer_tel,0x00,270);   //pongo en cero el buffer BLNK
      memcpy(buffer_tel,&myTelefono_default, sizeof(myTelefono_default));
      writeUserBlock(TEL_HAB,buffer_tel,270);

/***********Seteo datos tarjetas **************/
      memset(&Buffer_trj,0x00,380);   //pongo en cero el buffer BLNK
      memcpy(&Buffer_trj,&myTarjetas_default, sizeof(myTarjetas_default));
      writeUserBlock(TRJ_HAB,Buffer_trj,380);

/************Seteo datos Entradas***************/
      writeUserBlock(ENT_HAB,&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(1*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(2*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(3*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(4*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(5*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(6*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(7*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(8*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(9*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(10*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(11*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(12*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(13*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(14*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
      writeUserBlock(ENT_HAB+(15*sizeof(Entrada)),&myEntrada_default, sizeof(Entrada));
/************Seteo datos Salidas***************/
      writeUserBlock(SAL_HAB,&mySalidas_default, sizeof(salidas));
      writeUserBlock(SAL_HAB+(1*sizeof(salidas)),&mySalidas_default, sizeof(salidas));
      writeUserBlock(SAL_HAB+(2*sizeof(salidas)),&mySalidas_default, sizeof(salidas));
      writeUserBlock(SAL_HAB+(3*sizeof(salidas)),&mySalidas_default, sizeof(salidas));
      writeUserBlock(SAL_HAB+(4*sizeof(salidas)),&mySalidas_default, sizeof(salidas));
      writeUserBlock(SAL_HAB+(5*sizeof(salidas)),&mySalidas_default, sizeof(salidas));
      writeUserBlock(SAL_HAB+(6*sizeof(salidas)),&mySalidas_default, sizeof(salidas));
      writeUserBlock(SAL_HAB+(7*sizeof(salidas)),&mySalidas_default, sizeof(salidas));

/********* Seteo datos Entradas/Salidas ********/
      writeUserBlock(ENS_HAB,&myEntrada_Salida_default, sizeof(Entrada_Salida));
      writeUserBlock(ENS_HAB+(1*sizeof(Entrada_Salida)),&myEntrada_Salida_default, sizeof(Entrada_Salida));
      writeUserBlock(ENS_HAB+(2*sizeof(Entrada_Salida)),&myEntrada_Salida_default, sizeof(Entrada_Salida));
      writeUserBlock(ENS_HAB+(3*sizeof(Entrada_Salida)),&myEntrada_Salida_default, sizeof(Entrada_Salida));
      writeUserBlock(ENS_HAB+(4*sizeof(Entrada_Salida)),&myEntrada_Salida_default, sizeof(Entrada_Salida));
      writeUserBlock(ENS_HAB+(5*sizeof(Entrada_Salida)),&myEntrada_Salida_default, sizeof(Entrada_Salida));
      writeUserBlock(ENS_HAB+(6*sizeof(Entrada_Salida)),&myEntrada_Salida_default, sizeof(Entrada_Salida));
      writeUserBlock(ENS_HAB+(7*sizeof(Entrada_Salida)),&myEntrada_Salida_default, sizeof(Entrada_Salida));

/************Seteo datos SMS Entradas***********/
      writeUserBlock(SMS_HAB,&mySMS_Entrada_default, sizeof(SMS_Entrada));
      writeUserBlock(SMS_HAB+(1*sizeof(SMS_Entrada)),&mySMS_Entrada_default, sizeof(SMS_Entrada));
      writeUserBlock(SMS_HAB+(2*sizeof(SMS_Entrada)),&mySMS_Entrada_default, sizeof(SMS_Entrada));
      writeUserBlock(SMS_HAB+(3*sizeof(SMS_Entrada)),&mySMS_Entrada_default, sizeof(SMS_Entrada));
/************Seteo datos Alarmas Habilitadas ***/
      writeUserBlock(ALM_HAB,&myActivacion_default, sizeof(Activacion));
      writeUserBlock(ALM_HAB+(1*sizeof(Activacion)),&myActivacion_default, sizeof(Activacion));
      writeUserBlock(ALM_HAB+(2*sizeof(Activacion)),&myActivacion_default, sizeof(Activacion));
      writeUserBlock(ALM_HAB+(3*sizeof(Activacion)),&myActivacion_default, sizeof(Activacion));
/************Seteo datos Macros****************/
      writeUserBlock(MAC_HAB,&myMacro_default,sizeof(MyMacro));
      writeUserBlock(MAC_HAB+(sizeof(MyMacro)),&myMacro_default1,sizeof(MyMacro));



      /*strcpy(buffer_sms,"Estado Alarma: Activada \n");
      strcpy(buffer_sms+(1*30),"Estado Alarma: Desactivada \n\0");
      strcpy(buffer_sms+(2*30),"Luces de Acceso Encendidas \n\0");
      strcpy(buffer_sms+(3*30),"ha Activado la Alarma \0");
      strcpy(buffer_sms+(4*30),"ha Desactivado la Alarma \0");
      strcpy(buffer_sms+(5*30),"Llamada de Control Completada\0");
      writeUserBlock(SMS_HAB,Buffer_sms,300); */
      res_cbox();
      Set_SIM=on;          //seteo el SIM
      strcpy(STRING_TX,"Seteo OK\r\n");
}
/***************************************************************
 Conversión de tarjeta para Tx datos
****************************************************************/
void convertdato_tx(char dato[3])
{
   for(l=0;l<3; l++){
       tempo=(dato[l]&0xf0);
       tempo=tempo>>4;
       if(tempo<10){
         tempo=tempo+48;
       }else{
         tempo=tempo+55;
       }
       TARJ[(l*2)]=tempo;
       tempo=(dato[l]&0x0f);
       if(tempo<10){
         tempo=tempo+48;
       }else{
         tempo=tempo+55;
       }
       TARJ[(l*2)+1]=tempo;
   }
   TARJ[6]=0;
}


/*
 Función que da de baja un telefono en el buffer
*/
void ver_pas()
{
      if(strstr(answ,"control")){
         serCputs("pas OK\r\n");
      }else{
         serCputs("pas ER\r\n");
      }
}

/************************************************
Activar Macro----> num_mac= numero de macro
************************************************/
void Activar_Macro(int num_mac)
{
   memcpy(&MyMacro,Buffer_mac+(num_mac*sizeof(MyMacro)),sizeof(MyMacro));
   if (MyMacro.t0!=0){
      TS[0]=t0+MyMacro.t0;
      set(porta,0);
   }
    if (MyMacro.t1!=0){
      TS[1]=t0+MyMacro.t1;
      set(porta,1);
   }
   if (MyMacro.t2!=0){
      TS[2]=t0+MyMacro.t2;
      set(porta,2);
   }
   if (MyMacro.t3!=0){
      TS[3]=t0+MyMacro.t3;
      set(porta,3);
   }
   if (MyMacro.t4!=0){
      TS[4]=t0+MyMacro.t4;
      set(porta,4);
   }
   if (MyMacro.t5!=0){
      TS[5]=t0+MyMacro.t5;
      set(porta,5);
   }
   strcpy(buff_almacSMS.mensaje,MyMacro.descripcion);
   strcat(buff_almacSMS.mensaje," Activada\n");

}






/************************************************
Armar Mensaje de estado alarma para SMS
************************************************/
void Estado_Alarma()
{
   memset(string,0x00,100);
   switch(estado_alrm){
      case armado:    //alarma Activada
         strcpy(buff_almacSMS.mensaje,"Seguridad Activada\n");
         strcat(buff_almacSMS.mensaje,nombre_act);
         strcat(buff_almacSMS.mensaje," ");
         conversion(t_activacion);
         strcat(buff_almacSMS.mensaje,th);
         strcat(buff_almacSMS.mensaje,"\n");
         Estado_Inputs_Alrm();
         //strcat(buff_almacSMS.mensaje,"\n");
         break;
      case disparada:    //alarma Disparada
         strcpy(buff_almacSMS.mensaje,"Alarma Disparada\n");
         conversion(tds);
         strcat(buff_almacSMS.mensaje,th);
         strcat(buff_almacSMS.mensaje,"\n");
         Estado_Inputs_Alrm();
         //strcat(buff_almacSMS.mensaje,"\n");
         break;
      default:    //alarma Desactivada
         strcpy(buff_almacSMS.mensaje,"Seguridad Desactivada\n");
         strcat(buff_almacSMS.mensaje,nombre_act);
         strcat(buff_almacSMS.mensaje," ");
         conversion(t_activacion);
         strcat(buff_almacSMS.mensaje,th);
         strcat(buff_almacSMS.mensaje,"\n\n");
         Estado_Inputs_NoAlarma();
   }
   strcat(buff_almacSMS.mensaje,"Control Box");
}

/************************************************
 Función que Arma(activa) la Alarma
 modo -> "localmente, remotamente, etc"
 origen-> "WEB, SMS, RFID"
 alarma -> numero de alarma activada
 Estados de almacenamiento
 F -> Total RFID
 G -> Sleep RFID
 H -> Alm 1 WEB
 I -> Alm 2 WEB
 J -> Alm 3 WEB
 K -> Alm 4 WEB
 L -> Total SMS

 ************************************************/
enum{WEB=0, SMS=1, RFID=2};

void activar_alarma(char *modo, int origen, int alarma)
{
 //tomo las variables de entradas habilitadas y activas(alrm_set_flag)
   tipo_alarma=alarma;
   estado_alrm=armado;
   alrm_flag=off;
   //tiempo de activado
   timeon1=100;
   timeoff1=1000;
   lr1=off;        //reseteo Led's
   timeon2=100;
   timeoff2=1000;
   lr2=off;        //reseteo Led's

   if (origen==RFID){       //Viene activacion de tarjeta
      tipo_alarma=0;
      buff_almac.estdo='F';
      if (sensor){	//Lector PPAL
      	if(myHard.tipo_lector==1){    //Lector interno
	         if(!z1){    //No se abrió la puerta
	            tipo_alarma=1;
	            buff_almac.estdo='G';
	         }
	      }else{    //Lector Externo
	         if(z1){     //Se abrió la puerta
	            tipo_alarma=1;
	            buff_almac.estdo='G';
	         }
	      }
      }else{   //Lector secundario
         if(!z2){    //No se abrió la puerta
	            tipo_alarma=1;
	            buff_almac.estdo='G';
         }
      }
   }else if (origen==SMS){   //Viene activacion de mensaje de texto
      tipo_alarma=0;
      buff_almac.estdo='L';
   }else{   //Viene activacion de Web
      strcpy(nombre_act,"-Web-");
      if (tipo_alarma==0) {
       	buff_almac.estdo='H';
      } else if (tipo_alarma==1) {
       	buff_almac.estdo='I';
      } else if (tipo_alarma==2) {
       	buff_almac.estdo='J';
      } else {
       	buff_almac.estdo='K';
      }
   }

   set_alrm = myMasc_activacion[tipo_alarma]; 		//Tomo la Alarma elegida
   alrm_set_flag=INPUT & set_alrm;     // Filtro por entradas alarma
/// Busco el tipo de activación elegida
   readUserBlock(&myActivacion,ALM_HAB+(tipo_alarma*sizeof(Activacion)),sizeof(Activacion));

   t_activacion=t0;     //guardo fecha y hora activación

   conversion(t_activacion);
/// almaceno evento

   buff_almac.fcha=t0;
   memset(buff_almac.trjta,0x00,6);
   x=strlen(nombre_act);
   if(x > 6)
   	x=6;
   strncpy(buff_almac.trjta,nombre_act,x);
   almacenar_event(&buff_almac);


   if (myActivacion.sms=='S'){

      strcpy(buff_almacSMS.mensaje,nombre_act);
   	strcat(buff_almacSMS.mensaje," Activo Seg. ");
      strcat(buff_almacSMS.mensaje,myActivacion.nemonico);
   	strcat(buff_almacSMS.mensaje," ");
   	strcat(buff_almacSMS.mensaje,modo);
      strcat(buff_almacSMS.mensaje,th);
   	strcat(buff_almacSMS.mensaje,"\n");
   	Estado_Inputs_Alrm();  //Entradas activas/desactivadas
      //Estado_Inputs_NoAlarma();
   	strcat(buff_almacSMS.mensaje,"ControlBox\0");
      //printf("Activacion %s\r\n", buff_almacSMS.mensaje);
      Tel_categoria=buscar_nom_cel(nombre_act);
      if (Tel_categoria > 1){ //Busco el Telefono con Cat > 1
      	strcpy(buff_almacSMS.numero_envio, myTelefono.numero);
         almacenar_SMS(&buff_almacSMS);

      }
      fn_SMS_Cat(1,&buff_almacSMS);
   }else{
      if (origen==SMS){
         strcpy(buff_almacSMS.mensaje,nombre_act);
   		strcat(buff_almacSMS.mensaje," Activo Seg. ");
      	strcat(buff_almacSMS.mensaje,myActivacion.nemonico);
   		strcat(buff_almacSMS.mensaje," ");
   		strcat(buff_almacSMS.mensaje,modo);
      	strcat(buff_almacSMS.mensaje,th);
   		strcat(buff_almacSMS.mensaje,"\n");
   		Estado_Inputs_Alrm();  //Entradas activas/desactivadas
   		strcat(buff_almacSMS.mensaje,"\n");
   		//Estado_Inputs_NoAlarma();
   		strcat(buff_almacSMS.mensaje,"ControlBox\0");
         Tel_categoria=buscar_nom_cel(nombre_act);
         if (Tel_categoria > 0){ //Busco el Telefono con Cat > 1
	         strcpy(buff_almacSMS.numero_envio, myTelefono.numero);
	         almacenar_SMS(&buff_almacSMS);

	      }
      }
   }
/*********Activo Beep Activación ***************/
   if (!tipo_alarma){       //sólo para activación total
 //     printf("cuic activacion\r\n");
      beep_on=on;    //activo Beep Sirena
      T_beep1=30;
      T_beep2=200;
      T_beep3=30;
   }

   web=1;
 }

/************************************************
 Función que Desactiva la Alarma
 modo -> "localmente, remotamente, etc"
 origen-> "WEB, SMS, RFID"
Estados de almacenamiento
 S -> RFID
 T -> WEB
 U -> SMS


************************************************/
void desactivar_alarma(char *modo, int origen)
{
   estado_alrm=reposo;
   /*tiempo de reposo*/
   timeon1=1500;
   timeoff1=50;
   lr1=off;        //reseteo Led's
   timeon2=1500;
   timeoff2=50;
   lr2=off;        //reseteo Led's
   alrm_disp=off;  //reseteo sirena
   alrm_flag=off;
   est_alrm=off;
   Llamada_OK=off;
   beep_desactivar=off; //corto beep desactivación                  //Llamada al Nº1
   PORTERO1=on;
   t_activacion=t0;       //almaceno horario de desactivación
   conversion(t_activacion);

   if(origen == RFID){
   	buff_almac.estdo='S';
   }else if (origen == SMS) {
      buff_almac.estdo='U';
   }else{
      buff_almac.estdo='T';
   }

	buff_almac.fcha=t0;
   memset(buff_almac.trjta,0x00,6);
   x=strlen(nombre_act);
   if(x > 6)
   	x=6;
   strncpy(buff_almac.trjta,nombre_act,x);
   almacenar_event(&buff_almac);

   if (myActivacion.sms=='S'){

      strcpy(buff_almacSMS.mensaje,nombre_act);
      strcat(buff_almacSMS.mensaje," ha Desactivado la Seguridad ");
   	strcat(buff_almacSMS.mensaje,myActivacion.nemonico);
   	strcat(buff_almacSMS.mensaje," ");
   	strcat(buff_almacSMS.mensaje,modo);
      strcat(buff_almacSMS.mensaje,th);
      strcat(buff_almacSMS.mensaje,"ControlBox\0");

      Tel_categoria=buscar_nom_cel(nombre_act);
      if (Tel_categoria > 1){ //Busco el Telefono con Cat > 1
      	strcpy(buff_almacSMS.numero_envio, myTelefono.numero);
         almacenar_SMS(&buff_almacSMS);

      }
      fn_SMS_Cat(1,&buff_almacSMS);
   }else{
      if (origen==SMS){
         strcpy(buff_almacSMS.mensaje,nombre_act);
   		strcat(buff_almacSMS.mensaje," ha Desactivado la Seguridad ");
      	strcat(buff_almacSMS.mensaje,myActivacion.nemonico);
   		strcat(buff_almacSMS.mensaje," ");
   		strcat(buff_almacSMS.mensaje,modo);
         strcat(buff_almacSMS.mensaje,"ControlBox\0");
         Tel_categoria=buscar_nom_cel(nombre_act);
         if (Tel_categoria > 0){ //Busco el Telefono con Cat > 1
	         strcpy(buff_almacSMS.numero_envio, myTelefono.numero);
	         almacenar_SMS(&buff_almacSMS);

	      }
      }
   }

/*********Activo Beep Desactivación ************/
   if (!tipo_alarma){       //sólo para activación total
   //   printf("cuic desactivacion\r\n");
      beep_on=on;    //activo Beep Sirena
      T_beep1=30;
      T_beep2=0;
      T_beep3=0;
   }
   web=1;

}

/************** VARIABLES WEB ******************/
myb1(HttpState* state)
{
  //myTCPtick(&socket,&state,&astate);
/*  if(*(http_urldecode(buffer,"kx",16))) { // decode data, ignore empty strings
   	strcpy(caja_de_texto,buffer);
   }
   strcpy(&caja_de_texto,"ahivoy");
   printf("myb1::: %s\r\n",buffer);*/
   if (flag_activar){
      activar_alarma("",WEB,0);
 //     printf("activé alarma 1\r\n");
      ent_sal=1;
      CambioEnt=1;
      Hab_CambioEnt=1;
      flag_activar=0;
      strcpy(ent_sal_web,"Ir&#32a&#32Salidas");
      web=1;
      Hab_Alarmas=1;
   }else{
   	Cambiar_Salidas(0);
   }
   cgi_redirectto(state,REDIRECTOK);
   return(0);
}
myb2(HttpState* state)
{
   Cambiar_Salidas(1);
   cgi_redirectto(state,REDIRECTOK);
   return(0);
}

myb3(HttpState* state)
{
 //   printf("presioné boton 3\r\n");
   if (flag_activar){
  //    printf("activé alarma 2\r\n");
      activar_alarma("",WEB,1);
      ent_sal=1;
      CambioEnt=1;
      Hab_CambioEnt=1;
      flag_activar=0;
      strcpy(ent_sal_web,"Ir&#32a&#32Salidas");
      web=1;
      Hab_Alarmas=1;
   }else{
   	Cambiar_Salidas(2);
   }
   cgi_redirectto(state,REDIRECTOK);
   return(0);
}

myb4(HttpState* state)
{

   if (flag_activar){
    //  printf("activé alarma 3\r\n");
      activar_alarma("",WEB,2);
      ent_sal=1;
      CambioEnt=1;
      Hab_CambioEnt=1;
      flag_activar=0;
      strcpy(ent_sal_web,"Ir&#32a&#32Salidas");
     web=1;
      Hab_Alarmas=1;
   }else{
   	Cambiar_Salidas(3);   //inhabilité salida 4 funciona sólo con portero
   }
   cgi_redirectto(state,REDIRECTOK);
   return(0);
}
myb5(HttpState* state)
{
   Cambiar_Salidas(4);      //Habilito portero 1
   //PORTERO1=1;
   cgi_redirectto(state,REDIRECTOK);
   return(0);
}
myb6(HttpState* state)
{
   if (flag_activar){
      //printf("activé alarma 4\r\n");
      activar_alarma("",WEB,3);
      ent_sal=1;
      CambioEnt=1;
      Hab_CambioEnt=1;
      flag_activar=0;
      strcpy(ent_sal_web,"Ir&#32a&#32Salidas");
      web=1;
      Hab_Alarmas=1;
   }else{
      PORTERO1=on;
      printf("Toco web\r\n");
      //Cambiar_Salidas(5);
   }
   cgi_redirectto(state,REDIRECTOK);
   return(0);
}
myb7(HttpState* state)
{
   Cambiar_Salidas(6);
   cgi_redirectto(state,REDIRECTOK);
   return(0);
}
myb8(HttpState* state)
{
   Cambiar_Salidas(7);
   cgi_redirectto(state,REDIRECTOK);
   return(0);
}



/************************************************
Función que controla el boton de Activar
************************************************/

myb9(HttpState* state)
{
   if(estado_alrm==reposo){    //si está en reposo entro en proceso de activacion
    	//printf("activar en proceso 0\r\n");
   	if (!flag_activar){     //bandera de activación
	      //printf("activar en proceso 1\r\n");
	      flag_activar=1;
	      //cambio texto botones
	      sprintf(Btn_Texto[0],"%s",myNemo_activacion[0]);
         strcpy(Btn_Color[0],"black");
         //sprintf(Btn_Texto[2],"&nbsp;%s",myNemo_activacion[1]);
         sprintf(Btn_Texto[2],"%s",myNemo_activacion[1]);
         strcpy(Btn_Color[2],"black");
         sprintf(Btn_Texto[3],"%s",myNemo_activacion[2]);
         strcpy(Btn_Color[3],"black");
         sprintf(Btn_Texto[5],"%s",myNemo_activacion[3]);
         strcpy(Btn_Color[5],"black");
         strcpy(ent_sal_web,"Cancelar");
	      Hab_Alarmas=0;
         Btn_Habilitado[0]=0;
         Btn_Habilitado[2]=0;
         Btn_Habilitado[3]=0;
         Btn_Habilitado[5]=0;
	   }else{
	      //printf("activar en curso\r\n");
	      flag_activar=0;
	      //activar=1;
	      web=1;
	      Hab_Alarmas=1;
	   }
   }else{    //si no está en reposo desactivo
      flag_activar=0;
      Hab_Alarmas=1;
      strcpy(nombre_act,"-Web-");
      desactivar_alarma("\n",WEB);
      web=1;
   }
   cgi_redirectto(state,REDIRECTOK);
   //printf("presione boton\r\n");
   return(0);
}

/************************************************
Función del boton de Cambio entradas/salidas
************************************************/

myb0(HttpState* state)
{

   if (ent_sal==1||flag_activar==1){  // ent_sal -> entradas =1 Salidas=0
/**Cambio a Salidas si estoy en entradas o Cancelar de Activacion*/
      ent_sal=0;
      Hab_CambioEnt=0;    //deshabilito cambio de entradas 0
      strcpy(ent_sal_web,"Ir&#32a&#32Entradas");
      flag_activar=0;     //Pongo en cero la bandera de activar

      for(n=0;n<8;n++){    //Actualizo texto de botones
         if (Salida_Operativa[n]=='N')
            Btn_Habilitado[n]=1;
         else
            Btn_Habilitado[n]=0;
      }
   }else{              // analisis de Salidas
     // printf("voy a entradas \r\n");
      ent_sal=1;
      CambioEnt=1;
      Hab_CambioEnt=1;               //Habilito el cambio de entradas  1
      strcpy(Text_CambioEnt,"Ir&#32a&#32Entradas&#32&#57-16");
 //     strcpy(Col_CambioEnt,"green");
      for(n=0;n<8;n++){
         Btn_Habilitado[n]=1;
      }
      strcpy(ent_sal_web,"Ir&#32a&#32Salidas");

   }
   //actualizar_salidasWeb(7);
   Hab_Alarmas=1;
   web=1;
   habilitar_telnet=1;
   reiniciar_tcp=1;
   cgi_redirectto(state,REDIRECTOK);
   return(0);
}



/************************************************
 Lectura de inputs Optoacopladas
************************************************/
void lectura_inputs()
{
//leo los datos de la entradas s/optoacoplador
  INPUT =RdPortE(0x0001);
  INPUT=INPUT<<8;
//Leo las entradas con optoacoplador
  INPUT=INPUT|RdPortE(0x0005);;
  INPUT=INPUT & ent_operativas;   //Filtro la entradas con ent_operativas
  INPUT=INPUT ^ ent_tipo;  //Convierto por tipo de entradas
 // printf( "Entradas %04X \r\n",INPUT);
  INPUTB=RdPortE(0x0004);   //leo los datos de la entradas para EE
  //if (BIT(&INPUTEE,7)){     //Analisis de estado EE - Hard Viejo
  if (BIT(&INPUTB,5)){     //Analisis de estado EE
      //printf("Energia EE ON\r\n");
      alrm_ee=off;
      alrm_bat=off;
  }else{                   //EE OFF
      //printf("Energia EE OFF\r\n");
      alrm_ee=on;
      //analisis batería. Lo hago sólo cuando hay corte de EE
      if (BIT(&INPUTB,4)){
         alrm_bat=off;
      }else{
         alrm_bat=on;
      }
  }

/******* Control para activación Nocturna ******/
   if (estado_alrm==set_armado){
      if (!BIT(&INPUT,0)){ // Si en algún momento se abre la puerta
        z1=on;
      }
      if (!BIT(&INPUT,1)){ // Si en algún momento se abre la puerta
        z2=on;
      }
   }
/******  Abro portero con entrada 16         ***/
      if (BIT(&INPUT,15)){ // Si en algún momento se abre la puerta
         //printf("\r\nPortero ON\r\n");
         PORTERO1=on;
      }
/******  Cierro portero con entrada 1        ***/
		if (!BIT(&INPUT,0)){ // Si en algún momento se abre la puerta
         //printf("\r\nPortero OFF\r\n");
         PORTERO1=off;
      }


/************************************************
Control de cambio  en valores de entradas
************************************************/
	INPUT_COMP = INPUT ^ INPUT_OLD;
   if(INPUT_COMP){      //veo si hubo cambios en las entradas
   	for(i=0; i<15; i++){    //recorro las 16 entradas
         if (BIT(&INPUT_COMP,i)){	//Si es uno Hubo un cambio de entrada
            if(BIT(&ent_no_asociada,i)){   //entradas no seteadas para alarma
                  //printf("Cambio entrada NO Asociada\r\n");
               if (fn_SMS_asociado(i+1)==1){
                  if (BIT(&INPUT,i)){   // normal/falla
                     if (strlen(mySMS_Entrada.activacion)){
                        conversion(t0);
                        sprintf(buff_almacSMS.mensaje,"%s\n%s\nControlBox",
                        			mySMS_Entrada.activacion,th);
                        fn_SMS_Cat(1,&buff_almacSMS);
                     }
                  }else{
                     if (strlen(mySMS_Entrada.desactivacion)){
                        conversion(t0);
                        sprintf(buff_almacSMS.mensaje,"%s\n%s\nControlBox",
                        			mySMS_Entrada.desactivacion,th);
                        fn_SMS_Cat(1,&buff_almacSMS);
                     }
                  }
               }
            }
	      }
	   }
   	INPUT_OLD = INPUT;	//Igualo entradas con entradas OLD
      web=1;
   }
}

/************************************************
Función que arma mensaje de Activacion salidas
************************************************/
nodebug void fn_ArmarSMSActivacion(int j)
{
	memcpy(&salidas,buffer_sal+(38*j),38);
   strcpy(buff_almacSMS.mensaje,salidas.descripcion);
   if (Salida_Operativa[j]=='S'){
   	set(porta,j);
      TS[j]=t0+SalidaTiempo[j];
      strcat(buff_almacSMS.mensaje, " Activada\n");
      web=1;
   }else{
   	strcat(buff_almacSMS.mensaje," NO Acepta Operacion Manual.\n");
   }
}

/************************************************
Función que arma Mensaje de Desactivacion salidas
************************************************/
nodebug void fn_ArmarSMSDesactivacion(int j)
{
	memcpy(&salidas,buffer_sal+(38*j),38);
   strcpy(buff_almacSMS.mensaje,salidas.descripcion);
   res(porta,j);
   strcat(buff_almacSMS.mensaje, " Desactivada\n");
   web=1;

}
/************************************************
 Accionamiento de Salidas por Relés
************************************************/
nodebug void salida(char *dato_rx)
{

    if (strstr(dato_rx,"s1 on")){            //Reset General
       fn_ArmarSMSActivacion(0);

    }else if (strstr(dato_rx,"s1 of")){
        fn_ArmarSMSDesactivacion(0);

    }else if (strstr(dato_rx,"s2 on")){
        fn_ArmarSMSActivacion(1);

    }else if (strstr(dato_rx,"s2 of")){
       fn_ArmarSMSDesactivacion(1);

    }else if (strstr(dato_rx,"s3 on")){
        fn_ArmarSMSActivacion(2);

    }else if (strstr(dato_rx,"s3 of")){
        fn_ArmarSMSDesactivacion(2);

    }else if (strstr(dato_rx,"s4 on")){
        fn_ArmarSMSActivacion(3);

    }else if (strstr(dato_rx,"s4 of")){
       fn_ArmarSMSDesactivacion(3);

    }else if (strstr(ANSW,"s5 on")){
        fn_ArmarSMSActivacion(4);

    }else if (strstr(dato_rx,"s5 of")){
       fn_ArmarSMSDesactivacion(4);

    }else if (strstr(dato_rx,"s6 on")){
        fn_ArmarSMSActivacion(5);

    }else if (strstr(dato_rx,"s6 of")){
       fn_ArmarSMSDesactivacion(5);

    }else if (strstr(dato_rx,"s7 on")){
        set(porta,6);
        strcpy(buff_almacSMS.mensaje,"Salida 7 Activada.\n");
    }else if (strstr(dato_rx,"s7 of")){
        res(porta,6);
        strcpy(buff_almacSMS.mensaje,"Salida 7 Desactivada.\n");
    }else if (strstr(dato_rx,"s8 on")){
        set(porta,7);
        strcpy(buff_almacSMS.mensaje,"Sirena Activada.\n");
    }else if (strstr(dato_rx,"s8 of")){
        res(porta,7);
        strcpy(buff_almacSMS.mensaje,"Sirena Desactivada.\n");
    }else{
         //busco por nombre corto
         for(i=0; i<6; i++){
            memcpy(&salidas,buffer_sal+(i*38),38);
            strcpy(texto_bsq,salidas.nemonico);
            texto_bsq[sizeof(salidas.nemonico)]=0;
            //paso nombre a minuscula
            for(j=0; j< strlen(texto_bsq); j++){
               texto_bsq[j]=tolower(texto_bsq[j]);
            }

            if (strstr(dato_rx,texto_bsq)) {
               strcat(texto_bsq," on");
               if (strstr(dato_rx,texto_bsq)) {
                  fn_ArmarSMSActivacion(i);
                  goto fin_salida;
               }
               texto_bsq[strlen(texto_bsq)-1]='f';
               if (strstr(dato_rx,texto_bsq)) {
                  fn_ArmarSMSDesactivacion(i);
                  goto fin_salida;
               }
            }
         }
         //Macros
         for(i=0; i<2; i++){
            memcpy(&MyMacro,&Buffer_mac+(i*54),54);
            strcpy(texto_bsq,MyMacro.nemonico);
            texto_bsq[sizeof(salidas.nemonico)]=0;
            for(j=0; j< strlen(texto_bsq); j++){
               texto_bsq[j]=tolower(texto_bsq[j]);
            }
            if (strstr(dato_rx,texto_bsq)) {
               Activar_Macro(i);
               goto fin_salida;
            }
         }

         strcpy(buff_almacSMS.mensaje,"Error de Red reintente la Operacion.\n");
         INI=1;
     }
fin_salida:
    // armar_SMS(&buff_almacSMS);
}

/************************************
 Sms de respuesta para comandos no autorizados.
************************************/
void SMS_noauto(char comando[5])
{
   strcpy(buff_almacSMS.mensaje,"\"");
   strcat(buff_almacSMS.mensaje,comando);
   strcat(buff_almacSMS.mensaje,SINAUTO);
   armar_SMS(&buff_almacSMS);
}
/************************************
 Procesamiento de mensajes recibidos.
************************************/
void proceso_rx_SMS(char *rec_sms)
{
//paso a lower case y saco CR y LF
   n=0;
   for(i=0; i< strlen(rec_sms); i++){
      if (rec_sms[i]!=13 && rec_sms[i]!=10 ){
         if(n==0){
           if(rec_sms[i]!=32){    //elimino los espacios al inicio
               rec_sms[n]=tolower(rec_sms[i]);
               n++;
           }
         }else{
           rec_sms[n]=tolower(rec_sms[i]);
            n++;
       }
      }
   }
   rec_sms[n]=0;
   strncpy(comando,rec_sms,4);
   Comando[4]=0;
   strcpy(buff_almacSMS.numero_envio, numero_sms);
   if (!strcmp(comando,"sete")){
      Verificar_Telefono_Comandos(); //Busco si es categoria Cero
      if (Tel_categoria == 0){     //telefono autorizado
         strcpy(rec_sms,rec_sms+3);
         for(i=0; i< strlen(rec_sms); i++){
            rec_sms[i]=toupper(rec_sms[i]);
         }
         strcat(rec_sms,"\n");
         if (rec_sms[3]=='E'){         //seteo Entradas
            rx_ent();
         }else if (answ[3]=='C'){   //Seteo Celulares
            rx_telef();
         }else if (rec_sms[3]=='S'){
            rx_sal();
         }else if (rec_sms[3]=='T'){
            rx_trj();
         }
         strcpy(buff_almacSMS.mensaje, STRING_TX);
         almacenar_SMS(&buff_almacSMS);
      }
      return;
   }else if (!strcmp(comando,"rese")){
      Verificar_Telefono_Comandos(); //Busco si es categoria Cero
      if (Tel_categoria == 0){     //telefono autorizado
         res_cbox();
         strcpy(buff_almacSMS.mensaje, STRING_TX);
         almacenar_SMS(&buff_almacSMS);
      }
      return;
   }else if (!strcmp(comando,"setu")){
      Verificar_Telefono_Comandos(); //Busco si es categoria Cero
      if (Tel_categoria == 0){     //telefono autorizado
         set_cbox();
         strcpy(buff_almacSMS.mensaje, STRING_TX);
         almacenar_SMS(&buff_almacSMS);
      }
      return;
   }else if (!strcmp(comando,"desa")){
       if (Tel_categoria <= 2){     //telefono autorizado
         if(estado_alrm!=reposo){
            strcpy(nombre_act,Tel_nombre);
            desactivar_alarma("x SMS\n",SMS);
         }else{
            memset(buff_almacSMS.mensaje,0x00,10);   //pongo en cero buffer String
            Estado_Alarma();
            almacenar_SMS(&buff_almacSMS);
         }
       }else{
         goto sms_noauto;
       }
   }else if (!strcmp(comando,"reac")){
       if (Tel_categoria <= 2){     //telefono autorizado
         if(estado_alrm==disparada){
            estado_alrm=armado;
            alrm_disp=off;
            alrm_flag=off;
            web=1;
            strcpy(nombre_act,Tel_nombre);
            //strcpy(buff_almacSMS.mensaje,Tel_nombre);
            strcpy(buff_almacSMS.mensaje,"Alarma Reactivada\n");
            strcat(buff_almacSMS.mensaje,nombre_act);
            strcat(buff_almacSMS.mensaje," ");
            conversion(t_activacion);
            strcat(buff_almacSMS.mensaje,th);
            strcat(buff_almacSMS.mensaje,"\n");
            Estado_Inputs_Alrm();
            //strcat(buff_almacSMS.mensaje,"\n");
            //Estado_Inputs_NoAlarma();
            strcat(buff_almacSMS.mensaje,"Control Box");

            if (Tel_categoria>1){ //Busco el Telefono con Cat 1
               strcpy(buff_almacSMS.numero_envio, numero);
               almacenar_SMS(&buff_almacSMS);
            }
            fn_SMS_Cat(1,&buff_almacSMS);
         }else{
            memset(buff_almacSMS.mensaje,0x00,10);   //pongo en cero buffer String
            Estado_Alarma();
            almacenar_SMS(&buff_almacSMS);
         }
       }else{
          goto sms_noauto;
       }
   }else if (!strcmp(comando,"acti")){  //activo la alarma desde el celu
       if (Tel_categoria <= 2){     //telefono autorizado
            if(estado_alrm!=armado){
            strcpy(nombre_act,Tel_nombre);
            z1=on;
            activar_alarma("x SMS\n",SMS,0);
          }else{
            memset(buff_almacSMS.mensaje,0x00,10);   //pongo en cero buffer String
            Estado_Alarma();
            almacenar_SMS(&buff_almacSMS);
         }
      }else{
        goto sms_noauto;
        }
   }else if (!strcmp(comando,"sald")){  //Pido el saldo en Personal
       if (Tel_categoria <= 2){
         serBputs("ATD*150#\r");
       }else{
         goto sms_noauto;
       }
   }else if (!strcmp(comando,"esta")){  //Pido el saldo en Personal
      if (Tel_categoria <= 2){
         strcpy(buff_almacSMS.numero_envio, numero_sms);
         Estado_Alarma();
         almacenar_SMS(&buff_almacSMS);
      }else{
         goto sms_noauto;
      }
   }else if (!strcmp(comando,"sali")){  //Pido el saldo en Personal
      if (Tel_categoria <= 2){
         strcpy(buff_almacSMS.numero_envio, numero_sms);
         sms_sal();
      }else{
         goto sms_noauto;
      }
   }else if (!strcmp(comando,"celu")){  //Pido el saldo en Personal
      if (Tel_categoria <= 2){
         strcpy(buff_almacSMS.numero_envio, numero_sms);
         sms_telef();
      }else{
         goto sms_noauto;
      }
   }else if (!strcmp(comando,"tarj")){  //Pido el saldo en Personal
      if (Tel_categoria <= 2){
         strcpy(buff_almacSMS.numero_envio, numero_sms);
         sms_trj();
      }else{
         goto sms_noauto;
     }
   }else if (!strcmp(comando,"baja")){  //Comando Baja
      if (Tel_categoria <= 2){
         if (strstr(ANSW,"celular")){     //Baja de un celular
            i= buscar_nom_cel(&Tel_nombre);
            strcpy(buff_almacSMS.mensaje,"\"");
            strcat(buff_almacSMS.mensaje,ANSW);
            strcat(buff_almacSMS.mensaje,"\"");
            if(i != -1) {
               if (myTelefono.numero[0]!=' '){
                  myTelefono.numero[0]=' ';
                  memcpy(buffer_tel+(i*sizeof(myTelefono)),&myTelefono, sizeof(myTelefono));
                  strcat(buff_almacSMS.mensaje," se ha ejecutado correctamente.\n");
                  writeUserBlock(TEL_HAB,Buffer_tel,260);
               }else{
                  strcat(buff_almacSMS.mensaje," No se ha ejecutado.\n");
               }
            }else{
               strcat(buff_almacSMS.mensaje," No se ha ejecutado.\n");
            }
         }else if (strstr(ANSW,"tarjeta")){//Baja de un tarjeta
            i=buscar_nom_tarj();
            strcpy(buff_almacSMS.mensaje,"\"");
            strcat(buff_almacSMS.mensaje,ANSW);
            strcat(buff_almacSMS.mensaje,"\"");
            if(i != -1) {
               strcpy(buffer_trj+(i*sizeof(buff_trj)),buffer_trj+((i+1)*sizeof(buff_trj)));
               memset(buffer_trj + strlen(buffer_trj),0x00,(360-strlen(buffer_trj)));   //pongo en cero el buffer BLNK
               writeUserBlock(TRJ_HAB,Buffer_trj,360);
               strcat(buff_almacSMS.mensaje," se ha ejecutado correctamente.\n");
               //writeUserBlock(TRJ_HAB,Buffer_trj,360);
            }else{
               strcat(buff_almacSMS.mensaje," No se ha ejecutado.\n");
            }
         }
         armar_SMS(&buff_almacSMS);
      }else{
         goto sms_noauto;
      }

   }else if (!strcmp(comando,"alta")){  //Comando de alta
      if (Tel_categoria <= 2){
         if (strstr(rec_sms,"celular")){     //Alta de un celular
          i= buscar_nom_cel(&Tel_nombre);
            strcpy(buff_almacSMS.mensaje,"\"");     //pongo entre comillas
            strcat(buff_almacSMS.mensaje,rec_sms);
            strcat(buff_almacSMS.mensaje,"\"");
            if(i != -1) {
               if (myTelefono.numero[0]!='+'){
                  myTelefono.numero[0]='+';
                  memcpy(buffer_tel+(i*sizeof(myTelefono)),&myTelefono, sizeof(myTelefono));
                  strcat(buff_almacSMS.mensaje," se ha ejecutado correctamente.\n");
                  writeUserBlock(TEL_HAB,Buffer_tel,260);
               }else{
               strcat(buff_almacSMS.mensaje," No se ha ejecutado.\n");
               }
            }else{
               strcat(buff_almacSMS.mensaje," No se ha ejecutado.\n");
            }
            armar_SMS(&buff_almacSMS);
         }
      }else{
         goto sms_noauto;
      }

   }else{
      if (Tel_categoria <= 2){
          salida(rec_sms);  /// Voy a la rutina de activación de salidas
          armar_SMS(&buff_almacSMS);
      }else{
        goto sms_noauto;
      }
   }
   return;
sms_noauto:
	SMS_noauto(rec_sms);
}


/************************************************
Rutina de analisis de datos recibidos de la PC
************************************************/

void Proceso()
{
   strncpy(comando,answ,3);
   comando[3]=0;

   //printf("buffer rx %s",bufferrx);
   if (!strcmp(comando,"res")){     //Resetea Punteros e inicio
      res_cbox();
   }else if (!strcmp(comando,"dat")){ //Devuelve datos tarjetas
      buscar_tarj();
   }else if (!strcmp(comando,"evn")){ //Devuelve Datos Eventos
      buscar_event();
   }else if (!strcmp(comando,"vrs")){  //devuelve version software
      strcpy(STRING_TX,VERSION);
   }else if (!strcmp(comando,"set")){ //Setea Flash
      set_cbox();
   }else if (!strcmp(comando,"aln")){  //alta de celulares
      rx_telef();
   }else if (!strcmp(comando,"stn")){  //Transmision de Celulares a la PC
      tx_telef();
   }else if (!strcmp(comando,"alt")){  //AltaTarjetas
      rx_trj();
   }else if (!strcmp(comando,"stt")){  //listado tarjetas individual
      tx_trj();
   }else if (!strcmp(comando,"ale")){  //alta de entradas
      rx_ent();
   }else if (!strcmp(comando,"ste")){  //listado entradas
      tx_ent();
   }else if (!strcmp(comando,"alr")){  //alta de entradas/salidas
      rx_ent_sal();
   }else if (!strcmp(comando,"str")){  //listado entradas/salidas
      tx_ent_sal();
   }else if (!strcmp(comando,"stx")){  //listado SMS_entradas
   	tx_SMS_ent();
   }else if (!strcmp(comando,"alx")){  //alta SMS_entradas
      rx_SMS_ent();
   }else if (!strcmp(comando,"sta")){  //listado Activaciones
   	tx_alarma();
   }else if (!strcmp(comando,"ala")){  //alta Activaciones
      rx_alarma();
   }else if (!strcmp(comando,"als")){  //alta de salida de CBox
      rx_sal();
   }else if (!strcmp(comando,"sts")){  //listado salidas
      tx_sal();
   }else if (!strcmp(comando,"stj")){  //envía a PC datos equipo
      tx_equip();
   }else if (!strcmp(comando,"alj")){  //envía a PC datos equipo
      if (rx_equip()==2){   // si la IP es distinta voy as etear la IP
         set_IP();
      }
  // }else if (strstr(comando,"sip")){  //listado seteo equipo IP
  //       tx_equip_ip();
  // }else if (strstr(comando,"aip")){  //alta IP equipo
  //       rx_equip_ip();
   }else if (!strcmp(comando,"stm")){  //listado MyMacro  0/1
      tx_mac();
   }else if (!strcmp(comando,"alm")){  //alta de MAcro  0/1
      rx_mac();
   }else if (strstr(comando,"ofs")){
         strcpy(STRING_TX,"Proceso Apagado Sim OK\r\n");
         SIM_reset=1;
   }else if (strstr(comando,"ons")){
         strcpy(STRING_TX,"Proceso Inicio Sim OK\r\n");
         SIM_on=1;
   }else if (!strcmp(comando,"vhr")){  //Leer Hora del Rabbit
     // printf("Letura de t0:%lu\r\n",t0);
      conversion(t0);
      strcpy(STRING_TX,TH);
      strcat(STRING_TX,"\r\n");
   }else if (!strcmp(comando,"sld")){  //Buscar Saldo
      serBputs("ATD*150#\r");
      for(n=0; n<8; n++){
         if (traer_telef(n)<=1){ //Busco el Telefono con Cat 1
            break;   //si encuentro 1 voy a llamar
         }
      }
      numero_sms=NUMERO;
   }else if (!strcmp(comando,"hor")){  //Actualizar Hora del rabbit
      recept_time();
   }else if (!strcmp(comando,"est")){  //devuelve Estado Alarma
      Estado_Alarma();
      strcpy(STRING_TX,buff_almacSMS.mensaje);
   }else{
      salida(answ);  /// Voy a la rutina de activación de salidas
      //printf("resultado.%s\r\n",buff_almacSMS.mensaje);
      strcpy(STRING_TX,buff_almacSMS.mensaje);       //no es un comando válido
      web=1;
   }
}

/***********************************************
Recibe y transmite los datos por ethernet
***********************************************/
int application_handler(tcp_Socket *socket,int *estado)
{
int bytes_written,bytes_read;
   if(*estado==READ){         //estado indica si leo o escribo
         if((bytes_read=sock_fastread(socket,bufferrx,1024))!=0){
            //printf("Leer\r\n");
            if (bytes_read < 0){ // detecta desconexión
               return(-1);
            }
            bufferrx[bytes_read]=0; //pone un cero al final de string
            //printf("recepcion TCP %s \r\n",bufferrx);
            *estado=READED;
         }
   }else if(*estado==WRITE){
   //case WRITE:
         len=strlen(buffertx);
         //bytes_written=sock_fastwrite(socket,buffertx,len);
         bytes_written=sock_fastwrite(socket,buffertx,strlen(buffertx));
         if (bytes_written < 0){    // detecta desconexión
            return(-1);
         }
         if(bytes_written!=len)
            memcpy(buffertx,buffertx+bytes_written,len-bytes_written);
         len -= bytes_written;
         if(len==0)
            *estado=READ;
   }else{     // break;
   }
   return(0);
}

/***********************************************
state indica el estado del socket
astate indica si debo leer o escribir
***********************************************/

void myTCPtick(tcp_Socket *socket,int *state,int *astate)
{
   //tcp_tick Nos devuelve si el socket está vivo . =0 socekt cerrado
   if((tcp_tick(socket)==0) && (*state!=INIT)){    // si es cero y no INIT
      *state=INIT;      // falla/cierre de conexión. Cambio Estado
   }
   switch(*state){
   case INIT:
      // Espero a que me llamen
      if (tcp_listen(socket,MY_PORT,0,0,NULL,0) == 0){
            *state=IDLE;
            break;
      }
      *state=LISTEN; //Escuchando...
   case LISTEN:
      if((sock_established(socket)) || (sock_bytesready(socket) > 0)){
         *state=OPEN;
         sock_mode(socket,TCP_MODE_BINARY);
      }
      break;
   case OPEN:
      if(application_handler(socket,astate)<0)
         *state=CLOSE;
      break;
   case CLOSE:
   default:
      sock_close(socket);
      *state=CLOSING;
   case CLOSING:
   case IDLE:
      break;
   }
}



/******************************************
Inicio del Lazo Principal
******************************************/
main()
{
inicio_programa:
//tcp_Socket socket;
//inicio_IP();
//funciones control box
   #if __SEPARATE_INST_DATA__ && (_RK_FIXED_VECTORS)
      interrupt_vector timerb_intvec timerb_isr;
   #else
      SetVectIntern(0x0B, timerb_isr);          // set up ISR
      SetVectIntern(0x0B, GetVectIntern(0xB));  // re-setup ISR to show example of retrieving ISR address
   #endif
   tm_rd(&rtc);         // get time in struct tm
   t1=read_rtc();
   //printf("Lectura de t1:%u\r\n",t1);

   dirxmemo=xalloc(LARGO_BUFFTRJ*11); //reservo espacio para Tarjetas
   //printf("xalloc 1%u \n",dirxmemo);
   dirxalrm=xalloc(LARGO_BUFFTRJ*11); //reservo espacio para Eventos
   //printf("xalloc 1%u \n",dirxalrm);
   //printf("ID Blok %u \n",GetIDBlockSize());
   //printf("ID Blok direcion %u \n",TEL_HAB);


/************************************************
Chequeo si el reinicio se produjo por un WatchDog
Time Out
************************************************/
   if (chkWDTO()) {
      strcpy(buff_almac.trjta,"WDG   ");
      buff_almac.fcha=t0;
      buff_almac.estdo='R';
      almacenar_event(&buff_almac);
   }
/**************Seteo del Virtual WatchDog
Cada cuenta Corresponde a62,5 ms
P.Ej.    ID=VdGetFreeWd(5); corresponde a312.5 ms
************************************************/
   VdReleaseWd(ID);

   //ID=VdGetFreeWd(30);

/**************Chequeo el restore***************/
   readUserBlock(&myHard,EQP_HAB,sizeof(myHard));
   if (strcmp(&myHard.programador,PROGRAMADOR)) {
        set_cbox();
   }
/************************************************/

/************************************************
Chequeo si el reinicio se produjo por un Hard Reset
************************************************/
   if (chkHardReset()) {
      strcpy(buff_almac.trjta,"HrdRst");
      buff_almac.fcha=t0;
      buff_almac.estdo='R';
      almacenar_event(&buff_almac);
// Rutina de verificación corte de alimentación
     if ((t1-t0)>2){            //Almaceno corte energia
         buff_almac.fcha=t0;
         buff_almac.estdo='P';
         strcpy(buff_almac.trjta,"CProgm");
         almacenar_event(& buff_almac);
         buff_almac.fcha=t1;
         buff_almac.estdo='P';
         strcpy(buff_almac.trjta,"RProgm");
         almacenar_event(& buff_almac);
      }
   }

   initialize_timerb();      //inicializo Timer b
   initialize_ports();       //inicializo I/O ports
   inicio_cbox();            //Inicializo variables
   //set_cbox();        //Inicializo todo red 1
/*   th=&TH[0];
   intr=0;
   tv1=off;             //estado Tarj1 off
   tv2=off;             //estado Tarj2 off
   lr1=off;             //estado Led1 reset  off
   lr2=off;             //estado Led2 reset  off
   answ=&ANSW[0];
   ANSW[0]=0;
   comando[0]=0;
   RING_value=0;
   alrm_disp=off;    // alarma no activa
   Llamada_OK=off;   //llamada no activa
   Llamada_SAL=on;   //llamada no activa
   EnvioSMS_ON=off;  // no hay envío SMS
   EnvioSMS_ERROR=0; //Pongo en cero contador de errores sms
   input = &INPUT;
   input_old = &INPUT_OLD;
   porta = &PORTA;
   PORTA=0x00;
   bufferrx[0]=0;
   c = 0;
   serCopen(B_BAUDRATE);
   serBopen(B_BAUDRATE);
   serCputs("Inicio Programa\r\n");
   string=&STRING[0];
   buffer_tel=&Buffer_tel[0];
   buffer_trj=&Buffer_trj[0];
   buffer_sms=&Buffer_sms[0];
   buffer_ent=&Buffer_ent[0]; //buffer mensajes de aviso entradas
   buffer_txc=&Buffer_txc[0];
   buffer_sal=&Buffer_sal[0];
   buffer_envioSMS=&Buffer_envioSMS[0];
   numero=&NUMERO[0];
   numero_sms=&NUMERO_SMS[0];
   NUMERO[13]=0;
   nombre=&NOMBRE[0];
   NOMBRE[10]=0;
   nombre_act=&NOMBRE_ACT[0];    //nombre de quien activó o desactivo alarma
   NOMBRE_ACT[10]=0;
   texto_sms=&TEXTO_SMS[0];
   TEXTO_SMS[30]=0;
   Buffer_trj[360]=0;
   SIM_on=on;            //Encendido SIM
   Set_SIM=off;          //No voy a seter SIM
   SIM_off=off;          //Apagado SIM
   SIM_reset=off;       //no hago reset
   SIM_OK=off;           //SIM no Habilitado
   SMS_delay=1;
   alrm_bat=off;         //Borro alarma de batería baja
   leer_flash();         //lee y actualiza las variables del equipo
   //estodebe hacerse en el reset
   beep_on=off;      //desactivo beep sirena
   state=INIT;
   serial1=&SERIAL1[0];
   serial2=&SERIAL2[0];
   memset(serial1,0x00,22);   //pongo en cero el buffer BLNK
   memset(serial2,0x00,22);   //pongo en cero el buffer BLNK
  */
   strcpy(Btn_Color[0],"green");
   astate=READ;
   //printf ("inicio\n");




   while(1) {  // el loop infinito representa el llamado periódico a este handler

      if (INI==1)         // Verifico que deseo resetear el sistema
      	goto inicio_programa;

      costate{
      	waitfor(DelayMs(10));    		//Tomo un delay de 10 ms
	      t0=read_rtc();        			// Leo hora de inicio de Loop
   	   lectura_inputs();            	//Lectura Entradas
      }

/**************************************************
Rutina para detección de corte de Energía Eléctrica
espero cinco segundos para validar el corte
Sin tener en cuenta el Condensador
**************************************************/
      costate{
         if (alrm_ee){     //leo estado alarma EE
            if (energia){  //ya avisé el corte
               tcee=t0;    //almaceno hora inicial de corte
               waitfor(DelaySec(5)||!alrm_ee);
               if (alrm_ee){
                  //printf("Corte de EE - Envio SMS\r\n");
                  energia=off;      //corte de energia
                  Activar_Macro(1);      // Enciendo luces de Emergencia
                  buff_almac.fcha=tcee;
                  buff_almac.estdo='E';
                  strcpy(buff_almac.trjta,"CAlime");
                  almacenar_event(& buff_almac);
                  strcpy(buff_almacSMS.mensaje,"Corte de Energia Electrica\n");
                  conversion(tcee);
                  strcat(buff_almacSMS.mensaje,th);
                  strcat(buff_almacSMS.mensaje,"\nControlBox\0");
                  fn_SMS_Cat(1,&buff_almacSMS);


               }
            }
         }else{
            if (!energia){
               //printf("Reposicion de EE Envio SMS\r\n");
               energia=on;       //repongo energia
               bateria=on;    //repongo bateria
               buff_almac.fcha=t0;
               buff_almac.estdo='E';
               strcpy(buff_almac.trjta,"RAlime");
               almacenar_event(& buff_almac);
               strcpy(buff_almacSMS.mensaje,"Reposicion de Energia Electrica\n");
               conversion(t0);
               strcat(buff_almacSMS.mensaje,th);
               strcat(buff_almacSMS.mensaje,"\nControlBox\0");
               fn_SMS_Cat(1,&buff_almacSMS);
            }

         }
      }
/************************************************
Analisis de estado batería
************************************************/
      costate{
         if (alrm_bat) {
            if (bateria){
               waitfor(DelaySec(30)||!alrm_bat);
               if(alrm_bat){
                  bateria=off;
                  //printf("Alarma Batería Baja\r\n");
                  buff_almac.fcha=t0;
                  buff_almac.estdo='E';
                  strcpy(buff_almac.trjta,"BATBaj");
                  almacenar_event(& buff_almac);
                  strcpy(buff_almacSMS.mensaje,"Nivel de Bateria por Bajo.El Sistema dejara de Funcionar a la brevedad\n");
                  conversion(t0);
                  strcat(buff_almacSMS.mensaje,th);
                  strcat(buff_almacSMS.mensaje,"\nControlBox\0");
                  fn_SMS_Cat(1,&buff_almacSMS);
               }
            }
         }
      }


      http_handler();
/***********************************
Lectura TCP
***********************************/
      myTCPtick(&socket,&state,&astate);
      if (astate==READED){
         //printf("Recepcion Ethernet:\t %s\r\n",bufferrx);
         if (bufferrx[0]=='!'){
            strcpy(answ,bufferrx+1);
            Proceso();
            strcpy(buffertx,STRING_TX);
            astate=WRITE;
         }else if (bufferrx[0]=='?'){            //Simulo un sms en ethernet
            strcpy(answ,bufferrx+1);
            strcpy(numero_sms,Celular_Prueba);
            strcpy(numero,Celular_Prueba);
            Verificar_Telefono();
            proceso_rx_SMS(answ);     //Analisis de SMS entrante
            //Proceso();
            strcpy(buffertx,buff_almacSMS.mensaje);
            astate=WRITE;
            //serCputs(buff_almacSMS.mensaje);
         }else{
            dir_puerto=1;  //  0=serie 1=ethernet
            serBputs(bufferrx);
            //printf("Envio hacia SIM desde Ethernet:\t %s\r\n",bufferrx);
            bufferrx[0]=0;
            astate=READ;
         }
         leido=1;
      }

      costate{
         if(state==3){
             waitfor(DelaySec(300)||state!=3||leido!=0);
             if(state!=3||leido!=0){
               leido=0;
             }else{
              inicio_IP();
             }
         }
      }

/***********************************
Encendido del Sim SIM
***********************************/
      costate{
         if (SIM_on){
            wfd SIM_ON();
            SIM_on=off;
         }
      }
/***********************************
Apagado del SIM
***********************************/
      costate{
         if (SIM_off){
            wfd SIM_OFF();
            if(SIM_OFF_OK){
               buff_almac.estdo='C';
            }else{
               buff_almac.estdo='E';
            }
            SIM_off=off;
         }
      }
/***********************************
Setear SIM con los valores de fábrica
***********************************/
      costate{
         if (Set_SIM){
            wfd seteoSim();
            Set_SIM=off;
         }
      }

/***********************************
Lectura RS232
***********************************/
      costate{
         if ((c = serCgetc()) != -1) { /* Recibo algo*/
            if( c == '!' ) {     /* recibí un comando*/
                waitfor((count=serCread(ANSW,sizeof(ANSW)-1,200)));
                ANSW[count]=0;
                Proceso();     //Accionamiento de las comandos
            serCputs(STRING_TX);
            }else if( c == '¡' ) {     /* recibí un comando*/
                waitfor((count=serCread(ANSW,sizeof(ANSW)-1,200)));
                ANSW[count]=0;
                strcpy(buff_almacSMS.numero_envio,Celular_Prueba);
                salida(&ANSW);     //Accionamiento de las salidas
                serCputs(buff_almacSMS.mensaje);
            }else if( c == '?' ) {     /* recibí un comando simulo un SMS*/
                waitfor((count=serCread(ANSW,sizeof(ANSW)-1,200)));
                ANSW[count]=0;
                strcpy(numero_sms,Celular_Prueba);
                strcpy(numero,Celular_Prueba);
                Verificar_Telefono();
                printf("recepcion %s\r\n",answ);
                proceso_rx_SMS(answ);     //Analisis de SMS entrante
                printf("Transmision %s\r\n",buff_almacSMS.mensaje);
                serCputs(buff_almacSMS.mensaje);

            }else{      /* recibí un comando para SIM retransmito*/
                dir_puerto=0; //  0=serie 1=ethernet
                //serBputc(c);
                waitfor((count=serCread(ANSW,sizeof(ANSW)-1,200)));
                ANSW[count]=0;
                buffer[0]='A';
                buffer[1]=0;
                strcat(buffer,ANSW);
                serBputs(buffer);
                //printf("A sim:\t%s\r\n",buffer);
                waitfor((count=serBread(ANSW,sizeof(ANSW)-1,200)));
                //printf("De sim:\t%s\r\n",ANSW);
            }
         }
      }

/***********************************
Proceso Alarma
***********************************/
      if (SERIAL1[6]>=26) {   //lectora Wiegand
         con1=0;
         convertdato_tx(SERIAL1);
         //printf("lector1\t%s\r\n",TARJ);
         if(validar_trj(&TARJ)){
            tv1=on;
            strcpy (&SERIAL1,TARJ);
            //printf("Serial1\t%s\r\n",SERIAL1);
         }else{
            memset(&SERIAL1,0x00,22);   //pongo en cero el buffer RX tarjeta1
         }
      }
 /***********************************/

      if (SERIAL2[6]>=26) {   //lectora Wiegand
         con2=0;
         convertdato_tx(SERIAL2);
         if(validar_trj(&TARJ)){
            tv2=on;
            strcpy (&SERIAL2,TARJ);
         }else{
            memset(&SERIAL2,0x00,22);   //pongo en cero el buffer RX tarjeta1
         }
      }
// funcion que analiza Tjtas
      costate{
         if(tv1){
            strcpy(buff_tarj1.trjta,&SERIAL1); //almaceno tarjeta
            buff_tarj1.fcha=t0;
            buff_tarj1.estdo=34;
            memset(serial1,0x00,22);   //pongo en cero el buffer RX tarjeta1
            tv1=off;    //borro info de cambio
            switch(estado_alrm){
               case reposo:    //alarma en estado de reposo / espero armado
                  /*tiempo de espera*/
                  timeon1=0;
                  timeoff1=2000;
                  lr1=off;        //reseteo Led's
                  PORTERO1=on;
                  waitfor(DelayMs(2000)||tv1); //espero dos segundos/tarjeta
                  if(!tv1){  //tarjeta salió por tiempo
                     buff_tarj1.estdo='T'; //salio por tiempo Una marcación
                     timeon1=1500;
                     timeoff1=50;
                     lr1=off;        //reseteo Led's
                  }else{      //tarjeta salió por doble lectuta
                     if(!memcmp(buff_tarj1.trjta,&SERIAL1,6)){  // Tarjetas= armo alarma
                        PORTERO1=off;
                        buff_tarj1.estdo='A';
                        z1=off;
                        z2=off;
                        sensor=on;
                        estado_alrm=set_armado;
                        tv1=off;
                        /*tiempo de armado*/
                        timeon1=100;
                        timeoff1=500;
                        lr1=off;        //reseteo Led's
                        timeon2=100;
                        timeoff2=500;
                        lr2=off;        //reseteo Led's

                        memset(serial1,0x00,22);   //pongo en cero el buffer RX tarjeta1
                     }else{   // Tarjetas distintas espero proxima
                        buff_tarj1.estdo='R';
                        tv1=on;    //reinicio proceso
                     }
                     //almacenar_tarj(&buff_tarj1);      //guardo tarjeta anterior
                  }
                  break;
               case set_armado:    //alarma en estado seteada para armado
                  PORTERO1=on;
                  buff_tarj1.estdo='C';    //corto activación
                  estado_alrm=reposo;
                  /*tiempo de reposo*/
                  timeon1=1500;
                  timeoff1=50;
                  lr1=off;        //reseteo Led's
                  timeon2=1500;
                  timeoff2=50;
                  lr2=off;        //reseteo Led's

                  //almacenar_tarj(&buff_tarj1);
                  break;
               case armado:    //alarma en estado armada
                  PORTERO1=on;
                  buff_tarj1.estdo='D';    //corto activación
                  strcpy(nombre_act,nombre);
                  desactivar_alarma("Localmente\n",RFID);    //Desactivo la alarma
                  break;

               case disparada:    //alarma Disparada
                  PORTERO1=on;
                  buff_tarj1.estdo='E';    //corto activación
                  strcpy(nombre_act,nombre);
                  desactivar_alarma("Localmente\n",RFID);    //Desactivo la alarma
            }
            almacenar_tarj(&buff_tarj1);
         }
         if(tv2){
            strcpy( buff_tarj2.trjta, &SERIAL2); //almaceno tarjeta
            buff_tarj2.fcha=t0;
            buff_tarj2.estdo=34;
            memset(serial2,0x00,22);   //pongo en cero el buffer RX tarjeta1
            tv2=off;    //borro info de cambio
            PORTERO2=on;
            switch(estado_alrm){
               case reposo:    //alarma en estado de reposo / espero armado
                  /*tiempo de espera*/
                  timeon2=0;
                  timeoff2=2000;
                  lr2=off;        //reseteo Led's
                  waitfor(DelayMs(2000)||tv2); //espero dos segundos/tarjeta
                  if(!tv2){  //tarjeta salió por tiempo
                     buff_tarj2.estdo='t'; //salio por tiempo Una marcación
                     timeon1=1500;
                     timeoff1=50;
                     lr1=off;        //reseteo Led's
                     timeon2=1500;
                     timeoff2=50;
                     lr2=off;        //reseteo Led's
                  }else{      //tarjeta salió por doble lectuta
                     if(!memcmp(buff_tarj2.trjta,&SERIAL2,6)){  // Tarjetas= armo alarma
                        PORTERO2=off;
                        buff_tarj2.estdo='a';
                        z1=off;
                        z2=off;
                        estado_alrm=set_armado;
                        sensor=off;
                        tv2=off;
                        /*tiempo de armado*/
                        timeon1=100;
                        timeoff1=500;
                        lr1=off;        //reseteo Led's
                        timeon2=100;
                        timeoff2=500;
                        lr2=off;        //reseteo Led's
                        memset(serial2,0x00,22);   //pongo en cero el buffer RX tarjeta1
                     }else{   // Tarjetas distintas espero proxima
                        buff_tarj2.estdo='r';
                        tv2=on;    //reinicio proceso
                     }
                  }
                  break;
               case set_armado:    //alarma en estado seteada para armado
                  buff_tarj2.estdo='c';    //corto activación
                  estado_alrm=reposo;
                  /*tiempo de reposo*/
                  timeon1=1500;
                  timeoff1=50;
                  lr1=off;        //reseteo Led's
                  timeon2=1500;
                  timeoff2=50;
                  lr2=off;        //reseteo Led's

                  break;
               case armado:    //alarma en estado armada
                  buff_tarj2.estdo='d';    //corto activación
                  strcpy(nombre_act,nombre);
                  desactivar_alarma("Localmente\n",RFID);    //Desactivo la alarma
                  break;

               case disparada:    //alarma en estado de reposo
                  buff_tarj2.estdo='e';    //corto activación
                  strcpy(nombre_act,nombre);
                  desactivar_alarma("Localmente\n",RFID);    //Desactivo la alarma
            }
            almacenar_tarj(&buff_tarj2);
         }
      }

/******* Proceso Armado de Alarma***************************/
      costate{
         if(estado_alrm==set_armado){
            if (sensor){
            	time_act=time_act1;
            }else{
               time_act=time_act2;
            }
            waitfor(DelaySec(time_act)||!(estado_alrm==set_armado));   //tiempo para armar
            if(estado_alrm==set_armado){
               strcpy(nombre_act,nombre);  //almaceno nombre de activación
               activar_alarma("Localmente\n",RFID,0); //Armo la Alarma

           }else{
               //printf("Interrumpí la activacion alarma\r\n");
               estado_alrm=reposo;
               /*tiempo de reposo*/
               timeon1=1500;
               timeoff1=50;
               lr1=off;        //reseteo Led's
               timeon2=1500;
               timeoff2=50;
               lr2=off;        //reseteo Led's
            }
         }
      }
 /*****************************************
 Proceso supervisión Alarma Armada
 ******************************************/
      costate{
         if(estado_alrm==armado){         //Alarma Armada. Proceso supervisión
            est_alrm = INPUT ^ alrm_set_flag;
            est_alrm= est_alrm & alrm_set_flag;
            if(est_alrm!=0){

/****** Agrego tiempos de espera por entradas **/
               for(n=0; n<16; n++){  //informo Zona Alterada
                  if (BIT(&est_alrm,n)){
                     memcpy(&myEntrada,buffer_ent+(n*sizeof(Entrada)),sizeof(Entrada));
                     beep_desactivar=on;
                     //printf ("tiempo de espera alarma%d\r\n",myEntrada.time_alarma);
                     waitfor(DelaySec(myEntrada.time_alarma)||estado_alrm!=armado);   //tiempo para desactivar
                     beep_desactivar=off;
                     break;
                  }
               }

               //printf("\r\nmyentrada.nemonico 1%s\r\n\r\n",myEntrada.nemonico);

               if (estado_alrm==armado){  //la alarma está armada
                  estado_alrm=disparada;  //cambio estado alarma
                  alrm_disp=on;
                  strcpy(buff_almacSMS.mensaje,"Se ha Alarmado la Seguridad.\n");  //27
                  tds=t0;
               //   for(n=0; n<16; n++){  //informo Zona Alterada
               //      if (BIT(&est_alrm,n)){
                        SET(&alrm_flag,n);      //marco zona informada
                        //memcpy(&myEntrada,buffer_ent+(n*sizeof(Entrada)),sizeof(Entrada));
                        if(!strlen(myEntrada.descripcion)){
                           strcat(buff_almacSMS.mensaje,"Zona ");
                           h=strlen(buff_almacSMS.mensaje);
                           STRING[h]= n + 49;
                           STRING[h+1]= 0;
                        }else{
                           strcat(buff_almacSMS.mensaje,myEntrada.descripcion);
                        }
                        strcat(buff_almacSMS.mensaje,"\n");
                        conversion(tds);
                        strcat(buff_almacSMS.mensaje,th);
                        strcat(buff_almacSMS.mensaje,"\nControl Box");
                        web=1;
                   //  }
                 // }
                  printf("\r\nmyentrada.nemonico 2%s\r\n\r\n",myEntrada.nemonico);
                  buff_almac.estdo='A';
                  buff_almac.fcha=t0;
                  strncpy(buff_almac.trjta,myEntrada.nemonico,6);
   					almacenar_event(&buff_almac);

                  for(n=0; n<8; n++){
                     if (estado_alrm==reposo) break;   //si corto la alarma dejo de enviar SMS
                     if (traer_telef(n)==1){ //Busco el Telefono con Cat 1 y2
                        strcpy(buff_almacSMS.numero_envio, numero);
                        almacenar_SMS(&buff_almacSMS);

                     }
                  }
                  waitfor(DelaySec(20)||(estado_alrm==reposo));
                  //Rutina de envío SMS por disparo Alarma
                  for(n=0; n<8; n++){
                     if (estado_alrm==reposo) break;   //si corto la alarma dejo de enviar SMS
                     x= traer_telef(n);
                     if ((x <= 3) && (x > 1)){ //Busco el Telefono con Cat 1 y2
                        strcpy(buff_almacSMS.numero_envio, numero);
                        almacenar_SMS(&buff_almacSMS);
                     }
                  }
                  Llamada_OK=on;                //Llamada al Nº1
               }
            }
         }
      }
/*************************************
//proceso supervisión Alarma disparada
*************************************/
      costate{
         if(estado_alrm==disparada){         //Alarma Armada. Proceso supervisión
            //INPUT = INPUT & set_alrm;
            est_alrm = INPUT ^ alrm_set_flag;
            est_alrm= est_alrm & alrm_set_flag;
            if(est_alrm!=0){
               for(n=0; n<16; n++){  //informo Zona Alterada
                  if (BIT(&est_alrm,n)){
                     if(BIT(&alrm_flag,n)==0){
                        SET(&alrm_flag,n);    //marco zona informada
                        tds=t0;
                        strcpy(buff_almacSMS.mensaje,"Seguridad en Alarma\n");     //32
                        memcpy(&myEntrada,buffer_ent+(n*sizeof(Entrada)),sizeof(Entrada));
                        if(!strlen(myEntrada.descripcion)){
                           strcat(buff_almacSMS.mensaje,"Zona ");
                           h=strlen(buff_almacSMS.mensaje);
                           buff_almacSMS.mensaje[h]= n + 49;
                           buff_almacSMS.mensaje[h+1]= 0;
                        }else{
                           strcat(buff_almacSMS.mensaje,myEntrada.descripcion);
                        }
                        strcat(buff_almacSMS.mensaje,"\n");
                        conversion(tds);
                        strcat(buff_almacSMS.mensaje,th);
                        strcat(buff_almacSMS.mensaje,"\nControl Box");

                        buff_almac.estdo='a';
                  		buff_almac.fcha=t0;
                  		strncpy(buff_almac.trjta,myEntrada.nemonico,6);
   							almacenar_event(&buff_almac);

                        web=1;
                        for(x=0; x<8; x++){
                           if (estado_alrm==reposo) break;   //si corto la alarma dejo de enviar SMS
                           if (traer_telef(x)<=2){ //Busco el Telefono con Cat 1 y 2
                              strcpy(buff_almacSMS.numero_envio, numero);
                              almacenar_SMS(&buff_almacSMS);
                           }
                        }
                        break;
                     }
                  }
               }
            }
         }
      }

/***********************************
Conectividad con SIM
***********************************/
      costate{
         if(SIM_OK && Llamada_SAL){
            if (serBpeek()!= -1){   //hay algo en el Port B
/***********************************
Funcion que recepciona datos SIM - Recepcion llamadas / SMS
***********************************/
               count=0;
               memset(buffer,0x00,180);
               waitfor(DelayMs(1000)||(count=serBread(buffer,sizeof(buffer)-1,200))); // net confirm
               buffer[count]=0;  //guardo datos en buffer
               //printf("Caracteres leidos desde SIM :%d texto :%s\r\n",count,buffer);
rec_SIM:
               if (strstr(buffer,"RING")){
                  t1=t0+20;
                  SMS_delay=0;
                  RING_value++;
                  if (RING_value==1){
                     bsq=strstr(buffer,"+CLIP"); //Veo si viene un numero tel
                     if (bsq!=0){
                        bsqold=strstr(buffer,"\""); //busco comillas
                        bsq=strstr(bsqold+1,"\""); //busco comillas
                        //printf("largo bsqold %d \r\n",bsqold);
                        //printf("largo bsq %d \r\n",bsq);
                        i = bsq - bsqold;
                        strncpy(numero,bsqold+i-10,10);
                        NUMERO[10] = 0;
                        //printf("largo N %d  %s\r\n",i,numero);
                        //strncpy(numero,bsq+8,13);
                        if(Verificar_Telefono()==0){ //Verifico Numero Tel en base
                           serBputs("ATH\r");  //si no está en base corto
                           RING_value=0;
                        }else{ // si está en base guardo el numero de envio SMS
                           strncpy(buff_almacSMS.numero_envio,numero,13);
                        }
                     }
                  }else if (RING_value==6){        // al sexto ring conecto
                     if(strcmp(Tel_nombre,"NoConocido")!=0){
                        serBputs("ATA\r");
                     }
                  }
               //   goto sal_alrm;

               //} else
                  if (strstr(buffer,"NO CARRIER")){ //cortó llamada
                     SMS_delay=1;
                     memset(buff_almacSMS.mensaje,0x00,10);   //pongo en cero buffer String
                     if (RING_value == 6){         //Fue una llamada de control
                        strcpy(buff_almacSMS.mensaje,"Llamada de Control Completada\nControl Box");
                     }else if(RING_value == 0){   //no vino nada
                        goto sal_sms;
                     }else if (RING_value >= 3){   //Informo estado de la alarma
                        if (Tel_categoria<=2){ //Busco el Telefono con Cat 1y2
                           Estado_Alarma();
                        }else{
                           strcpy(buff_almacSMS.mensaje,"No posee autorizacion para realizar la Consulta\nControl Box\0");
                        }
                     }else if(RING_value <= 2){    //encender Luces
                        Activar_Macro(0);      // Activo la Macro 0
                        conversion(t0);
                        strcat(buff_almacSMS.mensaje,th);
                        strcat(buff_almacSMS.mensaje,"\nControl Box\0");
                     }
                     almacenar_SMS(&buff_almacSMS);
                     RING_value=0;
                  }
               //   goto sal_alrm;
               } else if (strstr(buffer,"NO CARRIER")){ //cortó llamada
                     SMS_delay=1;
                  memset(buff_almacSMS.mensaje,0x00,10);   //pongo en cero buffer String
                  if (RING_value == 6){         //Fue una llamada de control
                     strcpy(buff_almacSMS.mensaje,"Llamada de Control Completada\nControl Box");
                  }else if(RING_value == 0){   //no vino nada
                     goto sal_sms;
                  }else if (RING_value >= 3){   //Informo estado de la alarma
                     habilitar_telnet=1;
                     if (Tel_categoria<=2){ //Busco el Telefono con Cat 1
                        Estado_Alarma();
                     }else{
                        strcpy(buff_almacSMS.mensaje,"No posee autorizacion para realizar la Consulta\nControl Box\0");
                     }
                  }else if(RING_value <= 2){    //encender Luces
                     Activar_Macro(0);
                     //buscar_sms(2);
                     //strcat(buff_almacSMS.mensaje,Tel_nombre);
                     //strcat(buff_almacSMS.mensaje," Activada\n");
                     conversion(t0);
                     strcat(buff_almacSMS.mensaje,th);
                     strcat(buff_almacSMS.mensaje,"\nControl Box\0");
                  }
                  almacenar_SMS(&buff_almacSMS);
                  RING_value=0;
 /***************SMS Entrante***************************/
               }else if (bsq=strstr(buffer,"+CMT:")){
                  strcpy(string,bsq+5);
                  bsqold=strstr(string,"\""); //busco comillas
                  bsq=strstr(bsqold+1,"\""); //busco comillas
                  l = bsq - bsqold;
                  if (l>9){
                     strncpy(numero,bsqold+l-10,10);
                     NUMERO[10] = 0;
                     if(Verificar_Telefono()==1){    //Verifico Numero Tel en base
                        strcpy(numero_sms,numero);
                        serCputs(Tel_nombre);
                        serCputs("\r\n");
                        bsq=strstr(string,"\r\n");
                        //printf("largo N %d\r\n",bsq);
                        if (bsq){
                           strcpy(answ,bsq+2);
                           //printf("texto SMS %s \r\n",answ);
                           waitfor(DelaySec(5)||(count=serBread(buffer,sizeof(buffer)-1,300)));
                           if (count==0){
                              //printf("Lectura posterior SMS nula\r\n");
                           }else{
                              buffer[count]=0;
                              //printf("Lectura posterior SMS:%s\r\n", buffer);
                           }
                           proceso_rx_SMS(answ);     //Analisis de SMS entrante
                        }
                     }
                  }else{         //numero corto
                     strcpy(buff_almacSMS.mensaje,buffer);
                     for(n=0; n<8; n++){
                        if (traer_telef(n)<=1){ //Busco el Telefono con Cat 1
                        break;   //si encuentro 1 voy a llamar
                        }
                     }
                     strcpy(buff_almacSMS.numero_envio,numero);
                     almacenar_SMS(&buff_almacSMS);
                  }
//Mensaje prestadora usado para el pedido saldo *150#
               }else if (bsq=strstr(buffer,"+CUSD:")){
                  //printf("SMS Entrante:%s\n",buffer);
                  strcpy(buff_almacSMS.numero_envio, numero_sms);
                  bsq=strstr(buffer,",\"");
                  if (bsq){
                     strcpy(buff_almacSMS.mensaje,bsq+2);
                     buff_almacSMS.mensaje[strstr(buffer,"\",")-bsq-2]=0;

                     almacenar_SMS(&buff_almacSMS);
                  }
//tipo de SMS desconocido-mensaje desde Sim puede ser un comando
               }else{
                  if(strlen(buffer)){
                     sprintf(STRING_TX,"RX SIM:\t%s\r\n",buffer);
                     if(dir_puerto==0){   //  0=serie 1=ethernet
                        serCputs(STRING_TX);
                     }else{
                        strcpy(buffertx,STRING_TX);
                        astate=WRITE;
                        application_handler(&socket,&astate);
                        leido=1;
                     }
                  }
               }
            }else{
/************************************************
Funcion que envia SMS
Controla que haya SMS para enviar y que SMS_delay=1
************************************************/
               if ((puntin_SMS!=puntout_SMS) && SMS_delay){
                  if(!Llamada_SAL){goto sal_sms;}  //Verifico que np haya Llamada sal
                  EnvioSMS_ON=on;     //Comienzo envio SMS
                  t1=read_rtc();
                  //espero a que celular este ready
                  serBputs("AT+CPAS\r");
                  memcpy(&buff_envioSMS, buffer_envioSMS + (puntout_SMS * sizeof(buff_envioSMS)),sizeof(buff_envioSMS));
                  memset(buffer,0x00,180);
                  waitfor(DelaySec(5)||(count=serBread(buffer,sizeof(buffer)-1,300)));
                  if (!count){           //no devuelve estado
                     serBputs("\032");
                     t1=t0+90;      //No envío SMS por 90 seg
                     SMS_delay=0;
                     buff_almac.fcha=t0;
                     buff_almac.estdo='E';
                     strcpy(buff_almac.trjta,"SMS012");  //Salió por tiempo CPAS
                     //SIM_reset=1;
                     goto fin_sms;  //voy a incrementar contador de error y almaceno error
                  }
                  //printf("Recepcion +cpas %s\r\n", buffer);
                  if(!strstr(buffer,"+CPAS: 0")){
                     t1=t0+90;      //No envío SMS por 90 seg
                     SMS_delay=0;
                     buff_almac.fcha=t0;
                     buff_almac.estdo='E';
                     strcpy(buff_almac.trjta,"SMS000");
                     //SIM_reset=1;
                     goto fin_sms;  //voy a incrementar contador de error y almaceno error
                  }   //estado invalido
                  //
                  //buscar_SMS();
                  //printf("Pase el CpasOK SMS numero %d \r\n\r\n",puntout_SMS);
                  serBputs("AT+CMGS=\"");
                  i=strlen(buff_envioSMS.numero_envio);
                  if (i>10){
                     strncpy(buff_envioSMS.numero_envio,buff_envioSMS.numero_envio+(i-10),10);
                     buff_envioSMS.numero_envio[10]=0;
                  }
                  sprintf(texto_sms,"AT+CMGS=\"%s\"\r",buff_envioSMS.numero_envio);
                  serBputs(buff_envioSMS.numero_envio);
                  serBputs("\"\r");
                  //serBputs(texto_sms);
                  //printf("Numero para envio SMS %s \r\nfin1\r\n",buff_envioSMS.numero_envio);
                  memset(buffer,0x00,180);   //pongo en cero el buffer BLNK
                  waitfor(DelaySec(10)||(count=serBread(buffer,sizeof(buffer)-1,300)));
                  //printf("Recepcion del prompt envio SMS:%s\r\n\n", buffer,count);
                  buff_almac.fcha=t0;
                  if (count<5){
                     if (count==0){
                        //buff_almac.fcha=t0;
                        buff_almac.estdo='E';
                        strcpy(buff_almac.trjta,"SMS001"); //no vino nada
                        serBputs("\032");
                        //printf("No vino >, salio por tiempo SMS001\r\n\r\n");
                        goto fin_sms;  //voy a incrementar contador de error y almaceno error
                     }

                     //if (buffer[count-2]!='>'){ //No encontró el prompt
                     if (!strchr(buffer,'>')){ //No encontró el prompt
                        //buff_almac.fcha=t0;
                        buff_almac.estdo='E';
                        strcpy(buff_almac.trjta,"SMS002"); //no recibo el prompt
                        serBputs("\032");
                        //printf("No encontro el Prompt envio SMS002 %s \r\n\r\n ", buffer);
                        goto fin_sms; //voy a incrementar contador de error y almaceno error
                     }
                  }else{
                     if(strstr(buffer,"ERROR")){
                        //buff_almac.fcha=t0;
                        buff_almac.estdo='E';
                        strcpy(buff_almac.trjta,"SMS003"); //Vino error
                        serBputs("\032");
                        //printf("Vino ERROR envio SMS003 %s \r\n\r\n ", buffer);
                        //serCputs("4 Vino Error en lugar delPrompt\r\n");

                     }else{
                        //buff_almac.fcha=t0;
                        buff_almac.estdo='E';
                        strcpy(buff_almac.trjta,"SMS004"); //Vino otro error
                        serBputs("\032");
                        //printf("Vino otro error envio SMS004 %s \r\n\r\n ", buffer);

                        //serCputs("4 Vino otro Error en lugar delPrompt\r\n");
                     }
                     goto fin_sms;
                  }
                  //printf("Envio el mensaje para SMS %s \r\n\r\n",buff_envioSMS.mensaje);
                  buff_envioSMS.mensaje[158]=0;
                  //printf("Tamaño Mensaje Enviado %d \r\n", strlen(buff_envioSMS.mensaje));
                  serBputs(buff_envioSMS.mensaje);             //envio el mensaje
                  serBputs("\032");
 recsms:
                  memset(buffer,0x00,10);   //pongo en cero el buffer BLNK
                  waitfor(DelaySec(120)||(count=serBread(buffer,sizeof(buffer)-1,1000))); // net confirm
                  //printf("\n\nRecepcion del confirmacion envio SMS:%s\t%d\r\n\n", buffer,count);
                  buff_almac.fcha=t0;
                  if(!count){          //No vino confirmacion mensaje
                     buff_almac.estdo='E';
                     strcpy(buff_almac.trjta,"SMS010");  //salida por tiempo
                  }else{
                     if(strstr(buffer,"OK")){     //vino OK
                        buff_almac.estdo='O';
                        bsq=strchr(buffer,':');    //busco la primera coma
                        strcpy(CARACTER,"SMS000");
                        strncpy(CARACTER+3,bsq+2,3);
                        strcpy(buff_almac.trjta,CARACTER);
                        goto incrementar_puntero;
                     }else if(strstr(buffer,"+CMS ERROR")){   //Vino Error
                        buff_almac.estdo='E';
                        bsq=strchr(buffer,':');    //busco la primera coma
                        strcpy(CARACTER,"SMS000");
                        strncpy(CARACTER+3,bsq+2,3);
                        strcpy(buff_almac.trjta,CARACTER);
                        t1=t0+90;      //No envío SMS por 90 seg
                        SMS_delay=0;
                     }else{            //No vino ni OK ni Error
                        //printf("\n\nError en envio SMS:%s\n",buffer);
                        buff_almac.estdo='E';
                        strcpy(buff_almac.trjta,"SMS011");  //salida por tiempo
                        //almacenar_event(& buff_almac);
                        /*strcpy(buff_almacSMS.mensaje,"ERROR: ");
                        strncpy(buff_almacSMS.mensaje,buffer,strlen(buffer));
                        buff_almacSMS.mensaje[strlen(buffer)+7];
                        for(n=0; n<8; n++){
                           if (traer_telef(n)<=1){ //Busco el Telefono con Cat 1
                           break;   //si encuentro 1 voy a llamar
                           }
                        }
                        strcpy(buff_almacSMS.numero_envio,numero);
                        almacenar_SMS();

                        goto recsms;*/
                     }
                  }
fin_sms:
                  //Incrementar 90    t1=t0+90;      //No envío SMS por 90 seg
                  //      SMS_delay=0;


                  EnvioSMS_ERROR = EnvioSMS_ERROR + 1;
                  if (EnvioSMS_ERROR>2){      //pasó tres veces descarto el mensaje
incrementar_puntero:

                  puntout_SMS++;
                     if (puntout_SMS > LARGO_ENVIOSMS-1){
                        puntout_SMS=0;
                     }
                     EnvioSMS_ERROR=0;        //reseteo el contador
                     /*if(SMS_delay==0){
                        SIM_reset=on;
                     } */
                  }
almac_evento:
                  EnvioSMS_ON=off;
                  almacenar_event(& buff_almac);
               }  //cierro If envío SMS
            }
sal_sms:
         EnvioSMS_ON=off;
         }
      }
/*************Funciones del Lector******************************/
/************************************
Funcion que controla encendido Leds Lector 1
************************************/
         costate{
            while(1){
               WrPortI(PEB7R, NULL, 0x80);   // set initial output value
               //WrPortI(PEB4R, NULL, 0x10);    // set initial output value
               waitfor(DelayMs(timeon1)||!lr1);  // net confirm
               WrPortI(PEB7R, NULL, 0x00);   // set initial output value
               //WrPortI(PEB4R, NULL, 0x00);    // set initial output value
               waitfor(DelayMs(timeoff1)||!lr1);    // net confirm
               lr1=1;
            }
         }
/************************************
Funcion que controla encendido Leds Lector 2
************************************/
         costate{
            while(1){
               WrPortI(PEB4R, NULL, 0x10);    // set initial output value
               waitfor(DelayMs(timeon2)||!lr2);  // net confirm
               WrPortI(PEB4R, NULL, 0x00);    // set initial output value
               waitfor(DelayMs(timeoff2)||!lr2);    // net confirm
               lr2=1;
            }
         }
/******************************************************
Función que controla el Beep de Activación Lector1
******************************************************/
      costate{
         if (estado_alrm==set_armado){
             set(porta,6);  // set initial output value
             waitfor(DelayMs(100));  // Espero tiempo solicitado
             res(porta,6);  // set initial output value
             waitfor(DelayMs(500));  // Espero tiempo solicitado
         }
      }
/******************************************************
Función que controla el Beep de Desactivación Lector1
******************************************************/
      costate{
         if (beep_desactivar){
            set(porta,6);  // set initial output value
            waitfor(DelayMs(100)||!beep_desactivar);  // Espero tiempo solicitado
            res(porta,6);  // res initial output value
            waitfor(DelayMs(800)||!beep_desactivar);  // Espero tiempo solicitado
         }
      }
/*********************Fin Funciones lector*****************/
/*********************Funciones Salida Sirena**************/
/*** Funcion que controla Beep Des/activación Sirena*******/
      costate{
         if(beep_on){
            set(porta,7);
            waitfor(DelayMs(T_beep1));  // Tiempo de espera ON
            res(porta,7);
            waitfor(DelayMs(T_beep2));  // Tiempo de espera ON
            set(porta,7);
            waitfor(DelayMs(T_beep3));  // Tiempo de espera ON
            res(porta,7);
            beep_on=off;      //desactivo beep alarma
            web=1;
         }
      }

/*** Funcion que controla Salida Sirena Alarma Disparada***/
      costate{
         if (alrm_disp){
            for(l=0; l<10; l++){  //hago tres ciclos
               //set(porta,6);
               set(porta,7);
               //serCputs("Seteo Alarma\r\n");
               waitfor(DelaySec(30)||!alrm_disp);  // Tiempo de espera ON
               //res(porta,6);
               res(porta,7);
               //serCputs("Reseteo Alarma\r\n");
               waitfor(DelaySec(10)||!alrm_disp);  // Tiempo de espera OFF
               //set(porta,6);
               set(porta,7);
               //serCputs("Seteo Alarma\r\n");
               waitfor(DelaySec(15)||!alrm_disp);  // Tiempo de espera ON
               //res(porta,6);
               res(porta,7);
               //serCputs("Reseteo Alarma\r\n");
               waitfor(DelaySec(30)||!alrm_disp);  // Tiempo de espera OFF
            }
            waitfor(DelaySec(60)||!alrm_disp);    // Tiempo de espera Rehabilitac
            if (alrm_disp){
               serCputs("Rehabilito Seguridad\r\n");
               estado_alrm=armado;
               alrm_disp=off;
               alrm_flag=off;
            }
         }
      }
/*********************Fin Funciones Salida Sirena*****************/

 //fin de la funcion
 /****control salida 1 ***********/
      if (BIT(porta,0) == on){
         if (TS[0]<t0){
            res(porta,0);  // set initial output value
            web=1;
         }
      }
 /****control salida 2 ***********/
      if (BIT(porta,1) == on){
         if (TS[1]<t0){
            res(porta,1);  // set initial output value
            web=1;
         }
      }
 /****control salida 3  *****************/
      if (BIT(porta,2) == on){
         if (TS[2]<t0){
            res(porta,2);  // set initial output value
            web=1;
         }
      }
 /****control salida 4 ************/
      if (BIT(porta,3) == on){
		if (TS[3]<t0){
			res(porta,3);  // set initial output value
			web=1;
		}
	}
 /****control salida 5 ************/
      if (BIT(porta,4) == on){
         if (TS[4]<t0){
            res(porta,4);  // set initial output value
            web=1;
         }
      }

 /****control salida 6   *****************/
      /*if(PORTERO == off){
      	if (BIT(porta,5) == on){
	         if (TS[5]<t0){
	            res(porta,5);  // set initial output value
	            web=1;
	         }
	      }
      }*/

/****     acciono portero2 en salida6   **********/
      costate{
      	if (PORTERO1 == on){
         	res(porta,5);  // set initial output value
            web=1;
            //printf("HABILITO PORTERO");
            waitfor(DelaySec((SalidaTiempo[5]))||(PORTERO1==off));
            //printf("DESHABILITO PORTERO");
            PORTERO1=off;
            set(porta,5);  // set initial output value
	         web=1;
         }else{   //no accioné el portero
            if (BIT(porta,5) == off){
               web=1;
               printf("No acciono Portero\r\n");
               waitfor(DelaySec((SalidaTiempo[5])));
	            set(porta,5);  // set initial output value
	            web=1;
            }
	      }
		}
/****************************************************
Actualizo estados de las salidas para la web interna
****************************************************/
if (web && flag_activar==0){
  	if (ent_sal){      //estado entradas
   	for(i=0; i<8; i++){
      	if (CambioEnt){
      		j=i;
      	}else{
         	j=i+8;
      	}
      	Btn_Habilitado[i]=1;
      	if(BIT(&ent_operativas,j)){
            if(BIT(&ent_no_asociada,j)){	 //entradas no asociadas por seteo
            	if (!BIT(&INPUT,j)){
	            	strcpy(Btn_Color[i],"green");
	               sprintf(Btn_Texto[i],"%s&#58&#32Normal",Entrada_Nemonico[j]);
	            }else{
	            	strcpy(Btn_Color[i],"red");
	               sprintf(Btn_Texto[i],"%s&#58&#32Falla",Entrada_Nemonico[j]);
	            }
            }else{
               //selección de estado Alarma -Armada o disparada
               if (estado_alrm > 1){
                  if(BIT(&set_alrm,j)){   //Selecciono entrada supervisada
                     //seleccion de estado ON / OFF / Disp.
	                  if(BIT(&alrm_set_flag,j)){  //Entradas Armadas para la Alarma
	                     if(BIT(&alrm_flag,j)){  //Entrada Armada/Disparada
	                        strcpy(Btn_Color[i],"red");
	                        sprintf(Btn_Texto[i],"%s&#58&#32Alarma",Entrada_Nemonico[j]);
	                     }else{  //Entrada Armada/Normal
	                        strcpy(Btn_Color[i],"green");
                           sprintf(Btn_Texto[i],"%s&#58&#32On",Entrada_Nemonico[j]);
	                     }
	                  }else{ //Entrada Off en el momento de armado alarma
	                     strcpy(Btn_Color[i],"black");
	                     sprintf(Btn_Texto[i],"%s&#58&#32Off",Entrada_Nemonico[j]);
	                  }
	               }else{  //Entrada OFF al momento de Activar
                     strcpy(Btn_Color[i],"transparent");
			            sprintf(Btn_Texto[i],"%s&#58&#32Nop",Entrada_Nemonico[j]);
	               }
	            }else{  //Alarma en reposo
                  if (!BIT(&INPUT,j)){
	                     strcpy(Btn_Color[i],"#D9950D");
	                     sprintf(Btn_Texto[i],"%s&#58&#32No&#32Ok",Entrada_Nemonico[j]);
	                  }else{
	                     strcpy(Btn_Color[i],"#2D2D53");
	                     sprintf(Btn_Texto[i],"%s&#58&#32Ok",Entrada_Nemonico[j]);
	               }
	            }
         	}
         }else{   //entradas no operativas
            strcpy(Btn_Color[i],"transparent");
            sprintf(Btn_Texto[i],"%s&#58&#32Nop",Entrada_Nemonico[j]);
         }
      }

      Btn_Habilitado[8]=1;

      if(alrm_ee == on){
         strcpy(Btn_Color[8],"red");
         strcpy(Btn_Texto[8],"Red&#58&#32Falla");
      }else{
         strcpy(Btn_Color[8],"green");
          strcpy(Btn_Texto[8],"Red&#58&#32Normal");
      }

   }else{      //estado salidas
      for(i=0; i<8; i++){
         if  (Btn_Habilitado[i]!=1){
            if (BIT(porta,i)){
               strcpy(Btn_Color[i],"red");
               sprintf(Btn_Texto[i],"%s&#58&#32On",Salida_Nemonico[i]);
            }else{
               strcpy(Btn_Color[i],"green");
               sprintf(Btn_Texto[i],"%s&#58&#32Off",Salida_Nemonico[i]);
            }
         }else{
            strcpy(Btn_Color[i],"transparent");
            sprintf(Btn_Texto[i],"%s&#58&#32Nop",Salida_Nemonico[i]);
         }
      }
      Btn_Habilitado[8]=0;

      if(estado_alrm != reposo){
         strcpy(Btn_Color[8],"black");
         strcpy(Btn_Texto[8],"Desactivar");
      }else{
         strcpy(Btn_Color[8],"green");
         strcpy(Btn_Texto[8],"Activar");
      }
   }
   Estado_Alarma_Web();
   web=0;
}


//fin de la funcion
      costate{
         if (!SMS_delay){
            //printf("Tempo SMS\r\n");
            waitfor((t0>t1)||SMS_delay);  // net confirm
            //printf("salí Tempo SMS\r\n");
            SMS_delay=on;
         }
      }
      WrPortI(PADR, NULL, PORTA);      //Actualizo port A

/********************************************
Rutina de apagado del SIM por puerto de
********************************************/
     /* costate{
         if (SIM_off){
            //printf("proceso de apagado SIM \r\n");
            SIM_OK=off;       //Deshabilito funciones del SIM
            WrPortI(PBDR, NULL, 0x00);
            //printf("primer proceso de apagado SIM\t%u\r\n",MS_TIMER);
            waitfor(DelayMs(250 0)||BitRdPortI(PBDR,0));      //750 MS de SIM apagado
            //printf("segundo proceso de apagado SIM\t%u\r\n",MS_TIMER);
            WrPortI(PBDR, NULL, 0x80);
            waitfor(DelayMs(1000)||BitRdPortI(PBDR,0));

            WrPortI(PBDR, NULL, 0x00);
            waitfor(DelayMs(1500)||BitRdPortI(PBDR,0));      //750 MS de SIM apagado
            //printf("Tercer proceso de apagado SIM\t%u\r\n",MS_TIMER);
            WrPortI(PBDR, NULL, 0x80);
            waitfor(DelayMs(6000)||BitRdPortI(PBDR,0));
            if(BitRdPortI(PBDR,0)){
               buff_almac.estdo='C';
            }else{
               buff_almac.estdo='E';
            }
            SIM_off=off;
         }
      } */
/********************************************
Rutina de llamada por alarma
********************************************/
      costate{
         if (Llamada_OK){
            //printf("Habilito Llamada\r\n");
            waitfor(DelaySec(15)||!Llamada_OK);
            //printf("Habilito Llamada 1 \r\n");
            if(Llamada_OK){
               Llamada_SAL=off;     //Activo Llamada en curso
               waitfor(DelaySec(20)||(!EnvioSMS_ON && !RING_value));
               serBputs("AT+CPAS\r");
               memset(bufferc,0x00,180);
               waitfor(DelaySec(10)||(count=serBread(bufferc,sizeof(bufferc)-1,300)));
               if (count){           //no devuelve estado
                  if(!strstr(bufferc,"+CPAS: 0")){
                  //serBputs("Error CPAS\r");
                  goto fin_llamada;
                  }
               }else{
                  //serBputs("Error CPAS1\r");
                  goto fin_llamada;
               }

               for(n=0; n<8; n++){
                  if (traer_telef(n)<=1){ //Busco el Telefono con Cat 1
                     break;   //si encuentro 1 voy a llamar
                  }
               }
               //printf("Numero %s\r",NUMERO);
               serBputs("ATD");
               serBputs(NUMERO);
               serBputs(";\r");

               //serBputs("ATD3416403413;\r");
               waitfor(serBwrFree()!=BOUTBUFSIZE);    //espero que el buffer esté vacío
               memset(bufferc,0x00,180);
               waitfor(DelaySec(10));
               waitfor(DelaySec(30)||(count=serBread(bufferc,sizeof(bufferc)-1,300)));
               //printf("recepcion llamadas %s\t%d\r\n",bufferc,count);
               if (count){
                  if(strstr(bufferc,"+COLP:")){
                     serBputs("AT+VTS=\"1,5,1,5,1,5,1,5,1,5\"\r");
                     waitfor(DelaySec(5));
                  }else if (strstr(bufferc,"BUSY")){
                     goto fin_llamada;
                  }
               }
               serBputs("ATH\r");
               memset(bufferc,0x00,180);   //pongo en cero el buffer BLNK
               waitfor(DelaySec(20)||(count=serBread(bufferc,sizeof(bufferc)-1,300)));
               bufferc[count]=0;
               serCputs(bufferc);
               serCputs("\r\n");
               Llamada_OK=off;
               Llamada_SAL=on;      //Activo Llamada en curso
            }
         }
fin_llamada:
      }
/************************************************
Función que chequea el funcionamiento del timer B
************************************************/

       INTER++;
      if (INTER > 200){
         INTER=0;
        // printf("se corto la interrupción\r\n");
         initialize_timerb();
      }
/************************************************
Función que chequea el funcionamiento del SIM
Cada 15 minutos pide un CPAS y en caso de no obtener
el CPAS=0 procede a reiniciar el SIM
************************************************/
      costate{
         waitfor(DelaySec(900));    //hay que poner 900s - 15 min
         if (EnvioSMS_ON==off)
            SIM_OK=off;
            serBputs("AT+CPAS\r");
            //printf ("pido CPAS\r\n");
            waitfor(DelaySec(5)||(count=serBread(buffer,sizeof(buffer)-1,300)));
            if (!count){           //no devuelve estado
               //inicio_cbox();
               goto reset;
            }
            if(strstr(buffer,"+CPAS: 0")){
               //printf ("vino CPAS=0\r\n");
               goto fin_sup;
            }
reset:
         buff_almac.fcha=t0;
         buff_almac.estdo='E';
         strcpy(buff_almac.trjta,"RSTCel");
         almacenar_event(& buff_almac);
         SIM_reset=on;
fin_sup:
         SIM_OK=on;
      }
/************************************************
Función que resetea el SIM - Anda OK con SIM 900
************************************************/
      costate{
         if(SIM_reset==1){
            SIM_OK=off;
/****Apago el celular         ******************/
            wfd SIM_OFF();
            waitfor(DelaySec(10));
/****Prendo el celular        ******************/
            wfd SIM_ON();
            SIM_OK=off;          //espero otros 10 segundos
            waitfor(DelaySec(20));
            SIM_reset=off;
            SIM_OK=on;
         }
      }
      VdHitWd(ID);
   }
}

