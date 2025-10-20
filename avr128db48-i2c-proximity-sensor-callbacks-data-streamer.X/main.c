/*
© [2025] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/
/**
 * I2C_HOST EXAMPLE Generated Driver File
 * 
 * @file i2c_host example.c
 * 
 * @ingroup i2c_host example
 * 
 * @version I2C_HOST EXAMPLE Example Version 1.0.1
 *
 * @brief Generated file for
 *        Example:           4. I2C Proximity Sensor 
 *        Implementation:    Interrupts with callbacks 
 *        Visualization:     Data Streamer   
 *        MCU Device family: AVR
*/

#include "mcc_generated_files/system/system.h"

#include <util/delay.h>

// Note:  VCNL4200 - High Sensitivity Long Distance Proximity and Ambient Light Sensor With I2C Interface
//        Reference to the VCNL4200 data sheet:  https://www.vishay.com/docs/84430/vcnl4200.pdf
//        The VCNL4200 command codes are located in Table 1 (page 9) of the VCNL4200 data sheet
#define PROXIMITY_I2C_ADDRESS  ((uint8_t)0x51)
#define PROXIMITY_REGISTER     ((uint8_t)0x08)

uint8_t VCNL4200_Initialize(void);
uint8_t VCNL4200_ProximityRead(uint16_t* proximityValue);
void Timer_Callback_100ms(void);

// TODO: Replace TimerX with number of Timer chosen as dependency. 
//       Matches name of const struct TIMER_INTERFACE, from MCC Generated Files > timer > tcXX.c
static const struct TIMER_INTERFACE *Timer = &Timer0; 

static volatile bool SEND_FRAME = false; // volatile because this variable is used inside & outside the ISR.
static uint16_t proximityValue; // VCNL4200 sensor result
static volatile uint8_t frameCount; 
static i2c_host_error_t errorState = I2C_ERROR_NONE;

uint8_t VCNL4200_ProximityRead(uint16_t* proximityValue)
{
    union
    {
        uint8_t  bytes[2]; 
        uint16_t value;
    } proximityResponse;    
    uint8_t proximityData = PROXIMITY_REGISTER;  
    
    I2C_Host.WriteRead(PROXIMITY_I2C_ADDRESS, &proximityData, sizeof(proximityData), &proximityResponse.bytes[0], sizeof(proximityResponse));
    while(I2C_Host.IsBusy())
    {
    }   
      
    *proximityValue = proximityResponse.value;
    return errorState;  
}


uint8_t VCNL4200_Initialize(void)
{
    uint8_t initConfigOne[3] = {0x03, 0x2A, 0x0A};
    uint8_t initConfigTwo[3] = {0x03, 0x70, 0x07};

    I2C_Host.Write(PROXIMITY_I2C_ADDRESS, &initConfigOne, sizeof(initConfigOne));
    while(I2C_Host.IsBusy())
    {
    }   

    I2C_Host.Write(PROXIMITY_I2C_ADDRESS, &initConfigTwo, sizeof(initConfigTwo));
    while(I2C_Host.IsBusy())
    {
    }   

    return errorState; 
}

void Timer_Callback_100ms(void)
{
    SEND_FRAME = true;
}

int main(void)
{
    SYSTEM_Initialize();   

    errorState = VCNL4200_Initialize();     // Initializes the proximity sensor (VCNL4200) over the I2C bus
    DataStreamer.proximity = 0;   
    DataStreamer.frameCount = 0;                      
    Timer->TimeoutCallbackRegister(Timer_Callback_100ms);

    while(1)
    {
        if(SEND_FRAME == true)
        {
            errorState = VCNL4200_ProximityRead(&proximityValue);
            DataStreamer.proximity = proximityValue;
            DataStreamer.frameCount++;
            WriteFrame();
            IO_LED_Toggle();
            IO_Debug_Toggle();
            SEND_FRAME = false;
        }
    }    
}
