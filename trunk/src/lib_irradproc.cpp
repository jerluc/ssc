
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "lib_irradproc.h"

#ifndef M_PI
#define M_PI 3.14159265358979323
#endif

#ifndef DTOR
#define DTOR 0.0174532925
#endif


static const int __nday[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

static int julian(int yr,int month,int day)    /* Calculates julian day of year */
{
	int i=1,jday=0,k;

	if( yr%4 == 0 )                      /* For leap years */
		k = 1;
	else
		k = 0;

	while( i < month )
		{
		jday = jday + __nday[i-1];
		i++;
		}
	if( month > 2 )
		jday = jday + k + day;
	else
		jday = jday + day;
	return(jday);
}

static int day_of_year( int month, int day_of_month ) /* returns 1-365 */
{
	int i=1,iday=0;

	while ( i < month )
		iday += __nday[i++ - 1];

	return iday + day_of_month;
}

int solarpos2(double ts, int year,int month,int day,int hour,double minute,double lat,double lng,double tz,double sunn[9])
{
	/* calculates effective solar position, correcting for proper 
		midpoints given the timestep (ts) at sunrise and sunset hours
		returns 1 if sun is up, 0 otherwise

		taken from pvwatts C implementation.
		apd: 25aug2011
		*/

	return 0;
}

void solarpos(int year,int month,int day,int hour,double minute,double lat,double lng,double tz,double sunn[9])
{
/* This function is based on a paper by Michalsky published in Solar Energy
	Vol. 40, No. 3, pp. 227-235, 1988. It calculates solar position for the
	time and location passed to the function based on the Astronomical
	Almanac's Algorithm for the period 1950-2050. For data averaged over an
	interval, the appropriate time passed is the midpoint of the interval.
	(Example: For hourly data averaged from 10 to 11, the time passed to the
	function should be 10 hours and 30 minutes). The exception is when the time
	interval includes a sunrise or sunset. For these intervals, the appropriate
	time should be the midpoint of the portion of the interval when the sun is
	above the horizon. (Example: For hourly data averaged from 7 to 8 with a
	sunrise time of 7:30, the time passed to the function should be 7 hours and
	and 45 minutes).

	Revised 5/15/98. Replaced algorithm for solar azimuth with one by Iqbal
	so latitudes below the equator are correctly handled. Also put in checks
	to allow an elevation of 90 degrees without crashing the program and prevented
	elevation from exceeding 90 degrees after refraction correction.

	This function calls the function julian to get the julian day of year.

	List of Parameters Passed to Function:
	year   = year (e.g. 1986)
	month  = month of year (e.g. 1=Jan)
	day    = day of month
	hour   = hour of day, local standard time, (1-24, or 0-23)
	minute = minutes past the hour, local standard time
	lat    = latitude in degrees, north positive
	lng    = longitude in degrees, east positive
	tz     = time zone, west longitudes negative

	sunn[]  = array of elements to return sun parameters to calling function
	sunn[0] = azm = sun azimuth in radians, measured east from north, 0 to 2*pi
	sunn[1] = 0.5*pi - elv = sun zenith in radians, 0 to pi
	sunn[2] = elv = sun elevation in radians, -pi/2 to pi/2
	sunn[3] = dec = sun declination in radians
	sunn[4] = sunrise in local standard time (hrs), not corrected for refraction
	sunn[5] = sunset in local standard time (hrs), not corrected for refraction
	sunn[6] = Eo = eccentricity correction factor
	sunn[7] = tst = true solar time (hrs)               
	sunn[8] = hextra = extraterrestrial solar irradiance on horizontal at particular time (W/m2)  */

	int jday,delta,leap;                           /* Local variables */
	double zulu,jd,time,mnlong,mnanom,
			eclong,oblqec,num,den,ra,dec,gmst,lmst,ha,elv,azm,refrac,
			E,ws,sunrise,sunset,Eo,tst;
	double arg,hextra,Gon,zen;

	jday = julian(year,month,day);       /* Get julian day of year */
	zulu = hour + minute/60.0 - tz;      /* Convert local time to zulu time */
	if( zulu < 0.0 )                     /* Force time between 0-24 hrs */
		{                                 /* Adjust julian day if needed */
		zulu = zulu + 24.0;
		jday = jday - 1;
		}
	else if( zulu > 24.0 )
		{
		zulu = zulu - 24.0;
		jday = jday + 1;
		}
	delta = year - 1949;
	leap = delta/4;
	jd = 32916.5 + delta*365 + leap + jday + zulu/24.0;
	time = jd - 51545.0;     /* Time in days referenced from noon 1 Jan 2000 */

	mnlong = 280.46 + 0.9856474*time;
	mnlong = fmod((double)mnlong,360.0);         /* Finds doubleing point remainder */
	if( mnlong < 0.0 )
		mnlong = mnlong + 360.0;          /* Mean longitude between 0-360 deg */

	mnanom = 357.528 + 0.9856003*time;
	mnanom = fmod((double)mnanom,360.0);
	if( mnanom < 0.0 )
		mnanom = mnanom + 360.0;
	mnanom = mnanom*DTOR;             /* Mean anomaly between 0-2pi radians */

	eclong = mnlong + 1.915*sin(mnanom) + 0.020*sin(2.0*mnanom);
	eclong = fmod((double)eclong,360.0);
	if( eclong < 0.0 )
		eclong = eclong + 360.0;
	eclong = eclong*DTOR;       /* Ecliptic longitude between 0-2pi radians */

	oblqec = ( 23.439 - 0.0000004*time )*DTOR;   /* Obliquity of ecliptic in radians */
	num = cos(oblqec)*sin(eclong);
	den = cos(eclong);
	ra  = atan(num/den);                         /* Right ascension in radians */
	if( den < 0.0 )
		ra = ra + M_PI;
	else if( num < 0.0 )
		ra = ra + 2.0*M_PI;

	dec = asin( sin(oblqec)*sin(eclong) );       /* Declination in radians */

	gmst = 6.697375 + 0.0657098242*time + zulu;
	gmst = fmod((double)gmst,24.0);
	if( gmst < 0.0 )
		gmst = gmst + 24.0;         /* Greenwich mean sidereal time in hours */

	lmst = gmst + lng/15.0;
	lmst = fmod((double)lmst,24.0);
	if( lmst < 0.0 )
		lmst = lmst + 24.0;
	lmst = lmst*15.0*DTOR;         /* Local mean sidereal time in radians */

	ha = lmst - ra;
	if( ha < -M_PI )
		ha = ha + 2*M_PI;
	else if( ha > M_PI )
		ha = ha - 2*M_PI;             /* Hour angle in radians between -pi and pi */

	lat = lat*DTOR;                /* Change latitude to radians */

	arg = sin(dec)*sin(lat) + cos(dec)*cos(lat)*cos(ha);  /* For elevation in radians */
	if( arg > 1.0 )
		elv = M_PI/2.0;
	else if( arg < -1.0 )
		elv = -M_PI/2.0;
	else
		elv = asin(arg);

	if( cos(elv) == 0.0 )
		{
		azm = M_PI;         /* Assign azimuth = 180 deg if elv = 90 or -90 */
		}
	else
		{                 /* For solar azimuth in radians per Iqbal */
		arg = ((sin(elv)*sin(lat)-sin(dec))/(cos(elv)*cos(lat))); /* for azimuth */
		if( arg > 1.0 )
			azm = 0.0;              /* Azimuth(radians)*/
		else if( arg < -1.0 )
			azm = M_PI;
		else
			azm = acos(arg);

		if( ( ha <= 0.0 && ha >= -M_PI) || ha >= M_PI )
			azm = M_PI - azm;
		else
			azm = M_PI + azm;
		}

	elv = elv/DTOR;          /* Change to degrees for atmospheric correction */
	if( elv > -0.56 )
		refrac = 3.51561*( 0.1594 + 0.0196*elv + 0.00002*elv*elv )/( 1.0 + 0.505*elv + 0.0845*elv*elv );
	else
		refrac = 0.56;
	if( elv + refrac > 90.0 )
		elv = 90.0*DTOR;
	else
		elv = ( elv + refrac )*DTOR ; /* Atmospheric corrected elevation(radians) */

	E = ( mnlong - ra/DTOR )/15.0;       /* Equation of time in hours */
	if( E < - 0.33 )   /* Adjust for error occuring if mnlong and ra are in quadrants I and IV */
		E = E + 24.0;
	else if( E > 0.33 )
		E = E - 24.0;

	arg = -tan(lat)*tan(dec);
	if( arg >= 1.0 )
		ws = 0.0;                         /* No sunrise, continuous nights */
	else if( arg <= -1.0 )
		ws = M_PI;                          /* No sunset, continuous days */
	else
		ws = acos(arg);                   /* Sunrise hour angle in radians */

					/* Sunrise and sunset in local standard time */
	sunrise = 12.0 - (ws/DTOR)/15.0 - (lng/15.0 - tz) - E;
	sunset  = 12.0 + (ws/DTOR)/15.0 - (lng/15.0 - tz) - E;

	Eo = 1.00014 - 0.01671*cos(mnanom) - 0.00014*cos(2.0*mnanom);  /* Earth-sun distance (AU) */
	Eo = 1.0/(Eo*Eo);                    /* Eccentricity correction factor */

	tst = hour + minute/60.0 + (lng/15.0 - tz) + E;  /* True solar time (hr) */
	
	/* 25aug2011 apd: addition of calculation of horizontal extraterrestrial irradiance */
	zen = 0.5*M_PI - elv;
	Gon = 1367*(1+0.033*cos( 360.0/365.0*day_of_year(month,day)*M_PI/180 )); /* D&B eq 1.4.1a, using solar constant=1367 W/m2 */
	if (zen > 0 && zen < M_PI/2) /* if sun is up */
		hextra = Gon*cos(zen); /* elevation is incidence angle (zen=90-elv) with horizontal */
	else if (zen == 0)
		hextra = Gon;
	else
		hextra = 0.0;

	sunn[0] = azm;                        /* Variables returned in array sunn[] */
	sunn[1] = zen;               /*  Zenith */
	sunn[2] = elv;
	sunn[3] = dec;
	sunn[4] = sunrise;
	sunn[5] = sunset;
	sunn[6] = Eo;
	sunn[7] = tst;
	sunn[8] = hextra;
}


void incidence(int mode,double tilt,double sazm,double rlim,double zen,double azm,double angle[3])
{
/* This function calculates the incident angle of direct beam radiation to a
	surface for a given sun position, latitude, and surface orientation. The
	modes available are fixed tilt, 1-axis tracking, and 2-axis tracking.
	Azimuth angles are for N=0 or 2pi, E=pi/2, S=pi, and W=3pi/2.  8/13/98

	List of Parameters Passed to Function:
	mode   = 0 for fixed-tilt, 1 for 1-axis tracking, 2 for 2-axis tracking
	tilt   = tilt angle of surface from horizontal in degrees (mode 0),
				or tilt angle of tracker axis from horizontal in degrees (mode 1),
				MUST BE FROM 0 to 90 degrees.
	sazm   = surface azimuth in degrees of collector (mode 0), or surface
				azimuth of tracker axis (mode 1) with axis azimuth directed from
				raised to lowered end of axis if axis tilted.
	rlim   = plus or minus rotation in degrees permitted by physical constraints
				of tracker, range is 0 to 180 degrees.
	zen    = sun zenith in radians, MUST BE LESS THAN PI/2
	azm    = sun azimuth in radians, measured east from north

	Parameters Returned:
	angle[]  = array of elements to return angles to calling function
	angle[0] = inc  = incident angle in radians
	angle[1] = tilt = tilt angle of surface from horizontal in radians
	angle[2] = sazm = surface azimuth in radians, measured east from north  */

	/* Local variables: rot is the angle that the collector is rotated about the
	axis when viewed from the raised end of the 1-axis tracker. If rotated
	counter clockwise the angle is negative. Range is -180 to +180 degrees.
	When xsazm = azm : rot = 0, tilt = xtilt, and sazm = xsazm = azm  */

	double arg,inc=0,xsazm,xtilt,rot;

	switch ( mode )
		{
		case 0:                 /* Fixed-Tilt */
			tilt = tilt*DTOR;    /* Change tilt and surface azimuth to radians */
			sazm = sazm*DTOR;
			arg = sin(zen)*cos(azm-sazm)*sin(tilt) + cos(zen)*cos(tilt);
			if( arg < -1.0 )
				inc = M_PI;
			else if( arg > 1.0  )
				inc = 0.0;
			else
				inc = acos(arg);
			break;
		case 1:                 /* One-Axis Tracking */
			xtilt = tilt*DTOR;   /* Change axis tilt, surface azimuth, and rotation limit to radians */
			xsazm = sazm*DTOR;
			rlim  = rlim*DTOR;
									/* Find rotation angle of axis for peak tracking */
			if( fabs( cos(xtilt) ) < 0.001745 )    /* 89.9 to 90.1 degrees */
				{          /* For vertical axis only */
				if( xsazm <= M_PI )
					{
					if( azm <= xsazm + M_PI )
						rot = azm - xsazm;
					else
						rot = azm - xsazm - 2.0*M_PI;
					}
				else        /* For xsazm > pi */
					{
					if( azm >= xsazm - M_PI )
						rot = azm - xsazm;
					else
						rot = azm - xsazm + 2.0*M_PI;
					}
				}
			else          /* For other than vertical axis */
				{
				arg = sin(zen)*sin(azm-xsazm)/
						( sin(zen)*cos(azm-xsazm)*sin(xtilt) + cos(zen)*cos(xtilt) );
				if( arg < -99999.9 )
					rot = -M_PI/2.0;
				else if( arg > 99999.9 )
					rot = M_PI/2.0;
				else
					rot = atan(arg);
								/* Put rot in II or III quadrant if needed */
				if( xsazm <= M_PI )
					{
					if( azm > xsazm && azm <= xsazm + M_PI )
						{     /* Ensure positive rotation */
						if( rot < 0.0 )
							rot = M_PI + rot;   /* Put in II quadrant: 90 to 180 deg */
						}
					else
						{     /* Ensure negative rotation  */
						if( rot > 0.0 )
							rot = rot - M_PI;   /* Put in III quadrant: -90 to -180 deg */
						}
					}
				else        /* For xsazm > pi */
					{
					if( azm < xsazm && azm >= xsazm - M_PI )
						{     /* Ensure negative rotation  */
						if( rot > 0.0 )
							rot = rot - M_PI;   /* Put in III quadrant: -90 to -180 deg */
						}
					else
						{     /* Ensure positive rotation */
						if( rot < 0.0 )
							rot = M_PI + rot;   /* Put in II quadrant: 90 to 180 deg */
						}
					}
				}
	  /*    printf("rot=%6.1f azm=%6.1f xsazm=%6.1f xtilt=%6.1f zen=%6.1f\n",rot/DTOR,azm/DTOR,xsazm/DTOR,xtilt/DTOR,zen/DTOR);  */

			if( rot < -rlim ) /* Do not let rotation exceed physical constraints */
				rot = -rlim;
			else if( rot > rlim )
				rot = rlim;

									/* Find tilt angle for the tracking surface */
			arg = cos(xtilt)*cos(rot);
			if( arg < -1.0 )
				tilt = M_PI;
			else if( arg > 1.0  )
				tilt = 0.0;
			else
				tilt = acos(arg);
									/* Find surface azimuth for the tracking surface */
			if( tilt == 0.0 )
				sazm = M_PI;     /* Assign any value if tilt is zero */
			else
				{
				arg = sin(rot)/sin(tilt);
				if( arg < -1.0 )
					sazm = 1.5*M_PI + xsazm;
				else if( arg > 1.0  )
					sazm = 0.5*M_PI + xsazm;
				else if( rot < -0.5*M_PI )
					sazm = xsazm - M_PI - asin(arg);
				else if( rot > 0.5*M_PI )
					sazm = xsazm + M_PI - asin(arg);
				else
					sazm = asin(arg) + xsazm;
				if( sazm > 2.0*M_PI )       /* Keep between 0 and 2pi */
					sazm = sazm - 2.0*M_PI;
				else if( sazm < 0.0 )
					sazm = sazm + 2.0*M_PI;
				}
		/* printf("zen=%6.1f azm-sazm=%6.1f tilt=%6.1f arg=%7.4f\n",zen/DTOR,(azm-sazm)/DTOR,tilt/DTOR,arg); */
									/* Find incident angle */
			arg = sin(zen)*cos(azm-sazm)*sin(tilt) + cos(zen)*cos(tilt);
			if( arg < -1.0 )
				inc = M_PI;
			else if( arg > 1.0  )
				inc = 0.0;
			else
				inc = acos(arg);
			break;
		case 2:                 /* Two-Axis Tracking */
			tilt = zen;
			sazm = azm;
			inc = 0.0;
			break;
		}
	angle[0] = inc;           /* Variables returned in array angle[] */
	angle[1] = tilt;
	angle[2] = sazm;

}

#define SMALL 1e-6

void hdkr( double hextra, double dn, double df, double alb, double inc, double tilt, double zen, double poa[3] )
{
/* added aug2011 by aron dobos. Defines Hay, Davies, Klutcher, Reindl model for diffuse irradiance on a tilted surface
	
	List of Parameters Passed to Function:
	hextra = extraterrestrial irradiance on horizontal surface (W/m2)
	dn     = direct normal radiation (W/m2)
	df     = diffuse horizontal radiation (W/m2)
	alb    = surface albedo (decimal fraction)
	inc    = incident angle of direct beam radiation to surface in radians
	tilt   = surface tilt angle from horizontal in radians
	zen    = sun zenith angle in radians

	Variable Returned
	poa    = plane-of-array irradiances (W/m2)
				poa[0]: incident beam
				poa[1]: incident sky diffuse
				poa[2]: incident ground diffuse */

	double hb = dn*cos(zen); /* beam irradiance on horizontal */
	double ht = hb+df; /* total irradiance on horizontal */
	if (ht < SMALL) ht = SMALL;
	if (hextra < SMALL) hextra = SMALL;

	double Rb = cos(inc)/cos(zen); /* ratio of beam on surface to beam on horizontal (D&B eq 1.8.1) */
	double Ai = hb/hextra; /* anisotropy index, term for forward scattering circumsolar diffuse (D&B eq 2.16.3) */
	double f = sqrt(hb/ht); /* modulating factor for horizontal brightening correction */
	double s3 = pow( sin( tilt*0.5 ), 3 ); /* horizontal brightening correction */

	poa[0] = dn*cos(inc);
	poa[1] = df*(0.5*(1.0-Ai)*(1.0+cos(tilt))*(1.0+f*s3) + Ai*Rb);
	poa[2] = (hb+df)*alb*(1.0-cos(tilt))/2.0;
}


void isotropic( double hextra, double dn, double df, double alb, double inc, double tilt, double zen, double poa[3] )
{
/* added aug2011 by aron dobos. Defines isotropic sky model for diffuse irradiance on a tilted surface
	
	List of Parameters Passed to Function:
	hextra = extraterrestrial irradiance on horizontal surface (W/m2) (unused for isotropic sky)
	dn     = direct normal radiation (W/m2)
	df     = diffuse horizontal radiation (W/m2)
	alb    = surface albedo (decimal fraction)
	inc    = incident angle of direct beam radiation to surface in radians
	tilt   = surface tilt angle from horizontal in radians
	zen    = sun zenith angle in radians

	Variable Returned
	poa    = plane-of-array irradiances (W/m2)
				poa[0]: incident beam
				poa[1]: incident sky diffuse
				poa[2]: incident ground diffuse */

	poa[0] = dn*cos(inc);
	poa[1] = df*(1.0+cos(tilt))/2.0;
	poa[2] = (dn*cos(zen)+df)*alb*(1.0-cos(tilt))/2.0;
}

void perez( double hextra, double dn, double df, double alb, double inc, double tilt, double zen, double poa[3] )
{
/* Modified aug2011 by aron dobos to split out beam, diffuse, ground for output.
	Total POA is poa[0]+poa[1]+poa[2]

   Defines the Perez function for calculating values of diffuse + direct
	solar radiation + ground reflected radiation for a tilted surface
	and returns the total plane-of-array irradiance(poa).  Function does
	not check all input for valid entries; consequently, this should be
	done before calling the function.  (Reference: Perez et al, Solar
	Energy Vol. 44, No.5, pp.271-289,1990.) Based on original FORTRAN
	program by Howard Bisner.

	Modified 6/10/98 so that for zenith angles between 87.5 and 90.0 degrees,
	the diffuse radiation is treated as isotropic instead of 0.0.

	List of Parameters Passed to Function:
	hextra = extraterrestrial irradiance on horizontal surface (W/m2) (unused in perez model)
	dn     = direct normal radiation (W/m2)
	df     = diffuse horizontal radiation (W/m2)
	alb    = surface albedo (decimal fraction)
	inc    = incident angle of direct beam radiation to surface in radians
	tilt   = surface tilt angle from horizontal in radians
	zen    = sun zenith angle in radians

	Variable Returned
	poa    = plane-of-array irradiances (W/m2)
				poa[0]: incident beam
				poa[1]: incident sky diffuse
				poa[2]: incident ground diffuse */

													/* Local variables */
	double F11R[8] = { -0.0083117, 0.1299457, 0.3296958, 0.5682053,
							 0.8730280, 1.1326077, 1.0601591, 0.6777470 };
	double F12R[8] = {  0.5877285, 0.6825954, 0.4868735, 0.1874525,
							-0.3920403, -1.2367284, -1.5999137, -0.3272588 };
	double F13R[8] = { -0.0620636, -0.1513752, -0.2210958, -0.2951290,
							-0.3616149, -0.4118494, -0.3589221, -0.2504286 };
	double F21R[8] = { -0.0596012, -0.0189325, 0.0554140, 0.1088631,
							 0.2255647, 0.2877813, 0.2642124, 0.1561313 };
	double F22R[8] = {  0.0721249, 0.0659650, -0.0639588, -0.1519229,
							-0.4620442, -0.8230357, -1.1272340, -1.3765031 };
	double F23R[8] = { -0.0220216, -0.0288748, -0.0260542, -0.0139754,
							 0.0012448, 0.0558651, 0.1310694, 0.2506212 };
	double EPSBINS[7] = { 1.065, 1.23, 1.5, 1.95, 2.8, 4.5, 6.2 };
	double B2=0.000005534,
		EPS,T,D,DELTA,A,B,C,ZH,F1,F2,COSINC,x;
	double CZ,ZC,ZENITH,AIRMASS;

	int i;

	if ( dn < 0.0 )           /* Negative values may be measured if cloudy */
		dn = 0.0;

	if ( zen < 0.0 || zen > 1.5271631 ) /* Zen not between 0 and 87.5 deg */
		{
		if( df < 0.0 )
			df = 0.0;
		if ( cos(inc) > 0.0 && zen < 1.5707963 )  /* Zen between 87.5 and 90 */
			{                                      /* and incident < 90 deg   */
			poa[0] = dn * cos(inc);
			poa[1] = df*( 1.0 + cos(tilt) )/2.0;
			poa[2] = 0.0;
			return;
			}
		else
			{
			poa[0] = 0;
			poa[1] = df*( 1.0 + cos(tilt) )/2.0;   /* Isotropic diffuse only */
			poa[2] = 0.0;
			return;
			}
		}
	else                      /* Zen between 0 and 87.5 deg */
		{
		CZ = cos(zen);
		ZH = ( CZ > 0.0871557 ) ? CZ:0.0871557;    /* Maximum of 85 deg */
		D = df;                /* Horizontal diffuse radiation */
		if ( D <= 0.0 )        /* Diffuse is zero or less      */
			{
			if ( cos(inc) > 0.0 )    /* Incident < 90 deg */
				{
				poa[0] = dn*cos(inc);
				poa[1] = 0.0;
				poa[2] = 0.0;
				return;
				}
			else
				{
				poa[0] = 0;
				poa[1] = 0;
				poa[2] = 0;
				return;
				}
			}
		else                   /* Diffuse is greater than zero */
			{
			ZENITH = zen/DTOR;
			AIRMASS = 1.0 / (CZ + 0.15 * pow(93.9 - ZENITH, -1.253) );
			DELTA = D * AIRMASS / 1367.0;
			T = pow(ZENITH,3.0);
			EPS = (dn + D) / D;
			EPS = (EPS + T*B2) / (1.0 + T*B2);
			i=0;
			while ( i < 7 && EPS > EPSBINS[i] )
				i++;
			x = F11R[i] + F12R[i]*DELTA + F13R[i]*zen;
			F1 = ( 0.0 > x ) ? 0.0:x;
			F2 = F21R[i] + F22R[i]*DELTA + F23R[i]*zen;
			COSINC = cos(inc);
			if( COSINC < 0.0 )
				ZC = 0.0;
			else
				ZC = COSINC;
			A = D*( 1.0 + cos(tilt) )/2.0; // isotropic diffuse
			B = ZC/ZH*D - A; // circumsolar 
			C = D*sin(tilt); // horizon brightness term
			
			// original PVWatts: poa = A + F1*B + F2*C + alb*(dn*CZ+D)*(1.0 - cos(tilt) )/2.0 + dn*ZC;
			poa[0] = dn*ZC;
			poa[1] = A + F1*B + F2*C;
			poa[2] = alb*(dn*CZ+D)*(1.0 - cos(tilt) )/2.0;
			return;
			}
		}
}


