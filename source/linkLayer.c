#include "linkLayer.h"

int main()
{
  return 0;
}

static char write_sequence_number = 0;
static char read_sequence_number = 0;
static char last_frame_accepted = 1;

unsigned char getAddress(int caller, char control_field)
{
  if (control_field == C_SET || control_field == C_DISC)
  {
    if (caller == SENDER)
    {
      return A_SENDER_COMMAND;
    }
    return A_RECEIVER_COMMAND;
  }
  else
  {
    if (caller == RECEIVER)
    {
      return A_RECEIVER_COMMAND;
    }
    return A_SENDER_COMMAND;
  }
}

unsigned char getBCC(char *data, int data_size)
{
  unsigned char bcc = 0;
  int i;
  for (i = 0; i < data_size; i++)
  {
    bcc = bcc ^ data[i];
  }
  return bcc;
}

void buildControlFrame(char *frame, int caller, char control_field, long sequence_number)
{
  frame[0] = FLAG;
  frame[1] = getAddress(caller, control_field);
  if (sequence_number == NULL){
	frame[2] = control_field;
  }else{
	frame[2] = ((sequence_number % 2) << 7) + control_field;
  }
  frame[3] = frame[1] ^ frame[2];
  frame[4] = FLAG;
}

void buildDataFrame(char **frame, char *data, int data_size, int *frame_size, long sequence_number)
{
  printf("\nDEBUG: STRAT BUILDDATAFRAME\n");
  *frame_size = data_size + 6;
  (*frame) = (char *)malloc(*frame_size * sizeof(char));
  (*frame)[0] = FLAG;
  (*frame)[1] = A_SENDER_COMMAND;
  (*frame)[2] = (char)((sequence_number) % 2) << 6;
  (*frame)[3] = (*frame)[1] ^ (*frame)[2];
  memcpy(&((*frame)[4]), data, data_size);
  (*frame)[4 + data_size] = getBCC(data, data_size);
  (*frame)[5 + data_size] = FLAG;

  int i = 0;
  for(i = 0; i < *frame_size; i++){
    printf("DEBUG: Data Frame[%d] == 0x%02X\n", i, *frame[i]);
  }
  byteStuffing(frame, frame_size);
  for(i = 0; i < *frame_size; i++){
    printf("DEBUG: Data Frame[%d] == 0x%02X\n", i, *frame[i]);
  }
  printf("\nDEBUG: END BUILDDATAFRAME\n");
}

void byteStuffing(char **frame, int *frame_size)
{
  printf("\nDEBUG: START BYTESTUFFING\n");
  int i;
  int allocated_space = *frame_size;
  for (i = 1; i < *frame_size - 1; i++)
  {
    char currentByte = (*frame)[i];
    if (currentByte == FLAG || currentByte == ESCAPE)
    {
      (*frame)[i] = currentByte ^ STUFF_XOR;
      if (allocated_space <= *frame_size)
      {
        allocated_space = 2 * allocated_space;
        realloc(*frame, allocated_space * sizeof(char));
      }
      insertValueAt(ESCAPE, *frame, i, frame_size);
      i++;
    }
  }
  printf("\nDEBUG: END BYTESTUFFING\n");
}

void byteUnstuffing(char **frame, int *frame_size)
{
  printf("\nDEBUG: START BYTEUNSTUFFING\n");
  int i;
  for (i = 0; i < *frame_size; i++)
  {
    if ((*frame)[i] == ESCAPE)
    {
      char byte_xored = (*frame)[i + 1] ^ STUFF_XOR;
      if (byte_xored == FLAG || byte_xored == ESCAPE)
      {
        removeValueAt(*frame, i, frame_size);
        (*frame)[i] = byte_xored;
      }
    }
  }
  // Realloc to free unused memory
  realloc(*frame, *frame_size * sizeof(char));
  printf("\nDEBUG: END BYTEUNSTUFFING\n");
}

/*------------------------------------*/
/*------------------------------------*/
/*---------------LLOPEN---------------*/
/*------------------------------------*/
/*------------------------------------*/


int llopen(char *port, int caller){
  printf("\nDEBUG: START LLOPEN\n");
  int fileDescriptor = openSerialPort(port, caller);
  if(fileDescriptor < 0){
    return -1;
  }
  if(setNewSettings(fileDescriptor, caller) < 0){
    return -1;
  }

  int returnValue;
  if (caller == TRANSMITTER) {
    returnValue = llopenTransmitter(fileDescriptor);
  } else if (caller == RECEIVER) {
    returnValue = llopenReceiver(fileDescriptor);
  }

  printf("\nDEBUG: END LLOPEN\n");
  return returnValue;
}

int llopenSender(int fileDescriptor){
  printf("\nDEBUG: START LLOPENSENDER\n");
  char set_frame[5];
  buildControlFrame(set_frame, TRANSMITTER, C_SET, NULL);

  printf("\nDEBUG: END LLOPENSENDER\n");
  return 0;
}

int llopenReceiver(int fileDescriptor){
  printf("\nDEBUG: START LLOPENRECEIVER\n");
  char ua_frame[5];
  buildControlFrame(ua_frame, RECEIVER, C_UA, NULL);

  printf("\nDEBUG: END LLOPENRECEIVER\n");
}



int readControlFrame(int sp_fd, char address_expected, char expected_control_field, ControlStruct *control_struct){
	Control_State state = START;
	char char_read;
	while(state != STOP){
		read(sp_fd,&char_read,1);
		switch(state){
			case START:{
				if (char_read == FLAG){
					state = FLAG_REC;
				}
				break;
			}
			case FLAG_REC:{
				if (char_read == address_expected){
					state = A_REC;
				}else if (char_read != FLAG){
					state = START;
				}
				break;
			}
			case A_REC:{
				break;
			}
			case C_REC:{

			}
			case BCC_OK:{

			}
		}
	}
}

int writeAndReadReply(int sp_fd, char* frame_to_write, int frame_size){
	printf("\nDEBUG: START WRITEANDPROCESSREPLY\n");
	unsigned int currentTries = 0;
	while(currentTries++ < MAX_TRIES){
		write(sp_fd,frame_to_write,frame_size);
		if (processReply() == 0){
			return 0;
		}else
	}
	if (currentTries >= MAX_TRIES){
		printf("\nMAX TRIES REACHED\n");
		return -1;
	}
	printf("\nDEBUG: END WRITEANDPROCESSREPLY\n");
}
