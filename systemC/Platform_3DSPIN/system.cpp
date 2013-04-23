/**********************************************************************
 * File : system.cpp
 * Date : 01/09/2009 
 * Author : Hamed (Abbas) Sheibanyrad
 * This program is released under the GNU Public License
 * Copyright : TIMA
 **********************************************************************/

#define NUM_X 
#define NUM_Y 
#define NUM_Z			1

#define MIN_PER 		30
#define MAX_PER 		31

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "./Models/SystemC/Common/soclib_dspin_interfaces.h"
#include "./Models/SystemC/TG/soclib_TG.h"
#include "./Models/SystemC/3DSPIN/soclib_3dspin.h"

#define MAX_PACKET_LENGTH       16
#define MIN_PACKET_LENGTH       16	// the minimum length is 3

#define DSPIN_DATA_SIZE 	36
#define FIFO_SIZE 		8

#define DIFF_TIME(begin, end)       ((long long int)(end.tv_sec - begin.tv_sec)*1000000000 + (long long int)(end.tv_nsec - begin.tv_nsec))
#define COMMAND                     "echo \"     %6d   %2d      1       %12lld \" >> %s", \
                                                nb_proc, NUM_X*NUM_Y, \
                                            DIFF_TIME(begin,end), output_name


void next_cycle (sc_signal<bool> &signal_clk)
{
    signal_clk = false;
    sc_start (1);
    signal_clk = true;
    sc_start (1);
}

int sc_main (int argc, char *argv[])
{
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);

    int	ncycles;
    int nb_proc;
    sscanf(argv[1],"%d",&ncycles);
    sscanf(argv[2],"%d",&nb_proc);
    char *output_name = argv[3];

    struct timespec begin;
    struct timespec end;

    sc_signal<bool>		signal_clk("signal_clk");
    sc_signal<bool>		signal_resetn("signal_resetn");

    DSPIN_SIGNALS<DSPIN_DATA_SIZE> local_in[NUM_Z][NUM_Y][NUM_X];
    DSPIN_SIGNALS<DSPIN_DATA_SIZE> local_out[NUM_Z][NUM_Y][NUM_X];

    char str[20];

    SOCLIB_3DSPIN  <DSPIN_DATA_SIZE, FIFO_SIZE, NUM_Z, NUM_Y, NUM_X> dspin ("dspin");

    SOCLIB_PRODUCER   <DSPIN_DATA_SIZE, MAX_PACKET_LENGTH, MIN_PACKET_LENGTH, NUM_Z*NUM_Y*NUM_X>             *producer[NUM_Z][NUM_Y][NUM_X];


    dspin.CLK      (signal_clk);
    dspin.RESETN   (signal_resetn);


    int ADR [NUM_Z*NUM_Y*NUM_X];
    float PER [NUM_Z*NUM_Y*NUM_X];

    for (int z=0; z<NUM_Z ; z++)
        for (int y=0; y<NUM_Y ; y++)
            for (int x=0; x<NUM_X ; x++)
            {

                sprintf (str, "producer[%x][%x][%x]",z,y,x);
                producer[z][y][x] = new SOCLIB_PRODUCER <DSPIN_DATA_SIZE, MAX_PACKET_LENGTH, MIN_PACKET_LENGTH, NUM_Z*NUM_Y*NUM_X> (str, (z*NUM_Y*NUM_X)+(y*NUM_X)+x, ADR, 50);
                producer[z][y][x]->CLK                (signal_clk);
                producer[z][y][x]->RESETN             (signal_resetn);
                producer[z][y][x]->init_out           (local_out[z][y][x]);
                producer[z][y][x]->targ_in            (local_in[z][y][x]);

                dspin.OUT[z][y][x] (local_in[z][y][x]);
                dspin.IN[z][y][x] (local_out[z][y][x]);

                ADR [z*NUM_Y*NUM_X + y*NUM_X + x] = (z<<10) | (y<<5) | x;
                PER [z*NUM_Y*NUM_X + y*NUM_X + x] = 1.0/(NUM_Z*NUM_Y*NUM_X);

                producer[z][y][x]->unif();

                //	int dist [NUM_Z*NUM_Y*NUM_X];
                //	for (int zz=0; zz<NUM_Z ; zz++)
                //		for (int yy=0; yy<NUM_Y ; yy++)
                //		        for (int xx=0; xx<NUM_X ; xx++)
                //				dist [zz*NUM_Y*NUM_X + yy*NUM_X + xx] = abs(zz-z) + abs(yy-y) + abs(xx-x);
                //	producer[z][y][x]->localize(dist);


            }

    int y, x, i, z ;
    float per;
    float Aver;

    FILE *f;
    f = fopen ("res", "wt");

    for (per = MIN_PER; per < MAX_PER ; per+=1)
    {

        for (z=0; z<NUM_Z ; z++)
            for (y=0; y<NUM_Y ; y++)
                for (x=0; x<NUM_X ; x++)
                    producer[z][y][x]-> Percent = ((float)per/100) /* * PER[z*NUM_Y*NUM_X + y*NUM_X + x] * ((MAX_PACKET_LENGTH+MIN_PACKET_LENGTH)/2)*/;

        /* begin measure */
        clock_gettime(CLOCK_REALTIME, &(begin));
        
        signal_resetn = false;
        next_cycle (signal_clk);
        signal_resetn = true;
        next_cycle (signal_clk);

        for (i = 0; i < ncycles ; i++)
        {
            next_cycle (signal_clk);

            if((i % 1000) == 0) 
            {printf("%f Time elapsed : %d cycles.\n", per, i);} // end printf
        } // end simulation loop

        /* end measure */
        clock_gettime(CLOCK_REALTIME, &(end));
        /* output the measures */
        int size = snprintf(NULL, 0, COMMAND);
        char *command = (char*) malloc(sizeof(char) * (size+1));
        sprintf(command, COMMAND);

        system(command);
        free(command);

        Aver = 0;
        int Count = 0;

        for (z=0; z<NUM_Z ; z++)
            for (y=0; y<NUM_Y ; y++)
                for (x=0; x<NUM_X ; x++)
                {
                    if (producer[z][y][x]->A_COUNT)        
                    {
                        Aver += producer[z][y][x]->Latency;
                        Count += producer[z][y][x]->A_COUNT;
                    }
                }
        Aver /= Count;
        printf ("%f \n", Aver);

        fprintf (f,"%f : %f\n", (per/*/100*/)/**MAX_PACKET_LENGTH*/, Aver);
        fflush (f);
        //	cout << "average is :" << Aver << endl;
    }
    fclose (f);


    for (int z=0; z<NUM_Z ; z++)
        for (int y=0; y<NUM_Y ; y++)
            for (int x=0; x<NUM_X ; x++)
            {
                delete	producer[z][y][x];
            }

    return EXIT_SUCCESS;

};
