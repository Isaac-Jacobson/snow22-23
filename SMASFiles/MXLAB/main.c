/*
 * File:   main.c
 * Author: CSU
 *
 * Created on April 15, 2019, 12:59 PM
 */


#include <xc.h>
#include "pic18f26j53.h"
#include "header.h"

#define _XTAL_FREQ 8000000

#define Flash_DIR TRISAbits.TRISA0
#define Flash_PIN PORTAbits.RA0

#define SensorClear_DIR TRISAbits.TRISA1
#define SensorClear_PIN PORTAbits.RA1

#define SensorInput1_DIR TRISAbits.TRISA1 //TRISA3 for the OR gate
#define SensorInput1_PIN PORTAbits.RA1   //ra2 connected to OR gate for normal plane 
                                         // ra3 connected to AND gate for crossplane
#define SensorInput2_DIR TRISAbits.TRISA2 //TRISAX initialized to 1 in header file
#define SensorInput2_PIN PORTAbits.RA2 

#define AllCamera_DIR TRISBbits.TRISB5    //was rb2
#define AllCamera_PIN PORTBbits.RB5

#define SpeedCamera_DIR TRISBbits.TRISB4
#define SpeedCamera_PIN PORTBbits.RB4

#define AtoP1_DIR TRISCbits.TRISC0
#define AtoP1_PIN PORTCbits.RC0

#define AtoP2_DIR TRISCbits.TRISC1
#define AtoP2_PIN PORTCbits.RC1

#define Laser1Control_DIR TRISCbits.TRISC7
#define Laser1Control_PIN PORTCbits.RC7

#define Laser2Control_DIR TRISCbits.TRISC6
#define Laser2Control_PIN PORTCbits.RC6

#define INPUT 1
#define OUTPUT 0

void delay_ms(int x) {
    for (int i = 0; i < x; i++) {
        __delay_ms(1);
    }
}

//Testing method
int lasers_triggered() {
//    return SensorInput1_PIN && SensorInput2_PIN;
    return !SensorInput1_PIN; // This will make the flashes go crazy until a TRISAx/RAx that corresponds to a SensorConnectx pinout bank
//    return !SensorInput2_PIN || SensorInput1_PIN; //Trigger on both with SI2 in middle bank and SI1 in bank to left or right of that
//    return !SensorInput2_PIN;
//    return SensorInput1_PIN && !SensorInput2_PIN;
}


