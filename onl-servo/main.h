//#include <iostream.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


#include<strings.h>
#include<string.h>

#include<ncurses.h>
#include<sys/stat.h>
#include<termios.h>
#include<stdint.h>

#include <poll.h>
#include <time.h>
#include <errno.h>
#include <openssl/err.h>



#define PORT_NUMBER 50023
#define HOSTNAME  "tcp2can"


#define FALSE		0
#define TRUE		1

#define SUCCESS		0
#define ERROR		1

#define EMPTY_MESSAGE	0
#define ONE_SECOND		1

#define DEAD_TIME_WAITING	2
#define VALUE_BYTE_POSITION_IN_PACKET	9
#define ADRESS_BYTE_POSITION_IN_PACKET	7

#define	SPH_FACTOR		809

#define HEARTBIT_ETH2CAN_PROTOCOL_CMD			0x09    //Передается шлюзом для проврки связи
#define GETMODE_ETH2CAN_PROTOCOL_CMD			0x10    //Запрос режима самого шлюза
#define MYMODE_ETH2CAN_PROTOCOL_CMD				0x11    //Ответ с указанием режима шлюза
#define TRANSPAREOFF_ETH2CAN_PROTOCOL_CMD	0x12
#define TRANSPAREON_ETH2CAN_PROTOCOL_CMD	0x13
#define TOUART_ETH2CAN_PROTOCOL_CMD				0x14
#define FROMUART_ETH2CAN_PROTOCOL_CMD			0x15
#define TOCAN_ETH2CAN_PROTOCOL_CMD				0x16
#define FROMCAN_ETH2CAN_PROTOCOL_CMD			0x17
#define CAN_TRANSLATE_REQ_COM							0x0D

#define RxBEGIN	0
#define RxDATA	1


#define	servoS3X		01
#define	servoS3Y			02
#define	servoK1X		03
#define	servoK1Y			04
#define	servoK2X		05
#define	servoK2Y			06

#define S3Y_POLOJENIE_KONCEVIKA 141966


#define S3X_ID	1020252
#define S3Y_ID	0
#define K1X_ID	1010374
#define K1Y_ID	1010373
#define K2X_ID	1020091
#define K2Y_ID	1020049


// adress of parametr must be mirrorlike
//#define Dd11ServoStatus		0x040F
#define Dd11ServoStatus					0x0F04
#define	Lp11StatusInterpretatora		0x4200
#define Dd7CurrentPosition				0x0B04
#define Dd4CurrentSpeed					0x0804
#define REGISTOR_Dd19PositionMistake				0x0C04



//typedef enum servo_params {
enum servo_params { 			 			
				up0		= 0x0700,
				up1		= 0x0701,
				up2		= 0x0702,
				up3		= 0x0703,
				up4		= 0x0704,
				up5		= 0x0705,
				up6		= 0x0706,
				up7		= 0x0707,
				
				X		= 0x0708,
				Y		= 0x0709,
				Z		= 0x070A,
				
/**/
				Dd1 	= 0x0400,
				Dd2 	= 0x0402,
				Dd3 	= 0x0405,
				Dd6 	= 0x040A, 
				Dd9 	= 0x040D,
				Dd4 	= 0x0408,
				Dd5 	= 0x0409,
				dd21	= 0x0419,
				Dd7		= 0x040B,
				Dd8		= 0x040C,
				dd19	= 0x0417,
				
				Dd10	= 0x040E,
				

				Dd12	= 0x0410,
				Dd15	= 0x0413,
				Dd16	= 0x0414,
				Dd17	= 0x0415,
				Dd18	= 0x0416,
				
				Ss2		= 0x001F,
				Ss3		= 0x0020,
				Ss4		= 0x0021,
				Ss5		= 0x0022,
				Ss6		= 0x0023,
				Ss7		= 0x0024,
/*/*/			
				
} ; //  servo_params ;


struct ControlTranspare { 	unsigned char Type; 
							unsigned char Ctrl; 
						};

typedef enum  {
		NONE_BYTE_RECEIVED, 
		ONE_BYTE_RECEIVED, 
		PACKET_RECEIVED
		} 
RET_CODE_ENUM;

typedef enum  { S3X = 1, 	S3Y= 2,  	K1X = 3,	K1Y = 4,	K2X = 5,	K2Y = 6
} 
SERVOS ;


typedef int32_t id_type  ;

typedef struct  {
	id_type ID;
	int isAvailable;
	
} servosInSystem;

static char *ServoNames[] = {
	"",
    "Servomotor S3X ",
    "Servomotor S3Y ",
    "Servomotor K1X ",
    "Servomotor K1Y ",
    "Servomotor K2X ",
    "Servomotor K2Y "   
};	

static char *Servoprivod[] = {
	"",
    "S3X",
    "S3Y",
    "K1X",
    "K1Y",
    "K2X",
    "K2Y"   
};

void setX( char *data, int16_t value) ;
void printHex(char *data, int length);
int TranspareByte( unsigned char in_byte, unsigned char * out_bytes);
int  create_ETH2CAN_packet(unsigned int canid, unsigned char com, unsigned char len, unsigned char* data, unsigned char * data_out);
int TranspareMessage( unsigned char * in_message, unsigned char * out_bytes, int size_in_message);
void  writeIntInChar ( void * data,  int32_t value) ;
void	printUsage (void); 
int   	printPosition ( int  shp_number );
RET_CODE_ENUM RetranspareByte( unsigned char in_byte, unsigned char *out_byte);
int RetranspareMessage (unsigned char *message_in, unsigned char *message_out, int message_in_size);
void printRetro( unsigned char *data, int size ) ;
void   	printList(void ) ;
void   send_message_ETH2CAN( unsigned char *message, int total);
int  get_packet_from_ETH2CAN( unsigned char * buff, int buffSize );
//int get_messages_from_ETH2CAN ( unsigned char * data, int timeForWaiting) ;
int get_message_from_packet( unsigned char * data) ;
int 	move_privod(int sph_number, int destination);
int	getCanId(void   *data);
/*		wait for message from ETH2CAN shlyz
*		arg
*		return:		sizeof answer message, or  ERROR  (0) 
*/		
void server_connect( void);
void server_close(void);
void connect_with_ETH2CAN(void) ;
void disconnect_with_ETH2CAN(void) ;

int	wait_for_answer_for_ETH2CAN ( unsigned char *answer , int anwerMaxSize);
uint16_t getParametrAdress( void * data_in) ;
//int16_t getValue ( void * data,  int bytePosition);
int32_t		getValue ( void * data,  int bytePosition);
int16_t		getInt16Value ( void * data,  int bytePosition);
int32_t		getPosition ( void * data,  int bytePosition, int can_port) ;
int  executeProgramInBank(int  sph_number, int bank_number );

int checkStatusWhileMoving (int  sph_number );
int checkStatusANDDestinationWhileMoving (int  sph_number, int destination );
int32_t getFactoryID (int comPortNumber);
int createGetFactoryID_Message( unsigned char *messageGetSt3_Factory_ID, int comPortNumber );
int isThereRightMessage (unsigned char *data, int messageSize);
id_type getIDforThisMassive( unsigned char * data);
int findCompPortForThisDevice( id_type factoryID );	
void setDefaultParameters ( servosInSystem * allServo );



