// onl-servo  https://github.com/bowman85/test_onl-servo1/tree/master/onl-servo
// my comment

#include "main.h"

int  main(int ac, char** av)
{

	struct hostent *hp;
	int sph_number, destination;
   
	setvbuf(stdout, 0, _IONBF ,0);  					    
  

if (ac <2) {
	printUsage();
	return SUCCESS; 
	}

int a;
	int rezult;

//		PRINT	
if ( strcmp(av[1], "print") == 0) {
	if(ac != 3) {
		printUsage ( );
		return ERROR;
	}	
 	sph_number = atoi(av[2]);
  	if (sph_number <1 || sph_number >6 )	{  	     
  		printf("Error: sph number must be  1, 2, 3, 4, 5 or 6\n");
  		printUsage ( );
		return ERROR;
  	} 	  	
  	printPosition ( sph_number );
		return SUCCESS;
}

//		MOVE COMPORT TO
else if ( strcmp (av[1], "moveto") == 0 )
{
	if(ac != 4) {
		printUsage ( );
		return ERROR;
	}
  	sph_number = atoi(av[2]);
  	destination = atoi(av[3]);
  	if (destination  < 0 )	{
  	     printUsage ( );
		return ERROR;
  	}

	
//  для S3Y(2com)  положение отсчитывается по формуле P=X-Z а не по P=Z
// где X - положение концевика 141966

	if (sph_number == servoS3Y){
		printf("moveto for S3Y \n");
		int s3yDestination ;
		//printf ("destination = %d\n", destination);
		s3yDestination = ( S3Y_POLOJENIE_KONCEVIKA  - ( destination*SPH_FACTOR )) / SPH_FACTOR ;
		//printf ("destination2 = %d\n", s3yDestination);
		
		rezult =  	move_privod(sph_number, s3yDestination);
		if (rezult == ERROR){
			printf("ERROR while moving servoprivod %d on %d mm\n", sph_number, destination);
			return ERROR ;
		}		
		printf("SUCCESS! SERVOPRIVOD %d, POSITION: %d  mm\n\n", sph_number, destination);
	  	return SUCCESS;
	}
//-----------


	rezult =  	move_privod(sph_number, destination);
	if (rezult == ERROR){
		printf("ERROR while moving servoprivod %d on %d mm\n", sph_number, destination);
		return ERROR ;
	}		
	printf("SUCCESS! SERVOPRIVOD %d, POSITION: %d  mm\n\n", sph_number, destination);
  	return SUCCESS;

}

//		LIST
else if ( strcmp (av[1], "list") == 0 ){
  	// make magic
	  	printList();
		return SUCCESS;
		}
		
//		BANK		
else if (strcmp (av[1], "bank") == 0 ){
	if(ac != 4) 	{
		printUsage ( );
		return ERROR;
	}
  	sph_number = atoi(av[2]);
  	int bank_number = atoi(av[3]);
  	if (bank_number  < 0 || bank_number >7)	{
  	     printUsage ( );
		return ERROR;
  	}
  	rezult = executeProgramInBank( sph_number, bank_number );
  	if ( rezult == ERROR){
  		printf("ERROR while executing programm in bank %d  for %d  servoprivod\n", bank_number, sph_number) ;
  		return ERROR;
  	}
  	else{
  		printf("SUCCESS! SERVOPRIVOD %d, PROGRAMM IN %d BANK IS EXECUTED\n\n", sph_number, bank_number);  	
	  	return SUCCESS;
	  	}
}

//		GET_ID
else if ( strcmp (av[1], "getid") == 0 )
{
	if(ac != 3) {
		printUsage ( );
		return ERROR;
	}
	int comPortNumber;
	int32_t factoryID ;
	comPortNumber = atoi(av[2]);
  	if (comPortNumber  < 1 && comPortNumber >7)	{
  	     printUsage ( );
		return ERROR;
  	}
  	else{
  		connect_with_ETH2CAN();
  		factoryID = getFactoryID (comPortNumber);	
  		disconnect_with_ETH2CAN();
		if (factoryID <= 0){
			printf("ERROR. Can't get factory ID\n");
			return ERROR ;
		}		
		printf("COMPORT %d has this ID: %8d \n\n", comPortNumber, factoryID);
	  	return SUCCESS;
  	}
}

/*
else if ( strcmp (av[1], "moveto") == 0 )
{
	if(ac != 4) {
		printUsage ( );
		return ERROR;
	}
	// если dest не число - ошибка и завершение
  	destination = atoi(av[3]);
  	if (destination  < 0 )	{
  	     printUsage ( );
		exit (ERROR);
  	}
	id_type factoryID;

	// сверяем 3тий аргумент с записями компорта типа S3Y
	// определяем ID нужного нам ус-ва по своей "базе"	
	factoryID = getIDforThisMassive( av[2]);
	
	// если совпадений человеческого названия серва нет - ошибка и завершение
	if (factoryID == 0 ){
		printf("ERROR. Device names should be like S3X, S3Y, K1X, K1Y, K2X or K2Y\n");
		exit (ERROR);
	}
	
	printf("our factoryID of device = %d\n", factoryID);

	int comPortNumber;
	// определяем наличие в сети устройств, считывая COM-порт и  ID уст-ва
	comPortNumber = findCompPortForThisDevice( factoryID );	
	// если нужного нам ID  нет - завершение проги
	if (comPortNumber == 0 ){
		printf("ERROR. There is no device with %d factoryID \n", factoryID);
		exit(ERROR);
	}
	
	printf("comPortNumber = %d\n", comPortNumber);	
	// на данный COM-порт посылаем команду езды и т.д.	
	rezult =  	move_privod(comPortNumber, destination);
	if (rezult == ERROR){
		printf("ERROR while moving servoprivod %d on %d mm\n", comPortNumber, destination);
		return ERROR ;
	}		
	printf("SUCCESS! SERVOPRIVOD %d, POSITION: %d  mm\n\n", comPortNumber, destination);
  	return SUCCESS;
  	
}

*/

 
else {
		printUsage ( );
		return  ERROR;
	}

}

