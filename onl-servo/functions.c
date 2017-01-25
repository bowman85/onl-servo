#include "main.h"


  struct hostent *hp;
  struct sockaddr_in ServerAddress;
  int mainSocket;

void	printUsage ( )
{
		printf( "--------------------------------------------------------------------------\n");
		printf( "USAGE:\n");
		printf( "list\t\t\t- show all active devices;\n");
		printf( "print\t\t n\t- print \"n\" servo position;\n");		
		printf( "moveto\t\t n  o\t- move \"n\" (1 to 6) servo to \"o\" mm;\n");
		//printf( "com_moveto\t n  o\t- move device on COMPORT \"n\" (1 to 6) to \"o\" mm;\n");
		printf( "bank\t\t n  o\t- start programm for \"n\" servo in \"o\" bank.\n");
		printf( "getid\t\t n\t- show factory ID for servo on this COMPORT\n");
		printf( "--------------------------------------------------------------------------\n\n");
}



void  writeIntInChar ( void * data,  int32_t value) {
	int32_t * dat_int = (int32_t* ) data;
	*dat_int  = value; 
//	printf( "*dat_int = %d\n", *dat_int);
 
return  ;
}


int TranspareMessage( unsigned char * in_byte, unsigned char * out_bytes, int size_in_message){
	int total = 1;
	int i , code_out =0;
unsigned char crc_code ; 
	for (i =0; i < size_in_message; i++)	{
		code_out = TranspareByte (in_byte[i], &out_bytes[total]);
		crc_code ^= in_byte[i];
		total += code_out;
	}
	out_bytes[0] = 0x01;
	out_bytes[total] = crc_code ;
	out_bytes[total++] = 0x03 ;
int z ;

//	printf("out_bytes[%d] = %.2X\n", total, out_bytes[total]);
	return total;
}

int TranspareByte( unsigned char in_byte, unsigned char * out_bytes){
	unsigned int t;
	if ( (in_byte == 0x01) || (in_byte == 0x03) || (in_byte == 0x1A) ) 	{
		out_bytes[0] = 0x1A;
		out_bytes[1]=in_byte+0x40;
		return 2;
	}
	out_bytes[0] = in_byte;

	return 1;
}


void setX( char *data, int16_t value){
	data[8] = (int8_t)(value >>8);
	data[9] = (int8_t)(value); 
}


void printHex(char *data, int length){
	int i ;
	for ( i = 0; i<length ; i++)
	{
		printf("%.2X ", data[i] & 0xFF);
	}
	printf("\n");
}


int  create_ETH2CAN_packet(unsigned int canid, unsigned char com, unsigned char len, unsigned char* data, unsigned char * data_out){
	unsigned char b[32];
	int i, j;
	unsigned char cr;
	data_out[0] = TOCAN_ETH2CAN_PROTOCOL_CMD; //Протокол уровня CAN шлюза
	if(canid>7) {
		canid = 0x80000000 | (CAN_TRANSLATE_REQ_COM<<14) | (0<<7) |  (com << 0); 
	}
	else {

		canid = 0x00000000 | (CAN_TRANSLATE_REQ_COM<<6) | (0<<3) | (com << 0); 
	}


	data_out[1] = canid;
	data_out[2] = (canid>>8);
	data_out[3] = (canid>>16);
	data_out[4] = (canid>>24);
	data_out[5] = len ;

	for(	i = 0; i < len; i++) 
			data_out[6+i]=data[i]; //Технологический протокол привода
	j = i+6;

	cr=0;
	for(i = 0 ; i < j; i++) {
		cr ^= data_out[i];
	}
	data_out[j]=cr; 
j++;
/*
	for (i = 0 ; i < j; i++){
		printf("create_ETH2CAN_packet [%d]   \t= %.2X\n", i, data_out[i] );
}
printf("\n");
*/
return j;
} 

