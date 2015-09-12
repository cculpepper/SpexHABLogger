#ifndef GPS_H
#define GPS_H
#define GPSSTRINGLEN 15
typedef struct{
	char time[GPSSTRINGLEN];
	char stat;
	char lat[GPSSTRINGLEN];
	char latHemi;
	char lon[GPSSTRINGLEN];
	char lonHemi;
	char speed[GPSSTRINGLEN];
	char course[GPSSTRINGLEN];
	char date[GPSSTRINGLEN];
	char alt[GPSSTRINGLEN];
	char altUnit;
}GPSData;
char initGps();

char updatePos();

extern GPSData gpsData;

void ParseGPS(char c);


int testParse();
signed int getAlt();





#endif
		
