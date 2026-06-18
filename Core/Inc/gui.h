#ifndef GUI_H
#define GUI_H

typedef enum { GUI_CONFIG, GUI_RUN, GUI_CALIB } GuiScreen;

void GUI_Init(void);
void GUI_Task(void);
void GUI_SetScreen(GuiScreen s);

#endif
