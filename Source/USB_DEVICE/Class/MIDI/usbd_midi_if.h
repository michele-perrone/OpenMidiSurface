/*
 * usbd_midi_if.h
 *
 *  Created on: Oct 31, 2021
 *      Author: michele
 */

#ifndef APP_USBD_MIDI_IF_H_
#define APP_USBD_MIDI_IF_H_

#include "usbd_midi.h"

extern USBD_MIDI_ItfTypeDef  USBD_MIDI_fops_FS;

extern void sendMidiMessage(uint8_t *msg, uint16_t size);
extern uint8_t USBD_MIDI_SendData (USBD_HandleTypeDef *pdev, uint8_t *pBuf, uint16_t length);

extern void openMidiSurface_init(void);
extern void setHdlNoteOff(void (*fptr)(uint8_t ch, uint8_t note, uint8_t vel));
extern void setHdlNoteOn(void (*fptr)(uint8_t ch, uint8_t note, uint8_t vel));
extern void setHdlCtlChange(void (*fptr)(uint8_t ch, uint8_t num, uint8_t value));

extern void sendNoteOn(uint8_t ch, uint8_t note, uint8_t vel);
extern void sendNoteOff(uint8_t ch, uint8_t note);
extern void sendCtlChange(uint8_t ch, uint8_t num, uint8_t value);

// These functions are to be called in the main loop
extern void processMidiMessage(void);
extern void USBD_MIDI_SendPacket(void);

#endif /* APP_USBD_MIDI_IF_H_ */
