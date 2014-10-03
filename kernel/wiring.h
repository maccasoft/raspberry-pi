#ifndef WIRING_H_
#define WIRING_H_

// Pin modes

#define OUTPUT              0
#define INPUT               1
#define INPUT_PULLUP        2
#define INPUT_PULLDOWN      3

#define LOW                 0
#define HIGH                1

#if defined(__cplusplus)
extern "C" {
#endif

void pinMode(int pin, int mode);
int  digitalRead(int pin);
void digitalWrite(int pin, int value);

#if defined(__cplusplus)
}
#endif

#endif /* WIRING_H_ */
