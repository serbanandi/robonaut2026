#ifndef ANLG_H
#define ANLG_H

#define ANLG_MOTOR_BATTERY_COUNTS_PER_VOLT 217.1f // Number of ADC counts per volt for motor battery
#define ANLG_AUX_BATTERY_COUNTS_PER_VOLT 201.61f  // Number of ADC counts per volt for auxiliary battery
#define ANLG_MOTOR_SHUNT_COUNTS_PER_AMP 902.0f    // Number of ADC counts per volt for motor current shunt

#define ANLG_DISTANCE_ADC &hadc1
#define ANLG_BATTERY_ADC &hadc2

#endif // ANLG_H
