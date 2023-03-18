#include "mbed.h"

I2C i2c(P0_10,P0_11);

void lcd_init(int adr);     //lcd init func
const int lcd_adr = 0x7C;   //lcd i2c adr 0x7C
void char_disp(int adr, int8_t position, char data);
void val_disp(int adr, int8_t position, int8_t digit,int val);

#define intv 0.1
#define res 0.0625  //resolution 2.048/2^16
#define att 11      //att 110k/10k
const int sps=0b10;     //0b00->240sps,12bit 0b01->60sps,14bit 0b10->15sps,16bit
const int pga=0;     //0->x1 1->x2 2->x4 3->x8
const int adc_adr = 0xd0;
char buf[2],set[1];
int16_t raw_val;
float val_f;            //raw v/i val (float)
int16_t val;            //raw v/i val (int)
uint16_t disp_val[2];   //display val
float g_calib=0.998;        //gain error calibration

int main(){
    wait(0.2);
    lcd_init(lcd_adr);
    set[0]=(0b1001<<4)+(sps<<2)+pga;//0x98
    i2c.write(adc_adr,set,1);
    wait(0.2);
    char_disp(lcd_adr,4,'.');
    char_disp(lcd_adr,0x40+7,'V');
    while(1){
        i2c.read(adc_adr|1,buf,2);
        raw_val=(buf[0]<<8)+buf[1];
        val_f=raw_val*(att*res)*g_calib;     //mV unit expression
        val=(int16_t)val_f;

        if(raw_val<0){
            char_disp(lcd_adr,1,'-');
            val=val*-1;
        }else{
            char_disp(lcd_adr,1,'+');
        }
        
        disp_val[0]=val%1000;     //mV
        disp_val[1]=val/1000;     //V
        val_disp(lcd_adr,5,3,disp_val[0]);
        val_disp(lcd_adr,2,2,disp_val[1]);
    }
    //adc1 overflow check
    if(raw_val==32767||raw_val==-32768){
        char_disp(lcd_adr,0x40+0,'!');
    }else{
        char_disp(lcd_adr,0x40+0,' ');
    }
    wait(intv);
}

//disp char func
void char_disp(int adr, int8_t position, char data){
    char buf[2];
    buf[0]=0x0;
    buf[1]=0x80+position;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(adr,buf, 2);
    buf[0]='@';
    buf[1]=data;
    i2c.write(adr,buf, 2);
}

//disp val func
void val_disp(int adr, int8_t position, int8_t digit, int val){
    char buf[2];
    char data[4];
    int8_t i;
    buf[0]=0x0;
    buf[1]=0x80+position;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(adr,buf, 2);
    data[3]=0x30+val%10;        //1
    data[2]=0x30+(val/10)%10;   //10
    data[1]=0x30+(val/100)%10;  //100
    data[0]=0x30+(val/1000)%10; //1000
    buf[0]='@';
    for(i=0;i<digit;++i){
        buf[1]=data[i+4-digit];
        i2c.write(adr,buf, 2);
    }
}

//LCD init func
void lcd_init(int adr){
    char lcd_data[2];
    lcd_data[0] = 0x0;
    lcd_data[1]=0x38;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x39;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x14;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x70;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x56;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x6C;
    i2c.write(adr, lcd_data, 2);
    wait(0.2);
    lcd_data[1]=0x38;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x0C;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x01;
    i2c.write(adr, lcd_data, 2);
}