int   	printPosition ( int  sph_number ){


unsigned char message[100];
unsigned char data_out[100];
unsigned char question[] = {0x05, 0x07, 0x0A, 0x00, 0x00, 0x00 }; //  registor   Z  0x070A
int 	size=   create_ETH2CAN_packet( 0 , sph_number , sizeof(question) ,  question, data_out) ;

	int MESSAGE_DELIVERED = FALSE ;


	int total  = TranspareMessage ( data_out, message, size);


	server_connect();
	send_message_ETH2CAN( message, total);



  time_t  clock, cycleClock;
  unsigned char TextBuffer[100];
  unsigned char data[100];
  
  time( &clock );
  while(1){
		time(&cycleClock);
 		if ( (cycleClock - clock) >= 10) {
  			printf("Error: message does not received!\n");
	  		return ERROR ;
		  }
		int MessageSize = get_packet_from_ETH2CAN( TextBuffer, sizeof(TextBuffer) );		  
//		int MessageSize = recv(mainSocket,TextBuffer,sizeof(TextBuffer),0);
		if(MessageSize>0) {	
			int i;
//			printf("MessageSize\t= ");
//			printHex(	TextBuffer,	MessageSize	) ;
						
			i = RetranspareMessage ( TextBuffer, data, MessageSize);
			//printRetro( TextBuffer, MessageSize);
			
			if (data[0] == FROMCAN_ETH2CAN_PROTOCOL_CMD) {
				MESSAGE_DELIVERED = TRUE ;
				break;
				}
		}
		else if(MessageSize==-1)	{
			perror("Can't call recv()!\n");
			exit(ERROR);
		}
		else if(MessageSize==0)
			exit(ERROR);
	}

//	printf("data[9] = %.2X \n" , data[9]);
	// 9 because it's CAN_protocole 
	/*
		0byte 			com translete
		1byte-4byte		id
		5byte 			CAN lenght
		6byte			ECG command
		7byte-8byte		adress
		9byte			value	
		 
	*/
	int32_t value = getPosition (data, VALUE_BYTE_POSITION_IN_PACKET, sph_number);
	printf("SERVOPRIVOD %d, CURRENT POSITION:  %d  mm\n\n",sph_number, value);
	return SUCCESS;
  }



RET_CODE_ENUM RetranspareByte ( unsigned char in_byte, unsigned char *out_byte)
{
	RET_CODE_ENUM ret = NONE_BYTE_RECEIVED;
	static struct ControlTranspare ReTranspareStruct;
	/*
	static int byte_position  = 0;
	printf("byte position = %d\n", byte_position++);
	printf("ReTranspareStruct.Type = %d\n", ReTranspareStruct.Type);
	printf("ReTranspareStruct.Ctrl = %d\n", ReTranspareStruct.Ctrl);
	printf("\n");
	*/

	switch (ReTranspareStruct.Type)
	{
	case RxBEGIN:
		if (in_byte == 0x01) {
			ReTranspareStruct.Type = RxDATA;
			ReTranspareStruct.Ctrl = 0;
			}
		break;
	case RxDATA:
		switch (in_byte){
			case 0x03:
				ReTranspareStruct.Type = RxBEGIN;
				ret = PACKET_RECEIVED;
				// Пакет принят
				break;
			case 0x01:
				ReTranspareStruct.Type = RxBEGIN;
				break;
			case 0x1A:
				//Из следующего байта данных нужно будет вычесть 0x40
				ReTranspareStruct.Ctrl = 1;
				break;
	
			default:
				if (ReTranspareStruct.Ctrl == 1) {
				*out_byte = in_byte - 0x40;
				ReTranspareStruct.Ctrl = 0;
				ret = ONE_BYTE_RECEIVED;
				}
				else{
					*out_byte = in_byte;
					ret = ONE_BYTE_RECEIVED;
					}
		}
		
		break;
	}
	
return ret;
}



void printRetro( unsigned char *data, int size )
{
	int i, j;
	unsigned char text[200];
	RET_CODE_ENUM ret;
	unsigned char byte;
	for (i =0 , j= 0;i <size; i++)
	{
		byte = data[i];
		ret = RetranspareByte (byte, &text[j]) ;
//		printf("%d)ret = %d\n", i, ret);
		if (ret == ONE_BYTE_RECEIVED  )
			j++;
	}
	
	printf("before retro: ");
	printHex(&(*data), size);
	printf("after  retro: ");
	printHex(text, j);
	
	
return;
}


int RetranspareMessage (unsigned char *message_in, unsigned char *message_out, int message_in_size){
	RET_CODE_ENUM ret;
	int i ;
	unsigned char byte;
	int j ;
	for (i =0 , j= 0;  i <message_in_size; i++) 	{
		byte = message_in[i] ;
		ret = RetranspareByte (message_in[i], &message_out[j])  ;
		//printf("%d)ret = %d\n", i, ret);
		if (ret == ONE_BYTE_RECEIVED  )
				j++;

	}
return j ;
}


int32_t getValue ( void * data,  int bytePosition)
{
	int32_t * val;
	val = (int32_t * )(&data[bytePosition]) ;
	//printf("val = %d\n", *val);
	int32_t ret = (*val) ;
	//printf( "ret = %d\n", ret);
	
	return  ret ;
}

int16_t getInt16Value ( void * data,  int bytePosition)
{
	int16_t * val;
	val = (int16_t * )(&data[bytePosition]) ;
	//printf("val = %d\n", *val);
	int16_t ret = (*val) ;
	//printf( "ret = %d\n", ret);
	return  ret ;
}

int32_t getPosition ( void * data,  int bytePosition, int can_port)
{

	int32_t * val;
	val = (int32_t * )(&data[bytePosition]) ;
	//printf("val = %d\n", *val);
	int32_t result;
	
	if (can_port == servoS3Y) {
		result =  (S3Y_POLOJENIE_KONCEVIKA - *val) / SPH_FACTOR ;
	}
	else {
		result = (*val) / SPH_FACTOR;
	}
	//printf( "ret = %d\n", ret);
	
	return  result ;
}




