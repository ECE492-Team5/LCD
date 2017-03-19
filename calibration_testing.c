#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
// #include <error.h>
#include <stdlib.h>
#include <sys/mman.h>

//testing
#include <time.h>

// Information from Terasic's DE0-Nano-SoC_My_First_HPS-Fpga manual on the 
// DE0-Nano-SoC_v1.1.0_SystemCD:
//     Make sure the Altera Provided SoC EDS headers are included during build.
//     These headers are found in Quartus' Installation folder
//     /opt/altera/14.0/embedded/ip/altera/hps/altera_hps/hwlib/include
// #include "socal/socal.h"
// #include "socal/hps.h"
// #include "socal/alt_gpio.h"

// The hps_0 header file created with sopc-create-header-file utility.
// #include "hps_0.h"

// #define HW_REGS_BASE ( ALT_STM_OFST )
// #define HW_REGS_SPAN ( 0x04000000 )
// #define HW_REGS_MASK ( HW_REGS_SPAN - 1 )
#define SAMPLE_SIZE 10

// int set_current_range(int *min, int *max, char**argv);
double get_avg_sensor_value(double *value);

int main(int argc, char**argv) {
	//testing


	void *base;
    uint32_t *adc_base;
    int memdevice_fd;
    int i;
    double *value;
    const int nReadNum = 10;

    int channel = 0x00 & 0x07;
    
    if (argc == 2) {
    	// invalid input.
    } else if (argc == 3) {
    	// min/max
    	// configure all channels
    	// set_current_range(&min, &max, argv);
    } else if (argc == 4) {
    	// min/max/sensor#
    	// set_current_range(&min, &max, argv);
        // channel = ((uint8_t) atoi(argv[3])) & 0x07;
    } else {
    	// printf("Incorrect number of inputs.\n");
    	// exit(1);
    }






/*
    // Open /dev/mem device
    if( (memdevice_fd = open("/dev/mem", (O_RDWR | O_SYNC))) < 0) {
        perror("Unable to open \"/dev/mem\".");
        exit(EXIT_FAILURE);
    }

    // mmap the HPS registers
    base = (uint32_t*) mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, memdevice_fd, HW_REGS_BASE); 
    if(base == MAP_FAILED) {
        perror("mmap() failed.");
        close(memdevice_fd);
        exit(EXIT_FAILURE);
    }

    // derive leds base address from base HPS registers
    adc_base = (uint32_t*) (base + ((ALT_LWFPGASLVS_OFST + ADC_LTC2308_0_BASE) & HW_REGS_MASK));

    printf("ADC BASE ADDR = 0x%x\n", adc_base);

    // IOWR(adc_base, 0x01, nReadNum);
    *(adc_base + 0x01) = nReadNum;

    //IOWR(adc_base, 0x00, (channel << 1) | 0x00);
    //IOWR(adc_base, 0x00, (channel << 1) | 0x01);
    //IOWR(adc_base, 0x00, (channel << 1) | 0x00);
    
    *adc_base = (channel << 1) | 0x00;
    *adc_base = (channel << 1) | 0x01;
    *adc_base = (channel << 1) | 0x00;

    printf("wrote: 0x%04x", ((channel << 1) | 0x01));
    
    usleep(1);

    //while( (IORD(adc_base, 0x00) & 0x01) == 0x00 );
    while ((*adc_base & 0x01) == 0x00);
*/

    printf ("Turn off the current to the sensors. Press ANY KEY to continue. ");   
    getchar();
    
    // printf("CH%d = %.3fV (0x%04x) \n", channel, (float)value/1000.0, value);
    printf("%.3fV\n", get_avg_sensor_value(&value));
   	// printf( "You entered: %d \n", min);
   	// printf( "You entered: %d \n", max);

/*
    // unmap and close /dev/mem 
    if (munmap(base, HW_REGS_SPAN) < 0) {
        perror("munmap() failed.");
        close(memdevice_fd);
        exit(EXIT_FAILURE);
    }
*/
    close(memdevice_fd);
    exit(EXIT_SUCCESS);
}

// int set_current_range(int *min, int *max, char**argv) {
// 	*min = atoi(argv[1]);
// 	*max = atoi(argv[2]);
// 	if (*min > *max) {
//     	fprintf(stderr, "max is smaller than min current.\n");
//     	exit(0);
// 	}
// 	return 0;
// }
// 
double get_avg_sensor_value(double *value) {
    srand(time(NULL)); 
    int i;
    *value = 0;
    for (i = 0; i < SAMPLE_SIZE; ++i) {
        //testing
        *value += rand()/100000;    
    }
    *value /= SAMPLE_SIZE;
    return 0;
}
