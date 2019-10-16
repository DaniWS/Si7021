#include "mbed.h"
//QUITAR TICKER e implementarlo fuera

I2C i2c(PB_7, PB_6); //pins for I2C communication (SDA, SCL)
Serial pc(USBTX, USBRX, 9600); //9600 baudios - used to print some values
DigitalOut ledColour(PB_15); // TCS34725 led
DigitalOut ledR(PA_4); //RGB led - red light
DigitalOut ledG(PH_1);  //RGB led - green light
DigitalOut ledB(PA_14);  //RGB led - blue light

// We set the sensor address. For TCS34725 is 0x29 = ‭‭0010 1001‬ (bin) ->> ‭0101 0010‬ (bin) = 0x52
// We shift 1 bit to the left because in I2C protocol slave address is 7-bit. So we discard the 8th bit
int sensor_addr = 0x29 << 1;
//Variable for ISR
bool readColour =  false;

Ticker t;

//ISR code
void read_colour (void)
{
    readColour =  true;
}

//Get max value (r,g,b) function
char getMax(int r, int g, int b)
{
    char result;
    int max;
    if (r < g) {
        max = g;
        result = 'g';
    } else {
        max= r;
        result = 'r';
    }
    if (max < b) {
        result = 'b';
    }
    return result;
}

int main()
{

    t.attach(read_colour, 1.0); // Every second the ticker triggers an interruption
    green = 1; // LED of B-L072Z-LRWAN1 board on

    // Connect to the Color sensor and verify whether we connected to the correct sensor.

    //  i2c.frequency(200000);
    /*******************************************************
      * id_regval contains command register value: ‭1001 0010 *
      * COMMAND REGISTER structure                           *
      * 7   |  6     5  |  4   3   2   1  0                  *
      * CMD      TYPE          ADDR/SF                       *
      *                                                      *
      * CMD=1                                                *
      * TYPE=00 -> repeated byte protocol transaction        *
      * ADDR/SF= 10010 -> ADDR 0x12 - Device ID              *
      ********************************************************/




    // Initialize color sensor

    // Timing register address 0x01 (0000 0001). We set 1st bit to 1 -> 1000 0001
    char timing_register[2] = {0x81,0x50}; //0x50 ~ 400ms
    i2c.write(sensor_addr,timing_register,2,false);

    // Control register address 0x0F (0000 1111). We set 1st bit to 1 -> 1000 1111
    char control_register[2] = {0x8F,0}; //{0x8F, 0x00}, {1000 1111, 0000 0000} -> 1x gain
    i2c.write(sensor_addr,control_register,2,false);

    // Enable register address 0x00 (0000 0000). We set 1st bit to 1 -> 1000 0000
    char enable_register[2] = {0x80,0x03}; //{0x80, 0x03}, {1000 0000, 0000 0011} -> AEN = PON = 1
    i2c.write(sensor_addr,enable_register,2,false);

    // Read data from color sensor (Clear/Red/Green/Blue)
    char clear_reg[1] = {0x94}; // {‭1001 0100‬} -> 0x14 and we set 1st bit to 1
    char clear_data[2] = {0,0};
    char red_reg[1] = {0x96}; // {‭1001 0110‬} -> 0x16 and we set 1st bit to 1
    char red_data[2] = {0,0};
    char green_reg[1] = {0x98}; // {‭1001 1000‬} -> 0x18 and we set 1st bit to 1
    char green_data[2] = {0,0};
    char blue_reg[1] = {0x9A}; // {‭1001 1010‬} -> 0x1A and we set 1st bit to 1
    char blue_data[2] = {0,0};

    // Turn on the led in the sensor
    ledColour = 1;

    while (true) {
        //If ISR has been executed, we read clear & RGB values
        if (readColour) {
            readColour = 0; //readColour = false
            //Reads clear value
            i2c.write(sensor_addr,clear_reg,1, true);
            i2c.read(sensor_addr,clear_data,2, false);

            //We store in clear_value the concatenation of clear_data[1] and clear_data[0]
            int clear_value = ((int)clear_data[1] << 8) | clear_data[0];

            //Reads red value
            i2c.write(sensor_addr,red_reg,1, true);
            i2c.read(sensor_addr,red_data,2, false);

            //We store in red_value the concatenation of red_data[1] and red_data[0]
            int red_value = ((int)red_data[1] << 8) | red_data[0];

            //Reads green value
            i2c.write(sensor_addr,green_reg,1, true);
            i2c.read(sensor_addr,green_data,2, false);

            //We store in green_value the concatenation of green_data[1] and green_data[0]
            int green_value = ((int)green_data[1] << 8) | green_data[0];

            //Reads blue value
            i2c.write(sensor_addr,blue_reg,1, true);
            i2c.read(sensor_addr,blue_data,2, false);

            //We store in blue_value the concatenation of blue_data[1] and blue_data[0]
            int blue_value = ((int)blue_data[1] << 8) | blue_data[0];

            // print sensor readings

            pc.printf("Clear (%d), Red (%d), Green (%d), Blue (%d)\n\r", clear_value, red_value, green_value, blue_value);

            //Obtains which one is the greatest - red, green or blue
            char max = getMax(red_value, green_value, blue_value);

            //Switchs the color of the greatest value. First, we switch off all of them
            ledR.write(1);
            ledG.write(1);
            ledB.write(1);
            if (max == 'r') {
                ledR.write(0);
                pc.printf("R\r\n");
            } else if(max == 'g') {
                pc.printf("G\r\n");
                ledG.write(0);
            } else {
                pc.printf("B\r\n");
                ledB.write(0);
            }

        }
    }
}