void server_connect()
{
  if((hp = gethostbyname(HOSTNAME))==0)	{
	perror("Can't call gethostbyname()!\n");
	exit(ERROR);
	}

//  struct sockaddr_in ServerAddress;
  bzero(&ServerAddress,sizeof(ServerAddress));
  bcopy(hp->h_addr,&ServerAddress.sin_addr,hp->h_length);
  ServerAddress.sin_family = hp->h_addrtype;
  ServerAddress.sin_port = htons(PORT_NUMBER);

  mainSocket = socket(AF_INET,SOCK_STREAM,0);
  if(mainSocket==-1)	{
	perror("Can't call socket()!\n");
	exit(ERROR);
	}

  if(connect(mainSocket,(struct sockaddr *)&ServerAddress,sizeof(ServerAddress))==-1)	{
	perror("Can't call connect() to !\n");
	exit(ERROR);
	}

// printf("Client: OK\n");
}

void server_close()
{
	close (mainSocket);
}

void   send_message_ETH2CAN( unsigned char *message, int total)
{
	int size ;
	size =  send(mainSocket, message,  total, 0);
	if ( size == -1 ){
		printf("ERROR!!! Coudn't send message to socket\n");
		exit (ERROR);
	}
  
}




int  get_packet_from_ETH2CAN( unsigned char * buff, int buffSize )
{

	int size ;
	
	struct pollfd file_descriptor;
	file_descriptor.fd = mainSocket;
	file_descriptor.events = POLLIN;
	
	int rezult ;
	int u_timeOut = 1000 ; // 
	rezult = poll (&file_descriptor, 1 , u_timeOut) ;
	
	switch (rezult ){
		// ERROR
		case -1 :
			printf("ERROR in get_packet_from_ETH2CAN \n");
			printf("ERROR!!! Coudn't get message from socket\n");
			exit (ERROR) ;
		// Timeout 
		case  0 :
			size =  EMPTY_MESSAGE;
			break;
		// success. there are events in file_descriptor
		default :
			size = recv(mainSocket, buff , buffSize,0);
			break;		
		 
	}

	return size ;
}




