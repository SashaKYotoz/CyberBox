#ifndef DATAMANAGER_ACTIONS_H
#define DATAMANAGER_ACTIONS_H

long find_payload_offset(const char *filename);
unsigned char* read_hidden_data(const char *filename, long offset, long *size);
void modify_hidden_data(const char *filename, long offset, unsigned char *new_data, long new_size);

#endif