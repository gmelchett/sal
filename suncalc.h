#ifndef SUNCALC_H
#define SUNCALC_H

struct sunsetrise
{
        int sunset_h;
        int sunset_m;
        int sunrise_h;
        int sunrise_m;
        int daylen_h;
        int daylen_m;
};

void sun(struct sunsetrise *ssr, int year, int month, int day, int tzone, double latit, double longit);

#endif
