
#include <stdio.h>
#include <stdlib.h>

#define CPUSMOOTHNESS 30
/* returns current CPU load in percent, 0 to 100 */

/* See: Documentation/filesystems/proc.txt */
#define IDLE 3
int cpuload(void)
{
    static int firsttimes = 0, current = 0;
    static int cpu_average_list[CPUSMOOTHNESS];
    static long long int oload = 0, ototal = 0;

    unsigned int cpuload;
    long long int load, total = 0;
    long long int a[9];
    FILE *stat;
    int i;
    int n;

    stat = fopen("/proc/stat", "r");
    n = fscanf(stat, "%*s %Ld %Ld %Ld %Ld %Ld %Ld %Ld %Ld %Ld",
               &a[0], &a[1], &a[2], &a[3], &a[4],
               &a[5], &a[6], &a[7], &a[8]);
    fclose(stat);

    if (n != 9) {

            return 0;
    }

    if (firsttimes == 0) {
	for (i = 0; i < CPUSMOOTHNESS; i++)
	    cpu_average_list[i] = 0;
    }
    /* Wait until we have CPUSMOOTHNESS measures */
    if (firsttimes != CPUSMOOTHNESS)
	firsttimes++;
    for (i = 0; i < 9; i++)
            total += a[i];

    load = total - a[IDLE];

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
