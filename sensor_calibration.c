#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "cjson/cJSON.h"
#include "cjson/cJSON.c"

// Information from Terasic's DE0-Nano-SoC_My_First_HPS-Fpga manual on the 
// DE0-Nano-SoC_v1.1.0_SystemCD:
//     Make sure the Altera Provided SoC EDS headers are included during build.
//     These headers are found in Quartus' Installation folder
//     /opt/altera/14.0/embedded/ip/altera/hps/altera_hps/hwlib/include
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"

// The hps_0 header file created with sopc-create-header-file utility.
#include "hps_0.h"

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

#define SAMPLE_SIZE 10
#define FILE_CHAR_SIZE 2000

#define MIN_VOLTS "min_avg_voltage"
#define MAX_VOLTS "max_avg_voltage"
#define MIN_AMPS "min_amperage"
#define MAX_AMPS "max_amperage"
#define MULTIPLIER "multiplier"

void calibrate_adc_base(uint32_t *adc_base, int channel);
void get_avg_sensor_value(double *value, uint32_t *adc_base);
char *readFile();
void writeFile(char *str);
void init_sensor_json(cJSON **root, char *name);

// cJSON library link: https://github.com/DaveGamble/cJSON/tree/master
int main(int argc, char**argv) {
	void *base;
    uint32_t *adc_base;
    int memdevice_fd;
    char sensor_name[20];
    double voltage_off, voltage_on, max, multiplier, some_value;

    int channel = 0x00 & 0x07;
    
    // Initialize the sensor.
    if (argc == 2) {
        sprintf(sensor_name, "current_sensor_%i", atoi(argv[1]));
        channel = ((uint8_t) atoi(argv[1])) & 0x07;
    } else {
    	printf("Incorrect number of inputs.\n");
    	exit(1);
    }

    // Read file and initialize the json object.
    char *str = readFile();
    cJSON *root = cJSON_Parse(str);
    cJSON *sensor = cJSON_GetObjectItem(root, sensor_name);

    // Initialize any sensors not found in json file.
    if (sensor == NULL) {
        init_sensor_json(&root, sensor_name);
        sensor = cJSON_GetObjectItem(root, sensor_name);
    }

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
    adc_base = (uint32_t*) (base + ((ALT_LWFPGASLVS_OFST + ADC_LTC2308_0_BASE) & HW_REGS_MASK));


    // calibrate when the current is off.
    printf ("Turn off the sensors. Press ANY KEY to continue. ");  
    getchar();
    cJSON_GetObjectItem(sensor, MIN_AMPS)->valuedouble = 0.00;

    calibrate_adc_base(adc_base, channel);
    get_avg_sensor_value(&voltage_off, adc_base);
    cJSON_GetObjectItem(sensor, MAX_VOLTS)->valuedouble = voltage_off;
    printf("Average voltage for OFF sensors: %.2fmV\n", voltage_off);
    printf("\n");

    // calibrate when the current is on.
    printf ("Turn on the sensors. Enter max expected amperage (mA): ");   
    scanf("%lf", &max);
    cJSON_GetObjectItem(sensor, MAX_AMPS)->valuedouble = max;

    calibrate_adc_base(adc_base, channel);
    get_avg_sensor_value(&voltage_on, adc_base);
    cJSON_GetObjectItem(sensor, MIN_VOLTS)->valuedouble = voltage_on;
    printf("Average voltage for ON sensors: %.2fV\n", voltage_on);
    printf("You entered %.2fmA as max.\n", max);
    printf("\n");

    // calibrate the multiplier
    multiplier = max/fabs(voltage_off-voltage_on);
    cJSON_GetObjectItem(sensor, MULTIPLIER)->valuedouble = multiplier;

    // unmap and close /dev/mem
    if (munmap(base, HW_REGS_SPAN) < 0) {
        perror("munmap() failed.");
        close(memdevice_fd);
        exit(EXIT_FAILURE);
    }

    // write back to file and free memory
    char *updated = cJSON_Print(root);
    writeFile(updated);
    cJSON_Delete(root);

    close(memdevice_fd);
    exit(EXIT_SUCCESS);
}

void calibrate_adc_base(uint32_t *adc_base, int channel) {
    
    printf("ADC BASE ADDR = 0x%p\n", adc_base);
    *(adc_base + 0x01) = SAMPLE_SIZE;
    
    *adc_base = (channel << 1) | 0x00;
    *adc_base = (channel << 1) | 0x01;
    *adc_base = (channel << 1) | 0x00;

    printf("wrote: 0x%04x", ((channel << 1) | 0x01));
    
    usleep(1);

    //while( (IORD(adc_base, 0x00) & 0x01) == 0x00 );
    while ((*adc_base & 0x01) == 0x00);
}

void get_avg_sensor_value(double *value, uint32_t *adc_base) {
    int i;
    *value = 0;
    for (i = 0; i < SAMPLE_SIZE; ++i) {
        *value += *(adc_base + 0x01);
    }
    *value /= SAMPLE_SIZE;
}

// Function from a save file.
// Modified for our program
// credit: http://stackoverflow.com/questions/4823177/reading-a-file-character-by-character-in-c
char *readFile() {
    FILE *file = fopen("sensor_data_json.txt", "r");
    char *code;
    size_t n = 0;
    int c;

    if (file == NULL)
        exit(1);

    code = malloc(FILE_CHAR_SIZE);

    while ((c = fgetc(file)) != EOF) {
        code[n++] = (char) c;
    }

    code[n] = '\0';        
    return code;
}

// Function rewrites the file with passed in json string.
void writeFile(char *str) {
    FILE *file = fopen("/var/tmp/sensor-config/sensor_data_json.txt", "w");
    if (file != NULL) {
        fprintf(file, "%s", str);
        fclose(file);
    }
}

// Function to initializes the json for a specific sensor
void init_sensor_json(cJSON **root, char name[20]) {
    cJSON *sensor;
    cJSON_AddItemToObject(*root, name, sensor = cJSON_CreateObject());

    cJSON_AddNumberToObject(sensor, MIN_VOLTS, 0.00);
    cJSON_AddNumberToObject(sensor, MAX_VOLTS, 0.00);
    cJSON_AddNumberToObject(sensor, MIN_AMPS, 0.00);
    cJSON_AddNumberToObject(sensor, MAX_AMPS, 0.00);
    cJSON_AddNumberToObject(sensor, MULTIPLIER, 0.00);

    writeFile(cJSON_Print(*root));
}
