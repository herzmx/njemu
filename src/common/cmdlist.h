/******************************************************************************

	cmdlist.c

	MAME Plus!�`���R�}���h���X�g�r���[�A

******************************************************************************/

#ifndef CMDLIST_H
#define CMDLIST_H

void load_commandlist(const char *game_name, const char *parent_name);
void free_commandlist(void);
void commandlist(int flag);
int commandlist_size_reduction(void);

#endif /* CMDLIST_H */
