#ifndef PWM_Module_H
#define PWM_Module_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

void InitPWM(void);
void InitFlywheelPWM(void);
void SetDutyA(uint8_t duty);
void SetDutyB (uint8_t duty);
void SetFlywheelDuty(uint8_t duty);
//void ChangeDirectionA(void);
//void ChangeDirectionB(void);
void SetDirectionA(uint8_t dir);
void SetDirectionB(uint8_t dir);


#endif /* PWM_Module_H */