void main(void) {
    int PicsPerSecond = 5;
    int Sensing_Plane_Delay = 0;
    int High_Speed_Delay = 1000 / 125;              // most recent compiler would not let me do inline delays
    int High_Res_Delay = 1000 / (PicsPerSecond*2);
    
    ANCON0bits.PCFG2 = 1;
    VCFG1 = 0; // set vref- to AN2
    VCFG0 = 0; // set vref+ to
    
    Laser1Control_DIR = OUTPUT;
    Laser2Control_DIR = OUTPUT;
    Flash_DIR = OUTPUT;
    SensorClear_DIR = OUTPUT;
    AllCamera_DIR = OUTPUT;
    SpeedCamera_DIR = OUTPUT;
    
    SensorInput1_DIR = INPUT; //CHANGE BACK TO _DIR
    SensorInput2_DIR = INPUT; //CODE WORKS WHEN CHANGED TO _PIN... _DIR SEEMS UNNECESSARY
    AtoP1_DIR = INPUT;
    AtoP2_DIR = INPUT;
    
    
    SensorClear_PIN = 0;  
    Flash_PIN = 0;
    __delay_ms(40);
    SensorClear_PIN = 1;
    Laser2Control_PIN = 1;     //turns laser 1 on 
    Laser1Control_PIN = 1;   //uncomment when using cross planes

    //this loop waits for sensor to go low then cameras and flash fire
    while(1){
//        for(int i = 0; i<3; i++){ //REMOVE THIS BLOCK
//            Flash_PIN = 1;
//            __delay_ms(500);
//            SensorClear_PIN = 1;
//            __delay_ms(500);
//        }
//        if(SensorInput1_PIN == 0 && SensorInput2_PIN == 0){
//        if(SensorInput1_PIN == 0) {
//        if(SensorInput2_PIN == 0) {
        //if (lasers_triggered()) {
        
        if(1){ 
//            Laser OFF
            Laser2Control_PIN = 0;
            Laser1Control_PIN = 0; //uncomment when using cross planes
            //Flash ON
             Flash_PIN = 1;
            __delay_ms(2); //was originally 2
            
            
            //delay so long exposure isn't saturated by flash
            //__delay_ms(1);
            
            //All camera pin, triggers all cameras to take a pic
            AllCamera_PIN = 1; 
            __delay_ms(2);
            AllCamera_PIN = 0;
            //high speed delay
            __delay_ms(27); // 27ms is the lowest we can go to now to capture 2 frames 
//            SpeedCamera_PIN = 1;
//            __delay_ms(2);
//            SpeedCamera_PIN = 0;
//            __delay_ms(2);
//            SpeedCamera_PIN = 1;
//            __delay_ms(2);
//            SpeedCamera_PIN = 0;
//            __delay_ms(2);
            
            
            SpeedCamera_PIN = 1; // Commented in 
                 
            __delay_ms(2); // 26 is the minimum it can go to for all 7 frames
                //__delay_ms(2); // Commented in 
            SpeedCamera_PIN = 0;
            __delay_ms(2); // When these are both 26, it takes all 7 pictures and saves with 1 flash - Peter
            
//            for(int i =0; i<7; i++){ 
//                
//                SpeedCamera_PIN = 1; // Commented in 
//                 
//                __delay_ms(2); // 26 is the minimum it can go to for all 7 frames
//                //__delay_ms(2); // Commented in 
//                 SpeedCamera_PIN = 0;
//                __delay_ms(2); // When these are both 26, it takes all 7 pictures and saves with 1 flash - Peter
//                //SpeedCamera_PIN = 1; // Commented in
//                
//                //__delay_ms(2); // Commented in 
//               
//                //__delay_ms(2);
//                //SpeedCamera_PIN = 0;
//                //SpeedCamera_PIN = 0;
//                
//                
//            }
//           
            Flash_PIN = 0; 
            //-------------------------
            //strobe high speed
            //SpeedCamera_PIN = 1; // Commented in 
            //__delay_ms(5); // Commented in 
            //SpeedCamera_PIN = 0; // Commented in
            //-------------------------
            //__delay_ms(20); // was originally 1500
            
            
            //Laser ON
            Laser2Control_PIN = 1;
            Laser1Control_PIN = 1; //uncomment when using cross planes
            //clear on
            SensorClear_PIN = 0;
            //high res - high speed delay
            __delay_ms(2000);
           
            //clear off
            SensorClear_PIN = 1;
            
        }
//        __delay_ms(1000); //DELETE ME
        
    }
    
    return;
}
// //Laser OFF
//            Laser2Control_PIN = 0;
//            //Laser1Control_PIN = 0; //uncomment when using cross planes
//            //Flash ON
//             Flash_PIN = 1;
//            __delay_ms(1); //was originally 2
//            Flash_PIN = 0; 
//            
//            //delay so long exposure isn't saturated by flash
//            //__delay_ms(1);
//            
//            //All camera pin
//            AllCamera_PIN = 1;
//            __delay_ms(40);
//            AllCamera_PIN = 0;
//            //high speed delay
//            __delay_ms(50);
//            
//            
//            //-------------------------
//            //strobe high speed
//            //SpeedCamera_PIN = 1; // Commented in 
//            //__delay_ms(5); // Commented in 
//            //SpeedCamera_PIN = 0; // Commented in
//            //-------------------------
//            __delay_ms(500); // was originally 1500
//            
//            //Laser ON
//            Laser2Control_PIN = 1;
//            //Laser2Control_PIN = 1; //uncomment when using cross planes
//            //clear on
//            SensorClear_PIN = 0;
//            //high res - high speed delay
//            delay_ms(5);
//           
//            //clear off
//            SensorClear_PIN = 1;
//        }
//        
//    }
//    
//    return;
//}
