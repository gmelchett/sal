
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CPUSMOOTHNESS 5


/* See: Documentation/filesystems/proc.txt */
#define IDLE 3

static int cpu_average_list[CPUSMOOTHNESS];
static int current_load = 0;

void cpuload_update(void)
{

    static long long int oload = 0, ototal = 0;
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

    if (n != 9)
            return;

    for (i = 0; i < 9; i++)
            total += a[i];

    load = total - a[IDLE];

    if(total != ototal)
	current_load = (100 * (load - oload)) / (total - ototal);
    else
	current_load = (load - oload);

    oload = load;
    ototal = total;
}

/* returns current CPU load in percent, 0 to 100 */
int cpuload(void)
{
    static int firsttimes = 0, current = 0;
    unsigned int load = 0;
    int i;

    /* Wait until we have CPUSMOOTHNESS measures */
    if (firsttimes != CPUSMOOTHNESS)
	firsttimes++;

    cpu_average_list[current] = current_load;
    /* Calculates and average from the last CPUSMOOTHNESS messures */

    current++;
    if (current == CPUSMOOTHNESS)
	current = 0;

    if (firsttimes != CPUSMOOTHNESS)
	return 0;

    for (i = 0; i < CPUSMOOTHNESS; i++)
	load += cpu_average_list[i];

    return (load / CPUSMOOTHNESS);
}

void cpuload_init(void)
{
        memset(cpu_average_list, 0, sizeof(int) * CPUSMOOTHNESS);
}
