#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <stdlib.h>
#include <sys/mman.h>

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

#define lcd_write_cmd(base, data)                     IOWR(base, 0, data)
#define lcd_read_cmd(base)                            IORD(base, 1)
#define lcd_write_data(base, data)                    IOWR(base, 2, data)
#define lcd_read_data(base)                           IORD(base, 3) 

void LCD_Init()
{
    lcd_write_cmd(LCD_16207_0_BASE,0x38); 
    usleep(2000);
    lcd_write_cmd(LCD_16207_0_BASE,0x0C);
    usleep(2000);
    lcd_write_cmd(LCD_16207_0_BASE,0x01);
    usleep(2000);
    lcd_write_cmd(LCD_16207_0_BASE,0x06);
    usleep(2000);
    lcd_write_cmd(LCD_16207_0_BASE,0x80);
    usleep(2000);
}

void LCD_Show_Text(char* Text) {
    int i;
    for(i=0;i<strlen(Text);i++) {
        lcd_write_data(LCD_16207_0_BASE,Text[i]);
        usleep(2000);
    }
}

void LCD_Line2() {
  lcd_write_cmd(LCD_16207_0_BASE,0xC0);
  usleep(2000);
}

int main(int argc, char **argv)
{
    void *base;
    uint32_t *lcd_base;
    int memdevice_fd;

    // if(argc != 2) {
    //     printf("Performing 24 LED shifts.\n");

    // } else {
    //     // num_led_shifts = atoi(argv[1]);
    // }


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
    lcd_base = (uint32_t*) (base + ((ALT_LWFPGASLVS_OFST + LCD_16207_0_BASE) & HW_REGS_MASK));



    char line1[16] = "HERE IS THE";
    char line2[16] = "WAV WORLD";
    //  Initialize the LCD
    LCD_Init();
    LCD_Show_Text(line1);
    LCD_Line2();
    LCD_Show_Text(line2);


    // unmap and close /dev/mem 
    if( munmap(base, HW_REGS_SPAN) < 0) {
        perror("munmap() failed.");
        close(memdevice_fd);
        exit(EXIT_FAILURE);
    }

    close(memdevice_fd);
    exit(EXIT_SUCCESS);
}
