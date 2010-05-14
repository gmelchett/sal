
#include <stdio.h>
#include <stdlib.h>

#define CPUSMOOTHNESS 30
/* returns current CPU load in percent, 0 to 100 */

int cpuload(void)
{
    static int firsttimes = 0, current = 0;
    static int cpu_average_list[CPUSMOOTHNESS];
    static u_int64_t oload = 0, ototal = 0;

    unsigned int cpuload;
    u_int64_t load, total;
    u_int64_t ab, ac, ad, ae;
    FILE *stat;
    int i;

    stat = fopen("/proc/stat", "r");
    fscanf(stat, "%*s %Ld %Ld %Ld %Ld", &ab, &ac, &ad, &ae);
    fclose(stat);

    if (firsttimes == 0) {
	for (i = 0; i < CPUSMOOTHNESS; i++)
	    cpu_average_list[i] = 0;
    }
    /* Wait until we have CPUSMOOTHNESS messures */
    if (firsttimes != CPUSMOOTHNESS)
	firsttimes++;

    /* Find out the CPU load */
    /* user + sys = load
     * total = total */
    load = ab + ac + ad;	/* cpu.user + cpu.sys; */
    total = ab + ac + ad + ae;	/* cpu.total; */

    /* Calculates and average from the last CPUSMOOTHNESS messures */
    if(total != ototal)
	cpu_average_list[current] = (100 * (load - oload)) / (total - ototal);
    else
	cpu_average_list[current] = (load - oload);

    current++;
    if (current == CPUSMOOTHNESS)
	current = 0;

    oload = load;
    ototal = total;

    if (firsttimes != CPUSMOOTHNESS)
	return 0;

    cpuload = 0;

    for (i = 0; i < CPUSMOOTHNESS; i++)
	cpuload += cpu_average_list[i];

    return (cpuload / CPUSMOOTHNESS);
}
