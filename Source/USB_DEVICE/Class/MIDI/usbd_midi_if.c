/*
 * usbd_midi_if.c
 *
 *  Created on: Oct 31, 2021
 *      Author: michele
 */


/* Includes ------------------------------------------------------------------*/
#include "usbd_midi_if.h"
#include "queue32.h"

static uint16_t MIDI_DataRx(uint8_t *msg, uint16_t length);
static uint16_t MIDI_DataTx(uint8_t *msg, uint16_t length);

stB4Arrq rxq;

void (*cbNoteOff)(uint8_t ch, uint8_t note, uint8_t vel);
void (*cbNoteOn)(uint8_t ch, uint8_t note, uint8_t vel);
void (*cbCtlChange)(uint8_t ch, uint8_t num, uint8_t value);

static int checkMidiMessage(uint8_t *pMidi);

USBD_MIDI_ItfTypeDef USBD_MIDI_fops_FS =
{
  MIDI_DataRx,
  MIDI_DataTx
};

static uint16_t MIDI_DataRx(uint8_t *msg, uint16_t length)
{
  uint16_t cnt;
  uint16_t msgs = length / 4;
  uint16_t chk = length % 4;
  if(chk == 0)
  {
    for(cnt = 0;cnt < msgs;cnt ++)
    {
      b4arrq_push(&rxq,((uint32_t *)msg)+cnt);
    }
  }
  return 0;
}

void sendMidiMessage(uint8_t *msg, uint16_t size)
{
  if(size == 4)
  {
    MIDI_DataTx(msg, size);
  }
}

static uint16_t MIDI_DataTx(uint8_t *msg, uint16_t length)
{
  uint32_t i = 0;
  while (i < length)
  {
    APP_Rx_Buffer[APP_Rx_ptr_in] = *(msg + i);
    APP_Rx_ptr_in++;
    i++;
    if (APP_Rx_ptr_in == APP_RX_DATA_SIZE)
    {
      APP_Rx_ptr_in = 0;
    }
  }
  return USBD_OK;
}

static int checkMidiMessage(uint8_t *pMidi)
{
  if(((*(pMidi + 1) & 0xf0)== 0x90)&&(*(pMidi + 3) != 0))
  {
    return 2;
  }
  else if(((*(pMidi + 1) & 0xf0)== 0x90)&&(*(pMidi + 3) == 0))
  {
    return 1;
  }
  else if((*(pMidi + 1) & 0xf0)== 0x80){
    return 1;
  }
  else if((*(pMidi + 1) & 0xf0)== 0xb0){
    return 3;
  }
  else
  {
    return 0;
  }
}

static uint8_t buffer[4];

void openMidiSurface_init(void)
{
  b4arrq_init(&rxq);
}

void setHdlNoteOff(void (*fptr)(uint8_t ch, uint8_t note, uint8_t vel))
{
  cbNoteOff = fptr;
}

void setHdlNoteOn(void (*fptr)(uint8_t ch, uint8_t note, uint8_t vel))
{
  cbNoteOn = fptr;
}

void setHdlCtlChange(void (*fptr)(uint8_t ch, uint8_t num, uint8_t value))
{
  cbCtlChange = fptr;
}

void sendNoteOn(uint8_t ch, uint8_t note, uint8_t vel)
{
  buffer[0] = 0x09;
  buffer[1] = 0x90 | ch;
  buffer[2] = 0x7f & note;
  buffer[3] = 0x7f & vel;
  sendMidiMessage(buffer,4);
}

void sendNoteOff(uint8_t ch, uint8_t note)
{
  buffer[0] = 0x08;
  buffer[1] = 0x80 | ch;
  buffer[2] = 0x7f & note;
  buffer[3] = 0;
  sendMidiMessage(buffer,4);
}

void sendCtlChange(uint8_t ch, uint8_t num, uint8_t value)
{
  buffer[0] = 0x0b;
  buffer[1] = 0xb0 | ch;
  buffer[2] = 0x7f & num;
  buffer[3] = 0x7f & value;
  sendMidiMessage(buffer,4);
}

void processMidiMessage()
{
  uint8_t *pbuf;
  uint8_t kindmessage;
  // RX
  if(rxq.num > 0)
  {
    pbuf = (uint8_t *)b4arrq_pop(&rxq);
    kindmessage = checkMidiMessage(pbuf);
    if(kindmessage == 1)
    {
      if(cbNoteOff != NULL)
      {
        (*cbNoteOff)(*(pbuf+1)&0x0f,*(pbuf+2)&0x7f,*(pbuf+3)&0x7f);
      }
    }
    else if(kindmessage == 2)
    {
      if(cbNoteOn != NULL)
      {
        (*cbNoteOn)(*(pbuf+1)&0x0f,*(pbuf+2)&0x7f,*(pbuf+3)&0x7f);
      }
    }
    else if(kindmessage == 3)
    {
      if(cbCtlChange != NULL)
      {
        (*cbCtlChange)(*(pbuf+1)&0x0f,*(pbuf+2)&0x7f,*(pbuf+3)&0x7f);
      }
    }
  }
  // TX
  USBD_MIDI_SendPacket();
}

