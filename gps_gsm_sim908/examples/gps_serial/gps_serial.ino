/********************* start of gps_serial.ino *************************/

/*
 *	created:	2013-08-02
 *	by:		lisper (leyapin@gmail.com)
 *	Version:	0.1
 *	test gps gsm on Leonardo &XBEE R3
 *
 */

#include "gps_gsm_sim908.h"

//
void setup () {
	gps_init ();	//init GPS pin

	Serial.begin (9600);	//serial0 connect computer
	while (!Serial);

	Serial1.begin (9600);	//serial1 connect GPS
	while (!Serial1);

	Serial.println ("start GPS! ");

	start_gps ();	//open GPS
}

//
void loop () {
	int stat = gps_get_gga ();	// read data from GPS, return 0 is ok

	//
	switch (stat) {
	case 0:
		Serial.println ("data checksum is ok");
		break;
	case 1:
		Serial.println ("error! data checksum is fail");
		break;
	case 2:
		Serial.println ("GPGGA ID is error!");
		break;
	case 3:
		Serial.println ("data is error!");
	}

	Serial.println ("$GPGGA data:");
	gps_gga_print ();	//for test

	

	if (gps_gga_is_fix () == 0)	//if fix return 1
		Serial.println ("can't fix! please go outside!");
	else 
		Serial.println ("ok! is fix!");

	Serial.println ("gps_gga_utc_hh ()");
	Serial.println (gps_gga_utc_hh ());
	Serial.println ("gps_gga_utc_mm ()");
	Serial.println (gps_gga_utc_mm ());
	Serial.println ("gps_gga_utc_ss ()");
	Serial.println (gps_gga_utc_ss ());

	Serial.println ("gps_gga_NS ()");
	Serial.println (gps_gga_NS ());
	Serial.println ("gps_gga_EW ()");
	Serial.println (gps_gga_EW ());

	Serial.println ("gps_gga_lat ()");
	Serial.println (gps_gga_lat (), 6);
	Serial.println ("gps_gga_long ()");
	Serial.println (gps_gga_long (), 6);
	Serial.println ("gps_gga_HDOP ()");
	Serial.println (gps_gga_HDOP (), 6);
	Serial.println ("gps_gga_MSL ()");
	Serial.println (gps_gga_MSL (), 6);
	Serial.println ();
}

/********************* end of gps_serial.ino *************************/
