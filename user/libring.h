int rb_open(char *name);
int rb_close(int desc);
void bookr(int desc);
void bookw(int desc);
void rb_write_start(int desc, char **addr, int *bytes);
void rb_write_finish(int desc, int bytes);
void rb_read_start(int desc, char **addr, int *bytes);
void rb_read_finish(int desc, int bytes);
