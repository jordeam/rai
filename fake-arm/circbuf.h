#ifndef _data_log_h
#define _data_log_h

int circbuf_get_read_index(void);
int circbuf_get_write_index(void);
int circbuf_read_inc(void);
int circbuf_read_inc_n(int n);
int circbuf_write_inc(void);
int circbuf_get_buf_size(void);
void circbuf_init(int NumLogs);

#endif
