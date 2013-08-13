/********************* start of file *************************/

/*
 *	by 2013-08-02
 *	test on Leonardo &XBEE, can't use on Arduino Uno board
 *	Serial1 <-> GPS and Serial <-> computer
 *
 */

//debug
//#define DEBUG

#include <Arduino.h>


#define gps_enable()	digitalWrite (4, LOW)
#define gps_disable()	digitalWrite (4, HIGH)

#define gsm_enable()	digitalWrite (3, LOW)
#define gsm_disable()	digitalWrite (3, HIGH)

#define GPS_BUF_SIZE 500
#define GGA_NUM 15
#define RMC_NUM 14

//$GPGGA
char *gga_table[GGA_NUM] = {
	"Message ID", 			//0
	"UTC Time", 			//1
	"Latitude", 			//2
	"N/S Indicator",		//3
	"Longitude", 			//4
	"E/W Indicator", 		//5
	"Position Fix Indicator", 	//6
	"Satellites Used", 		//7
	"HDOP", 			//8
	"MSL Altitude", 		//9
	"Units(M)", 			//10
	"Geoid Separation", 		//11
	"Units",  			//12
	"Diff.Ref.Station ID", 		//13
	"Checksum",			//14
};

//$GPRMC
char *gprmc_table[RMC_NUM] = {
	"Message ID",			//0
	"UTC Time",                     //1
	"Status",                       //2
	"Latitude",                     //3
	"N/S Indicator",                //4
	"Langitude",                    //5
	"E/W Indicator",                //6
	"Speed Over Ground",            //7
	"Course Over Ground",           //8
	"Date",                         //9
	"Magnetic Variation",           //10
	"East/West Indicator",          //11
	"Mode",                         //12
	"Checksum",                     //13
};


//save data from GPS
uint8_t gps_buf[GPS_BUF_SIZE];

//save pointer of gga block
uint8_t* gga_p[GGA_NUM];
uint8_t* gprmc_p[RMC_NUM];

// check sum using xor
uint8_t checksum_xor (uint8_t *array, uint8_t leng) {
	uint8_t sum = array[0];
	for (uint8_t i=1; i<leng; i++) {
		sum ^= array[i];
	}
	return sum;
}


//power and start gps
void start_gps () {
	digitalWrite (5, HIGH);
	delay (1500);
	digitalWrite (5, LOW);
	delay (1500);

	gsm_enable ();
	gps_disable ();

	delay (2000);
	#ifdef DEBUG
	Serial.println ("waiting for GPS! ");
	#endif
	
	Serial1.println ("AT");
	#ifdef DEBUG
	Serial.println ("Send AT");
	#endif
	delay (1000);
	Serial1.println ("AT+CGPSPWR=1");
	#ifdef DEBUG
	Serial.println ("Send AT+CGPSPWR=1");
	#endif
	delay (1000);
	Serial1.println ("AT+CGPSRST=1");
	#ifdef DEBUG
	Serial.println ("Send AT+CGPSRST=1");
	#endif
	delay (1000);

	gsm_disable ();
	gps_enable ();

	delay (2000);
	#ifdef DEBUG
	Serial.println ("$GPGGA statement information: ");
	#endif
}

// read data to gps_buf[] from GPS
static int gps_read () {
	uint32_t start_time = millis ();
	while (!Serial1.available ()) {
		if (millis() - start_time > 1500) {
			#ifdef DEBUG
			Serial.println ("restart GPS......");
			#endif
			start_gps ();
		}
	}
	for (int i=0; i<GPS_BUF_SIZE; i++) {
		delay (7);
		if (Serial1.available ()) {
			gps_buf [i] = Serial1.read ();
		} else {
			#ifdef DEBUG
			Serial.print ("read ");
			Serial.print (i);
			Serial.println (" character");
			#endif
			return 1;
		}
	}
	#ifdef DEBUG
	Serial.println ("error! data is so big!");
	#endif
	return 0;
}

//test head of gps_buf[] if is "$GPGGA" or not
static int is_GPGGA () {
	char gga_id[7] = "$GPGGA";
	for (int i=0; i<6; i++)
		if (gga_id[i] != gps_buf[i])
			return 0;
	return 1;
}

//get gpgga length
uint8_t get_gga_leng () {
	uint8_t l;
	for (l=0; l<GPS_BUF_SIZE && gps_buf[l] != 0x0d ; l++);
	return l;
}

// build gga_p[] by gps_buf
static void build_gga_p () {
	int p,b;
	for (p=b=0; p<GGA_NUM && b<GPS_BUF_SIZE; p++,b++) {
		gga_p[p] = (gps_buf+b);//
		if (gps_buf[b] == ',') 
			continue;
		for (b++; b<GPS_BUF_SIZE && gps_buf[b]!=','; b++);
	}
}

//test if fix
int gps_gga_is_fix (void) {
	if (gga_p[6][0] == '1')
		return 1;
	else 
		return 0;
}

//get gga checksum
static uint8_t gps_gga_checksum () {
	uint8_t sum = 0;
	if (gga_p[14][0] != '*')
		return 0;
	if (gga_p[14][2] >= '0' && gga_p[14][2] <= '9')
		sum = gga_p[14][2] - '0';
	else 
		sum = gga_p[14][2] - 'A' + 10;
	if (gga_p[14][1] >= '0' && gga_p[14][1] <= '9')
		sum += (gga_p[14][1] - '0') * 16;
	else 
		sum += (gga_p[14][1] - 'A' + 10) * 16;
	return sum;
}

