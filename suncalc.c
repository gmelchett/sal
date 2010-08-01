

/*
 * Stolen from: http://www.nic.fi/~benefon/rscalc.c
 *
 * C program calculating the sunrise and sunset for
 * the current date and a fixed location(latitude,longitude)
 * Note, twilight calculation gives insufficient accuracy of results
 * Jarmo Lammi 1999 - 2001
 * Last update July 21st, 2001
 */


#include <math.h>

#include "suncalc.h"

#define DEGS (180.0 / M_PI)
#define RADS (M_PI / 180.0)
#define SUNDIA 0.53		/* Sunradius degrees */
#define AIRREFR (34.0 / 60.0)	/* athmospheric refraction degrees */

/*
 * Get the days to J2000
 * h is UT in decimal hours
 * FNday only works between 1901 to 2099 - see Meeus chapter 7
 */
static double FNday(int y, int m, int d, float h)
{
        long int luku = -7 * (y + (m + 9) / 12) / 4 + 275 * m / 9 + d;
        luku += (long int) y *367;
        return (double) luku - 730531.5 + h / 24.0;
}

static double FNrange(double x)
{
        double b = 0.5 * x / M_PI;
        double a = 2.0 * M_PI * (b - (long) (b));
        if (a < 0)
                a = 2.0 * M_PI + a;
        return a;
}

/* Calculating the hourangle */
static double f0(double lat, double declin)
{
        double fo, dfo;

        dfo = RADS * (0.5 * SUNDIA + AIRREFR);
        if (lat < 0.0)
                dfo = -dfo;
        fo = tan(declin + dfo) * tan(lat * RADS);
        if (fo > 0.99999)
                fo = 1.0;
        return asin(fo) + M_PI / 2.0;
};

/* Calculating the hourangle for twilight times */
static double f1(double lat, double declin)
{
        double fi, df1;

        df1 = RADS * 6.0;
        if (lat < 0.0)
                df1 = -df1;
        fi = tan(declin + df1) * tan(lat * RADS);
        if (fi > 0.99999)
                fi = 1.0;
        return asin(fi) + M_PI / 2.0;
}

void sun(struct sunsetrise *ssr, int year, int month, int day, int tzone, double latit, double longit)
{
        double L, g;
        double d, lambda;
        double obliq, LL, equation, ha, hb;
        double settm, riset;
        double daylen;

        d = FNday(year, month, day, 12);

        /* mean longitude of the Sun */
        L = FNrange(280.461 * RADS + .9856474 * RADS * d);

        /* mean anomaly of the Sun */
        g = FNrange(357.528 * RADS + .9856003 * RADS * d);

        /* Ecliptic longitude of the Sun */
        lambda = FNrange(L + 1.915 * RADS * sin(g) + .02 * RADS * sin(2 * g));

        /* Obliquity of the ecliptic */

        obliq = 23.439 * RADS - .0000004 * RADS * d;

        /* Find the RA and DEC of the Sun */

        /* Find the Equation of Time in minutes */
        /* Correction suggested by David Smith */
        LL = L - atan2(cos(obliq) * sin(lambda), cos(lambda));
        if (L < M_PI)
                LL += 2.0 * M_PI;
        equation = 1440.0 * (1.0 - LL / M_PI / 2.0);
        ha = f0(latit, asin(sin(obliq) * sin(lambda)));
        hb = f1(latit, asin(sin(obliq) * sin(lambda)));
        /* Conversion of angle to hours and minutes */
        daylen = DEGS * ha / 7.5;
        if (daylen < 0.0001)
                daylen = 0.0;

        /* arctic winter */
        riset = 12.0 - 12.0 * ha / M_PI + (double)tzone - longit / 15.0 + equation / 60.0;
        settm = 12.0 + 12.0 * ha / M_PI + (double)tzone - longit / 15.0 + equation / 60.0;

        if (riset > 24.0)
                riset -= 24.0;
        if (settm > 24.0)
                settm -= 24.0;

        ssr->sunrise_h = (int)riset;
        ssr->sunrise_m = (int)((riset - ssr->sunrise_h) * 60);
        ssr->sunset_h = (int)settm;
        ssr->sunset_m = (int)((settm - ssr->sunset_h) * 60);
        ssr->daylen_h = (int)daylen;
        ssr->daylen_m = (int)((daylen - ssr->daylen_h) * 60);

}
