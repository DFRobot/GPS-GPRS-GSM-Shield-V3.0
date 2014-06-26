/********************* start of gps_gsm.ino *************************/

/*
 *	created:	2013-08-02
 *	by:		lisper (leyapin@gmail.com)
 *	Version:	0.1
 *	test gps gsm on Leonardo &XBEE R3
 *
 */


#include "gps_gsm_sim908.h"

//for test, changed it to yours phone numble 
char *phone_num = "18501683475";


//
void setup () {
	gps_init ();	//init GPS pin


	Serial.begin (9600);	//serial1 connect GPS
	while (!Serial);


	start_gps ();	//open GPS
}

//
void loop () {
	gsm_begin_msg (phone_num);
	gsm_send_msgln ("hello world\n");
	gsm_stop_send ();
	while (1);

}


/********************* end of gps_gsm.ino *************************/
