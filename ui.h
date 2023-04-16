#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED

int ui_init(int *argc, char ***argv, void *userdata);

void ui_run(int *quit);

void ui_exit(void);

#endif /* UI_H_INCLUDED */