//check sum of gga
static int checksum_gga () {
	uint8_t sum = checksum_xor (gps_buf+1, get_gga_leng ()-4);
	return sum - gps_gga_checksum ();
}

// set gga, change ',' to '\0'
static void gps_gga_set_str () {
	int i;
	for (i=0; gps_buf[i] != 0x0d && i<GPS_BUF_SIZE; i++)
		if (gps_buf[i] == ',')
			gps_buf[i] = '\0';
	//gps_buf[i] = '\0';
}

//must do this to get data, and return state 
int gps_get_gga (void) {
	int stat = 0;
	if (gps_read ()) {//read data from GPS
		if (is_GPGGA ()) {
			build_gga_p ();	// build *gga_p[] by gps_buf
			gps_gga_set_str ();
			if (checksum_gga () == 0)
				stat = 0;//gpgga data checksum is ok
			else 
				stat = 1;//checksum error
		} else 
			stat = 2;//not "$GPGGA", id error
	} else 
		stat = 3;//data is bad

	return stat;
}


//get UTC second
uint8_t gps_gga_utc_ss () {
	return (gga_p[1][4]-'0')*10 + gga_p[1][5]-'0';
}

//get UTC minute
uint8_t gps_gga_utc_mm () {
	return (gga_p[1][2]-'0')*10+gga_p[1][3]-'0';
}

//get UTC hour
uint8_t gps_gga_utc_hh () {
	return (gga_p[1][0]-'0')*10+gga_p[1][1]-'0';
}

//return UTC Time string, hhmmss
char* gps_gga_utc_s () {
	return (char*)gga_p[1];
}

//get latitude
double gps_gga_lat () {
	return atof ((char*)gga_p[2]);
}

//get latitude
char* gps_gga_lat_s () {
	return (char*)gga_p[2];
}

//get longitude
double gps_gga_long () {
	return atof ((char*)gga_p[4]);
}

//get longitude
char* gps_gga_long_s () {
	return (char*)gga_p[4];
}

//get HDOP
double gps_gga_HDOP () {
	return atof ((char*)gga_p[8]);
}

//get HDOP
char* gps_gga_HDOP_s () {
	return (char*)gga_p[8];
}

//get N/S
char gps_gga_NS () {
	if (gga_p[3][0] == '\0')
		return '0';
	else if (gga_p[3][0] == 'N' || gga_p[3][0] == 'S')
		return (char)gga_p[3][0];
	else 
		return '?';
}

//get N/S
char* gps_gga_NS_s () {
	if (gga_p[3][0] == '\0')
		return "0";
	else if (gga_p[3][0] == 'N' || gga_p[3][0] == 'S')
		return (char*)gga_p[3];
	else 
		return "?";
}

//get E/W
char gps_gga_EW () {
	if (gga_p[5][0] == '\0')
		return '0';
	else if (gga_p[5][0] == 'E' || gga_p[5][0] == 'W')
		return (char)gga_p[5][0];
	else 
		return '?';
}

//get E/W
char* gps_gga_EW_s () {
	if (gga_p[5][0] == '\0')
		return "0";
	else if (gga_p[5][0] == 'E' || gga_p[5][0] == 'W')
		return (char*)gga_p[5];
	else 
		return "?";
}

//
double gps_gga_MSL () {
	return atof ((char*)gga_p[9]);
}

//
char* gps_gga_MSL_s () {
	return (char*)gga_p[9];
}

//get gpggpa Geoid Separation
double gps_gga_geoid_sep () {
	return atof ((char*)gga_p[11]);
}

//get gpggpa Geoid Separation
char* gps_gga_geoid_sep_s () {
	return (char*)gga_p[11];
}

#ifdef DEBUG
//
void gps_gga_print () {
	for (int i=0; i<GPS_BUF_SIZE && gps_buf[i]!=0xd; i++)
		Serial.print ((char)gps_buf[i]);
	Serial.println ();
}
#endif

/*
void send_msg_with_num (char* numble, char*string) {
		char num_buf[25];
		sprintf (num_buf, "AT+CMGS=\"%s\"", numble);
		gsm_enable ();
		gps_disable ();
		delay (2000);
		Serial1.println ("AT");
		delay (200);
		Serial1.println ("AT");
		delay (200);
		Serial1.println ("AT+CMGF=1");
		delay (200);
		Serial1.println (num_buf);
		delay (200);
		Serial1.println (string);
		Serial1.write (26);
}
*/

//set mobile numble, begin send message
void gsm_begin_msg (char *numble) {
	char num_buf[25];
	sprintf (num_buf, "AT+CMGS=\"%s\"", numble);
	gsm_enable ();
	gps_disable ();
	delay (2000);
	Serial1.println ("AT");
	delay (200);
	Serial1.println ("AT");
	delay (200);
	Serial1.println ("AT+CMGF=1");
	delay (200);
	Serial1.println (num_buf);
	delay (200);
}

//send message to mobile 
void gsm_send_msg (char *message) {
	Serial1.print (message);
}

//send message to mobile 
void gsm_send_msgln (char *message) {
	Serial1.println (message);
}

//stop send message and start gps
void gsm_stop_send () {
	Serial1.write (26);
	delay (200);
	gsm_disable ();
	gps_enable ();
	delay (2000);
}


//gps and gsm init
void gps_init () {
	pinMode (3, OUTPUT);
	pinMode (4, OUTPUT);
	pinMode (5, OUTPUT);

}

/********************* end of file *************************/
