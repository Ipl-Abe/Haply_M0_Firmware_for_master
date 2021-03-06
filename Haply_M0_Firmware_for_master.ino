/**
 ************************************************************************************************
* @file       Haply_M0_Firmware.ino
* @author     Steve Ding, Colin Gallacher
* @version    V0.3.0
* @date       11-December-2018
* @brief      Haply M0 board firmware for encoder and sensor read and torque write using
 *             on-board actuator ports
************************************************************************************************
* @attention
*
*
************************************************************************************************
*/
 

#include <stdlib.h>
#include <Encoder.h>
#include "ADC_Boost.h"
#include "PWM_Arduino_Zero.h"
#include "Haply_M0_Firmware_V04.h"
 

//char torque_data_char[4];
char M1_angle_data[4];
char M2_angle_data[4];
char tip_pos_data[2]; // x_pos,y_pos
 

char M1_torque_data[4];
char M2_torque_data[4];
int count=0;
 

bool M1_angle_status = false;
bool M2_angle_status = false;

float prev_pos[2];
float prev_angle[2];
float ref_pos[2];




/* Actuator, Encoder, Sensors parameter declarations *******************************************/
actuator actuators[TOTAL_ACTUATOR_PORTS];
encoder encoders[TOTAL_ACTUATOR_PORTS];
pwm pwmPins[PWM_PINS];
sensor analogSensors[ANALOG_PINS];
 


/* Actuator Status and Command declarations ****************************************************/
 

/* Address of device that sent data */
char deviceAddress;
 

/* communication interface control, defines type of instructions recieved */
char cmdCode;
 

/* communication interface control, defines response to send */
char replyCode = 3;
 

/* Iterator and debug definitions **************************************************************/
long lastPublished = 0;
long currentState = 0;

// char inChar;
float M1_angle_value=0;
float M2_angle_value=0;

float angle_mat[2];
float ref_M1_angle=0.0;
float ref_M2_angle=0.0;
  
int dataLength = 2;
int dataLength2 = 4;
int j;
float torque_data[2];
  
char ref_M1_angle_char[4];
char ref_M2_angle_char[4];
  
char vir_M1torque_char[4];
char vir_M2torque_char[4];

/* main setup and loop block  *****************************************************************/
 

/**
* Main setup function, defines parameters and hardware setup
*/
void setup() {
  
  ADC_Boost();
  SerialUSB.begin(0);
  Serial1.begin(115200);
}

/**
* Main loop function
*/ 
void loop() {
  char send_angleValues[4 * dataLength + 1];
  char incomeValues[4*dataLength2+1];

  currentState = micros();
  if(currentState - lastPublished >= 50){
    lastPublished = currentState;
    if(SerialUSB.available() > 0){
      cmdCode = command_instructions();
      switch(cmdCode){
        case 0:
          deviceAddress = reset_haply(actuators, encoders, analogSensors, pwmPins);
          break;
        case 1:
          deviceAddress = setup_device(actuators, encoders, analogSensors, pwmPins);
          break;
        case 2:                    
          // get angle data from encoders
          M1_angle_value = read_encoder_value(&encoders[0]);
          FloatToBytes(M1_angle_value, M1_angle_data);
          M2_angle_value = read_encoder_value(&encoders[1]);
          FloatToBytes(M2_angle_value, M2_angle_data);
          angle_mat[0] = M1_angle_value;
          angle_mat[1] = M2_angle_value;
          torque_data[0] = 0.0;
          torque_data[0] = 0.0;

          //if(Serial1.available()>0){
            Serial1.readBytes(incomeValues, 4*dataLength2+1);
             if(incomeValues[0]=='+')
             {            
                j = 1;            
                ArrayCopy(incomeValues, j, ref_M1_angle_char, 0, 4);
                j = j + 4;   
                ArrayCopy(incomeValues, j, ref_M2_angle_char, 0, 4);
                j = j + 4;   
                ArrayCopy(incomeValues, j, vir_M1torque_char, 0, 4);
                j = j + 4;   
                ArrayCopy(incomeValues, j, vir_M2torque_char, 0, 4);
                
                vir_torques[0] = BytesToFloat(vir_M1torque_char);
                vir_torques[1] = BytesToFloat(vir_M2torque_char);
                vir_torques[2] = 0.0;
                vir_torques[3] = 0.0;
                
                ref_M1_angle = BytesToFloat(ref_M1_angle_char);
                ref_M2_angle = BytesToFloat(ref_M2_angle_char);

                ref_pos[0] = ref_M1_angle;
                ref_pos[1] = ref_M2_angle;

              create_angle(&actuators[0], angle_mat[0], ref_M1_angle);
              create_angle(&actuators[1], angle_mat[1], ref_M2_angle);
             }         
          deviceAddress = write_states(pwmPins, actuators);
          replyCode = 1;

          M1_angle_value = read_encoder_value(&encoders[0]);
          FloatToBytes(M1_angle_value, M1_angle_data);
          M2_angle_value = read_encoder_value(&encoders[1]);
          FloatToBytes(M2_angle_value, M2_angle_data);
          angle_mat[0] = M1_angle_value;
          angle_mat[1] = M2_angle_value;

          send_angleValues[0] = '!';
          j = 1;
          ArrayCopy(M1_angle_data, 0, send_angleValues, j, 4);
          j = j + 4;
          ArrayCopy(M2_angle_data, 0, send_angleValues, j, 4);

          Serial1.write(send_angleValues, 4* dataLength+1);
          
          count++;
            break;
          default:
            break;
        }
    } 

    switch(replyCode){
      case 0:
        break;
      case 1:
        read_states(encoders, analogSensors, deviceAddress);
        replyCode = 3;
        break;
      default:
        break;
    }
  }
 

}