int  	move_privod(int sph_number, int destination)
{

	unsigned char change_up0[] = {	0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char code_move[] = {	0x11, 0xFF, 0xFF, 0x02, 0x00, 0x00, 0x00	};
	unsigned char heartbeating[] = { 0x01, 0x05, 0x09 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};

	unsigned char data_out[100];
	unsigned char message[100];
	unsigned char answer[100];
	writeIntInChar (&change_up0[3],  (( int32_t)destination*SPH_FACTOR) );

//         1 message	change up0 registor
	memset(data_out, 0, sizeof(data_out)) ;
	memset(message, 0, sizeof(message) );	
	int 	size=   create_ETH2CAN_packet( 0 , sph_number , sizeof(change_up0) ,  change_up0, data_out) ;
	int MESSAGE_DELIVERED = FALSE ;
	int total  = TranspareMessage ( data_out, message, size);

	connect_with_ETH2CAN();
	send_message_ETH2CAN( message, total);
	usleep(100000);


//         2 message  start program in bank 2
	memset(data_out, 0, sizeof(data_out)) ;
	memset(message, 0, sizeof(message) );
	size=   create_ETH2CAN_packet( 0 , sph_number , sizeof(code_move) ,  code_move, data_out) ;
	total  = TranspareMessage ( data_out, message, size);
	send_message_ETH2CAN( message, total);
//	sleep(1);	
	
/*
//			3	get servo status - registor Dd11
	memset(data_out, 0, sizeof(data_out)) ;
	unsigned char messageGetStatusDd11[100];
	int totalSizeGetStatusDd11 = 0 ;
	size = 0 ;
	unsigned char servo_statusDd11[]= {  0x05, 0x04, 0x0F, 0x00, 0x00, 0x00, 0x00};	
	size=   create_ETH2CAN_packet( 0 , sph_number , sizeof(servo_statusDd11) ,  servo_statusDd11, data_out) ;
	totalSizeGetStatusDd11  = TranspareMessage ( data_out, messageGetStatusDd11, size);
//	send_message_ETH2CAN( messageGetStatusDd11, totalSizeGetStatusDd11);


// 			4 	get current speed - registor Dd4
	memset(data_out, 0, sizeof(data_out)) ;
	unsigned char currentSpeed_Dd4[] = {  0x05, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00};
	unsigned char messageGetSpeedDd4[100];
	int sizeDd4=   create_ETH2CAN_packet( 0 , sph_number , sizeof(currentSpeed_Dd4) ,  currentSpeed_Dd4, data_out) ;
	int totalSizeDd4  = TranspareMessage ( data_out, messageGetSpeedDd4, sizeDd4);

// 			5 	get current position - registor Dd7
	memset(data_out, 0, sizeof(data_out)) ;
	unsigned char currentSpeed_Dd7[] = {  0x05, 0x04, 0x0B, 0x00, 0x00, 0x00, 0x00};
	unsigned char messageGetPositionDd7[100];
	int sizeDd7=   create_ETH2CAN_packet( 0 , sph_number , sizeof(currentSpeed_Dd7) ,  currentSpeed_Dd7, data_out) ;
	int totalSizeDd7  = TranspareMessage ( data_out, messageGetPositionDd7, sizeDd7);


//	Lp11StatusInterpretatora
	memset(data_out, 0, sizeof(data_out)) ;
	unsigned char StatusInterpretatora_Lp11[] = {  0x05, 0x00, 0x42, 0x00, 0x00, 0x00, 0x00};
	unsigned char messageGetStatusInterpretatora_Lp11[100];
	int sizeLp11=   create_ETH2CAN_packet( 0 , sph_number , sizeof(StatusInterpretatora_Lp11) ,  StatusInterpretatora_Lp11, data_out) ;
	int totalSizeLp11  = TranspareMessage ( data_out, messageGetStatusInterpretatora_Lp11, sizeLp11);


//	Dd19PositionMistake
	memset(data_out, 0, sizeof(data_out)) ;
	unsigned char commandDd19PositionMistake[] = {  0x05, 0x04, 0x0C, 0x00, 0x00, 0x00, 0x00};
	unsigned char messageGetDd19PositionMistake[100];
	int sizeDd19=   create_ETH2CAN_packet( 0 , sph_number , sizeof(commandDd19PositionMistake) ,  commandDd19PositionMistake, data_out) ;
	int totalSizeDd19  = TranspareMessage ( data_out, messageGetDd19PositionMistake, sizeDd19);
*/
		
usleep(100000);		


int result = checkStatusANDDestinationWhileMoving ( sph_number , destination) ;
disconnect_with_ETH2CAN();
if (result == ERROR) return ERROR ;	

	//time (&endtime);
	//printf("Success, alltime = %d\n", (endtime - starttime) ) ;
return SUCCESS ;
}



int 	wait_for_answer_for_ETH2CAN ( unsigned char *answer , int aswerMaxSize )
{
	int result = ERROR;
	time_t clock, cycleClock;
	printf("Function  wait_for_answer_for_ETH2CAN(...)	START\n");
	int k =0;
	int step1 = 0;
	int step2 = 0;
	int messageSize = 0;
	unsigned char dataout[100];
	time( &clock );
	
	unsigned char TextBuffer[100];
	
  while(1)
	{
	int MessageSize = recv(mainSocket,TextBuffer,sizeof(TextBuffer),0);
	if(MessageSize>0)
	  	{
		printf("MessageSize:%c:  ",MessageSize);
		int i;
		for(i=0;i<MessageSize;i++)
			printf("%.2X ",TextBuffer[i] & 0xFF);
		printf("\n");
		}
	else if(MessageSize==-1)
		{
		perror("Can't call recv()!\n");
		exit(ERROR);
		}
	else if(MessageSize==0)
		exit(SUCCESS);
	}
	
  while(1){
  		time( &cycleClock );
  		if ( (cycleClock-clock) >= k){
  			k++;
  			printf(". ");
  			}
		if (cycleClock-clock > DEAD_TIME_WAITING ){
  			printf("Error: ETH2CAN not responding\n");
  			exit (ERROR);
//	return ERROR; // return 0
			}
		messageSize = get_message_from_packet( dataout  ) ;
		if (messageSize >0){
			if ( dataout[0] != 0xFF) {
				if (aswerMaxSize<=messageSize)	{
						memcpy ( answer , dataout, messageSize);
						return messageSize;
					}
				else
  					exit (ERROR);
  			//			return ERROR; // return 0
				}
			}
		}
}






void   	printList(void )
{

	unsigned char dataout[1000] ;

	//unsigned char datain[] = {0x01, 0x16, 0x47, 0x1A, 0x43, 0x05, 0x00, 0x35, 0xCC, 0x03} ;
//	unsigned char datain[] = {0x01, 0x16, 0x47, 0x1A, 0x43, 0x05, 0x00, 0x35, 0xCC, 0x03} ;
//	unsigned char datain[] = {0x01, 0x16, 0x45, 0x1A, 0x43, 0x00, 0x00, 0x1A, 0x43, 0x05, 0x00, 0x35, 0x61, 0x03} ;
//	unsigned char datain[] = {0x16, 0x45, 0x1A, 0x43, 0x00, 0x00, 0x1A, 0x43, 0x05, 0x00, 0x35, 0x61, 0x03} ;
	unsigned char datain[] = { 0x05, 0x00, 0x35, 0x00, 0x00, 0x00} ;
	


	time_t  clock, cycleClock;
	int SPH_base[7];
	memset (SPH_base, 0, sizeof(SPH_base) );
	int k =0;
	int step1 = 0;
	int step2 = 0;
	int messageSize = 0;
	
	
	int size ;
	int total; 
	unsigned char data_out[1000];
	unsigned char message[1000];
	memset(data_out, 0, sizeof(data_out)) ;
	memset(message, 0, sizeof(message) );

	memset(data_out, 0, sizeof(data_out)) ;
	memset(message, 0, sizeof(message) );
	size=   create_ETH2CAN_packet( 0 , 7 , sizeof(datain) ,  datain, data_out) ;
	total  = TranspareMessage ( data_out, message, size);
//	send_message_ETH2CAN( message, total);

/*
	servosInSystem allServo[6];
	setDefaultParameters ( allServo );
	*/
	
	server_connect();
	//send_message_ETH2CAN(datain, sizeof(datain) ) ;
	send_message_ETH2CAN( message, total);
	
	int timeOut = DEAD_TIME_WAITING ; 

	int servoSPH[6] ;
	int canid =0 ;
	memset (&servoSPH , 0,  sizeof (servoSPH) );
	
	time( &clock );
	//printf("clock = %lu\n", clock);
	printf("Searching ");

  while(1){
  		time( &cycleClock );
  		//printf("cycleClock = %lu\n", cycleClock);  	
  		if ( (cycleClock-clock) >= ONE_SECOND){
  			clock = cycleClock ;
  			timeOut--;
  			printf(". ");
  			}
  		// exit from while 
		if (timeOut < 0   ){
  			printf("\n");
			break;
			}
		messageSize = get_message_from_packet( dataout  ) ;
		
		if (messageSize >0){
			if ( (dataout[0] != 0xFF) && (messageSize > 5) ) {

					int  i = 0, totalSph = 0, canid;
					for (i =0 ; i< messageSize; i++){
						if (dataout[i] == 0x17) totalSph++;
					}
					//printf("totalSph = %d\n", totalSph);
					if (totalSph > 0 ){
						for( i = 0 ; i < messageSize; i++){
							if ((dataout[i] == 0x17) && ((i +VALUE_BYTE_POSITION_IN_PACKET)<=messageSize) ){
								canid = getInt16Value (dataout, i+VALUE_BYTE_POSITION_IN_PACKET);
								//servoSPH[value] = 1 ;
								SPH_base[canid] = TRUE;
								
							}
						}
					}
					/*
					int canid = getCanId(dataout);
					memset(dataout, '\0' , messageSize);
					SPH_base[canid] = TRUE;
					/**/
					messageSize = 0;

				}
			//printf("SPH_base[%d] = TRUE;\n", canid);
			}
			
			
		}
		// для каждого найденного COM-порта узнать ID устройства
		for (canid = 1 ; canid<7; canid++){
			if (SPH_base[canid] == TRUE){
				// получаем ИД ус-ва
				// печать состояния ус-ва
				
			}
		}
		
		server_close();
		usleep(20000);
		
		printf("\n");

	if (SPH_base[S3X] == TRUE )
		printf("%s (com %d) IS AVAILABLE\n", ServoNames[S3X], S3X );
	else
		printf("%s (com %d) IS DISABLE\n", ServoNames[S3X], S3X);			

	if (SPH_base[S3Y] == TRUE )
		printf("%s (com %d) IS AVAILABLE\n", ServoNames[S3Y], S3Y);
	else
		printf("%s (com %d) IS DISABLE\n", ServoNames[S3Y], S3Y);			

	if (SPH_base[K1X] == TRUE )
		printf("%s (com %d) IS AVAILABLE\n", ServoNames[K1X], K1X);
	else
		printf("%s (com %d) IS DISABLE\n", ServoNames[K1X], K1X);			

	if (SPH_base[K1Y] == TRUE )
		printf("%s (com %d) IS AVAILABLE\n", ServoNames[K1Y], K1Y);
	else
		printf("%s (com %d) IS DISABLE\n", ServoNames[K1Y], K1Y);			

	if (SPH_base[K2X] == TRUE )
		printf("%s (com %d) IS AVAILABLE\n", ServoNames[K2X], K2X);
	else
		printf("%s (com %d) IS DISABLE\n", ServoNames[K2X], K2X);			

	if (SPH_base[K2Y] == TRUE )
		printf("%s (com %d) IS AVAILABLE\n", ServoNames[K2Y], K2Y);
	else
		printf("%s (com %d) IS DISABLE\n", ServoNames[K2Y], K2Y);			


}

int	getCanId(void   *data)
{
	int  * num = (int *)(&data[VALUE_BYTE_POSITION_IN_PACKET]);
	int val = *num;
	printf("CanId = %d\n", val);
	return val;
}

int get_message_from_packet( unsigned char * data) 
{
  time_t  clock, cycleClock;
  unsigned char TextBuffer[1000];
    int size = 0;

		int MessageSize = get_packet_from_ETH2CAN( TextBuffer, sizeof(TextBuffer) );		  

		if (MessageSize > sizeof(TextBuffer)) {
			printf("Error! MessageSize > %4lu bytes\n", sizeof(TextBuffer) );
			exit(ERROR) ;	
		}
		if(MessageSize==-1)	{
			printf("Can't call recv()!\n");
			exit(ERROR) ;
		}
		
		if (MessageSize == 0 ){
			//printf("MessageSize == 0\n");
			size = EMPTY_MESSAGE;		
		}
				
		// success
		if(MessageSize>0) {		
		//printRetro( TextBuffer, MessageSize);
		size = RetranspareMessage ( TextBuffer, data, MessageSize);
		}
		return size ;	
		
		
}

uint16_t getParametrAdress( void * data_in)
{	
	uint16_t* num ;
	num = (uint16_t*)(&data_in[ADRESS_BYTE_POSITION_IN_PACKET]);
	uint16_t ret = *num;
	//printf("getParametrAdress: ");
	//printHex (&ret, 2);
	return ret ;
}







int  executeProgramInBank(int  sph_number, int bank_number )
{

	unsigned char code_move[] = {	0x11, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00	};
	writeIntInChar (&code_move[3],  (( int32_t)bank_number) );

	unsigned char data_out[100];
	unsigned char message_start_bank0[100];

	int total, size;
	server_connect();
 
//         2 message  start program in bank 1
	memset(data_out, 0, sizeof(data_out)) ;
	memset(message_start_bank0, 0, sizeof(message_start_bank0) );
	size=   create_ETH2CAN_packet( 0 , sph_number , sizeof(code_move) ,  code_move, data_out) ;
	total  = TranspareMessage ( data_out, message_start_bank0, size);
	send_message_ETH2CAN( message_start_bank0, total);
//	sleep(1);	


int result = checkStatusWhileMoving ( sph_number ) ;


if (result == ERROR) return ERROR ;
	
		
	//time (&endtime);
//	printf("Success, alltime = %d\n", (endtime - starttime) ) ;
return SUCCESS ;


}


int checkStatusWhileMoving (int  sph_number ){

int result =  checkStatusANDDestinationWhileMoving ( sph_number, 0 );
return result; 
}




int checkStatusANDDestinationWhileMoving (int  sph_number , int destination ){

unsigned char data_out[100];
unsigned char message[100];

unsigned char TextBuffer[100];
unsigned char data_in[100];

int32_t value  ;	
time_t clock, cycleClock;
int MessageSize, size, counter;

	memset(data_out, 0, sizeof(data_out)) ;
	memset(message, 0, sizeof(message) );
	memset(data_in, 0, sizeof(data_in) );



	//			3	get servo status - registor Dd11
	memset(data_out, 0, sizeof(data_out)) ;
	unsigned char messageGetStatusDd11[100];
	int totalSizeGetStatusDd11 = 0 ;
	size = 0 ;
	unsigned char servo_statusDd11[]= {  0x05, 0x04, 0x0F, 0x00, 0x00, 0x00, 0x00};	
	size=   create_ETH2CAN_packet( 0 , sph_number , sizeof(servo_statusDd11) ,  servo_statusDd11, data_out) ;
	totalSizeGetStatusDd11  = TranspareMessage ( data_out, messageGetStatusDd11, size);
//	send_message_ETH2CAN( messageGetStatusDd11, totalSizeGetStatusDd11);


//	Dd19PositionMistake
	memset(data_out, 0, sizeof(data_out)) ;
	unsigned char commandDd19PositionMistake[] = {  0x05, 0x04, 0x0C, 0x00, 0x00, 0x00, 0x00};
	unsigned char messageGetDd19PositionMistake[100];
	int sizeDd19=   create_ETH2CAN_packet( 0 , sph_number , sizeof(commandDd19PositionMistake) ,  commandDd19PositionMistake, data_out) ;
	int totalSizeDd19  = TranspareMessage ( data_out, messageGetDd19PositionMistake, sizeDd19);




	

	usleep(100000);		

//  check na zastrevanie		
//printf("1) check na zastrevanie\n");
	time(&clock);
	send_message_ETH2CAN( messageGetStatusDd11, totalSizeGetStatusDd11);
	 while(1)
	{
 		time(&cycleClock);
		if ( (cycleClock - clock) >= 8) send_message_ETH2CAN( messageGetStatusDd11, totalSizeGetStatusDd11);
 		if ( (cycleClock - clock) >= 10) {
		printf("ERROR! Servomotor does not response!\n");
		return ERROR;
		}
		//get_packet_from_ETH2CAN
		MessageSize = get_packet_from_ETH2CAN( TextBuffer, sizeof(TextBuffer) );
		//if ( (MessageSize  = recv(mainSocket,TextBuffer,sizeof(TextBuffer),0)  ) == 0 ){
		if (MessageSize <= 0 ){	
			printf("848\n");	
	        fprintf( stderr, "Failed receiving data.\n" );
            return(ERROR);
        }	

			memset(data_in, 0, sizeof(data_in) ) ;	
			//printf("Message:\t");
			//printHex(TextBuffer, MessageSize);	
			//printRetro(TextBuffer, MessageSize);	
			int i = RetranspareMessage ( TextBuffer, data_in, MessageSize);
			if ( Dd11ServoStatus == getParametrAdress( data_in) ){
				//printf("Dd11ServoStatus == getParametrAdress( data_in)\n");
				value = getInt16Value (data_in, VALUE_BYTE_POSITION_IN_PACKET);
				if (value == 0){
					break;
				}
				else{
					printf("ERROR! Servomotor stuck! Dd11 = %d \n",value );
					return ERROR;
				}
			}	
	}		



int stat = 0 ;
usleep(20000);
int32_t  old_value ;
//  waiting while speed of servo get to null 
time(&clock);
	send_message_ETH2CAN ( messageGetDd19PositionMistake, totalSizeDd19  ) ;	
	printf("Status: moving  ");
  while(1)
	{
		time(&cycleClock);
 		if ( (cycleClock - clock) >= 25) {
		printf("ERROR! Exceeded the time limit while geting to zerro position\n");
		return ERROR;
		}
		MessageSize = get_packet_from_ETH2CAN( TextBuffer, sizeof(TextBuffer) );
		if ( MessageSize <= 0 ){
			printf("889\n");
			fprintf( stderr, "Failed receiving data.\n" );
			return(ERROR);
		}	
			//printf("Message\t\t= ");
			//printHex( TextBuffer, MessageSize);
			memset(data_in, 0, sizeof(data_in) ) ;		
			int i = RetranspareMessage ( TextBuffer, data_in, MessageSize);
			if (REGISTOR_Dd19PositionMistake		== getParametrAdress( data_in) ){
				value = getPosition (data_in, VALUE_BYTE_POSITION_IN_PACKET, sph_number);
				usleep(250000);
				if ((stat++ % 2) ==0)
					printf("\b+");
				else printf("\b ");
				//printf("Current position \t= %d\n", value);
				
				// если проверка соответсвия назначению и тек. положение нужна - выполняем if
				if (destination == 0 ){
//					printf("destination ");
					if ( value == old_value ){
						counter++ ;
					}
					else { 
						counter = 0 ;
						old_value = value ;
					}
				}
				else{
//				printf("destination = %d  \n", destination);
					if ( value == destination ){
						counter++ ;
					}
					else { 
						counter = 0 ;						
					}
				}
				
				if (counter >=10)
					break;
			}
			send_message_ETH2CAN ( messageGetDd19PositionMistake, totalSizeDd19  ) ;					
     }
printf("\n");


//usleep(20000);
time(&clock);
	send_message_ETH2CAN( messageGetStatusDd11, totalSizeGetStatusDd11);
	 while(1)
	{
 		time(&cycleClock);
		if ( (cycleClock - clock) >= 8) send_message_ETH2CAN( messageGetStatusDd11, totalSizeGetStatusDd11);
 		if ( (cycleClock - clock) >= 10) {
		printf("ERROR! Servomotor does not response!\n");
		return ERROR;
		}
		MessageSize = get_packet_from_ETH2CAN( TextBuffer, sizeof(TextBuffer) );
		if ( MessageSize <= 0 ){
			printf("947\n");
			fprintf( stderr, "Failed receiving data.\n" );
			return(ERROR);
		}	
			memset(data_in, 0, sizeof(data_in) ) ;	
			//printf("Message:\t");
			//printHex(TextBuffer, MessageSize);	
			//printRetro(TextBuffer, MessageSize);	
			int i = RetranspareMessage ( TextBuffer, data_in, MessageSize);
			if ( Dd11ServoStatus == getParametrAdress( data_in) ){
				//printf("Dd11ServoStatus == getParametrAdress( data_in)\n");
				value = getInt16Value (data_in, VALUE_BYTE_POSITION_IN_PACKET);
				if (value == 0){
					break;
				}
				else{
					printf("ERROR! Servomotor stuck! Dd11 = %d \n",value );
					return ERROR;
				}
			}
	}		

return SUCCESS;
}



int32_t getFactoryID (int comPortNumber)
{
	int32_t factoryID = 0 ;
	int messageSize = 0 ;
	unsigned char messageGetSt3_Factory_ID[100];
	memset( messageGetSt3_Factory_ID, 0 , sizeof(messageGetSt3_Factory_ID) );
	
	unsigned char dataout[1000];
	memset(dataout, 0, sizeof(dataout)) ;
	
	// создаем команду для CAN-шлюза, (для выбранного порта) запрашивая заводской номер устройства.
	
	int totalsizeSt3 = createGetFactoryID_Message( messageGetSt3_Factory_ID, comPortNumber );
	
	// посылаем команду в шлюз
	send_message_ETH2CAN( messageGetSt3_Factory_ID, totalsizeSt3);
	

	int timeOut = DEAD_TIME_WAITING;
	time_t clock, cycleClock ;	
	time( &clock );

	//printf("Searching ");
	usleep(10000);
// входим в цикл, ждем ответ в течение нескольких секунд	
	while(1)
	{
		// если время ожидания закончилось 
		// выводим сообщение и завершаем программу ошибкой
	  		time( &cycleClock );
  		//printf("cycleClock = %lu\n", cycleClock);  	
  		if ( (cycleClock-clock) >= ONE_SECOND){
  			clock = cycleClock ;
  			timeOut--;
  			//printf(". ");
  			}
  		// exit from while 
		if (timeOut < 0   ){
  			//printf("\n");
			break;
			}
			
		//получаем сообщение
		messageSize = get_message_from_packet( dataout  ) ;
	   // если ответ получен 
	   		// расшифровываем посылку и получаем номер устройтва
	   		// и выходим из цикла
   		if (messageSize >0){
			if ( (dataout[0] != 0xFF) && (messageSize > 5) ) {
				 int isOkey= 0 ;
				 isOkey = isThereRightMessage (dataout, messageSize);
				 if ( isOkey ){
					//printHex(dataout, 1000);
					factoryID = getValue (dataout, VALUE_BYTE_POSITION_IN_PACKET);
					//printf("-factoryID = %d \n", factoryID );
					//printf("\n");
					break;
				}
				messageSize = 0;
			}
		}
		
	   		
	}
	
 
	// возвращаем заводской номер устройства
	return factoryID ;
}


int isThereRightMessage (unsigned char *data, int messageSize)
{
	if( (data[0] == 0x17) && (messageSize >= VALUE_BYTE_POSITION_IN_PACKET)){
		return 1;
	}
	else{
		return 0;
	}
}


int createGetFactoryID_Message( unsigned char *messageGetSt3_Factory_ID, int comPortNumber )
{
	int sizeOfMessage = 0 ;
	
	unsigned char packetETH2CAN[100];
	memset(packetETH2CAN, 0, sizeof(packetETH2CAN) ) ;
	unsigned char commandSt3_Factory_ID[] = {  0x05, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00};
	//unsigned char messageGetSt3_Factory_ID[100];
	int sizeSt3=   create_ETH2CAN_packet( 0 , comPortNumber , sizeof(commandSt3_Factory_ID) ,  commandSt3_Factory_ID, packetETH2CAN) ;
	sizeOfMessage  = TranspareMessage ( packetETH2CAN, messageGetSt3_Factory_ID, sizeSt3);

	return sizeOfMessage;
}

id_type getIDforThisMassive( unsigned char * data)
{
	id_type device_id = 0 ;
	
	printf("data: %s\n", data);
	
	if ( strncmp (data, "s3x", 3) == 0 || strncmp (data, "S3X", 3) == 0 ){
		device_id = S3X_ID ;
	}
	else if ( strncmp (data, "s3y", 3) == 0 || strncmp (data, "S3Y", 3) == 0 ){
		printf("Don't now ID of device. Call to MOTOMASTER administrator\n");
		device_id = S3Y_ID ;
	}
	else if ( strncmp (data, "k1x", 3) == 0 || strncmp (data, "K1X", 3) == 0 ){
		device_id = K1X_ID ;
	}	
	else if ( strncmp (data, "k1y", 3) == 0 || strncmp (data, "K1Y", 3) == 0 ){
		device_id = K1Y_ID ;
	}		
	else if ( strncmp (data, "k2x", 3) == 0 || strncmp (data, "K2X", 3) == 0 ){
		device_id = K2X_ID ;
	}	
	else if ( strncmp (data, "k2y", 3) == 0 || strncmp (data, "K2Y", 3) == 0 ){
		device_id = K2Y_ID ;
	}		
	
	return device_id ;
}

int findCompPortForThisDevice( id_type factoryID )
{
	int comPortNumber ;
	id_type currentID = 0 ;
	connect_with_ETH2CAN();
	
	for (comPortNumber =1 ; comPortNumber<7 ; comPortNumber++){
		currentID = getFactoryID (comPortNumber) ;
		//printf("currentID = %d\n", currentID);
		if ( currentID == factoryID){
			disconnect_with_ETH2CAN();
			usleep(100000);
			return comPortNumber ;
		}
	}
	disconnect_with_ETH2CAN();
	usleep(100000);
	// error. coudn't find anything.
	return 0;
}



void connect_with_ETH2CAN() 
{
	 server_connect( );
}

void disconnect_with_ETH2CAN() 
{
	server_close();
}

/*
void setDefaultParameters ( servosInSystem * allServo )
{	
	int comPort;
	for (comPort = 0 ; comPort < 6; comPort++){
		allServo[comPort].isAvailable = FALSE ;
		allServo[comPort].ID = 0 ;
	}

}
*/








