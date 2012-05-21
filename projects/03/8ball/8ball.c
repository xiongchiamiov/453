#include <minix/drivers.h>
#include <minix/driver.h>

FORWARD _PROTOTYPE( void init, (void)					);
FORWARD _PROTOTYPE( void do_cancel, (message *m_ptr)			);
FORWARD _PROTOTYPE( void do_ioctl, (message *m_ptr)			);
FORWARD _PROTOTYPE( void do_open, (message *m_ptr)			);
FORWARD _PROTOTYPE( void do_close, (message *m_ptr)			);
FORWARD _PROTOTYPE( void do_read, (message *m_ptr)			);
FORWARD _PROTOTYPE( void do_write, (message *m_ptr)			);
FORWARD _PROTOTYPE( void do_select, (message *m_ptr)			);
FORWARD _PROTOTYPE( void reply, (int replyAddress, int process, int status));

/*===========================================================================*
 *				tty_task				     *
 *===========================================================================*/
PUBLIC int main(void)
{
	/* Main routine of the terminal task. */

	message eightBallMessage;		/* buffer for all incoming messages */
	int ipc_status;
	int r;

	init();

	while (TRUE) {
		printf("Waiting for a message...\n");
		/* Get a request message. */
		r= driver_receive(ANY, &eightBallMessage, &ipc_status);
		if (r != 0)
			panic("driver_receive failed with: %d", r);

		printf("8ball driver received message!\n");
		/* Execute the requested device driver function. */
		/* These constants are defined in minix/com.h . */
		switch (eightBallMessage.m_type) {
			/*case CANCEL:        do_cancel(&eightBallMessage);  break;*/
			case DEV_OPEN:      do_open(&eightBallMessage);    break;
			case DEV_CLOSE:     do_close(&eightBallMessage);   break;
			/*case TTY_SETPGRP:   break;*/
			/*case TTY_EXIT:      break;*/
			/*case DEV_SELECT:    do_select(&eightBallMessage);  break;*/
			/*case DEV_STATUS:    break;*/
			/*case DEV_REOPEN:    break;*/
			case DEV_READ_S:    do_read(&eightBallMessage);    break;
			/*case DEV_WRITE_S:   do_write(&eightBallMessage);   break;*/
			/*case DEV_SCATTER_S: break;*/
			/*case DEV_GATHER_S:  break;*/
			/*case DEV_IOCTL_S:   do_ioctl(&eightBallMessage);   break;*/
			/*case DEV_MMAP_S:    break;*/
			default:
				printf("Warning, 8ball got unexpected request %x from %d\n",
				       eightBallMessage.m_type, eightBallMessage.m_source);
				reply(eightBallMessage.m_source, eightBallMessage.IO_ENDPT, EINVAL);
		}
	}

	return 0;
}

PRIVATE void init()
{
	printf("Shaking the Magic 8 Ball(TM)...");
	sef_startup();
	printf("ready.\n");
}

PRIVATE void reply(endpoint_t replyAddress, int process, int status)
{
	message response;
	response.m_type = TASK_REPLY;
	response.REP_ENDPT = process;
	response.REP_STATUS = status;
	
	printf("8ball: replying to message\n");
	
	status = sendnb(replyAddress, &response);
	if (status != OK) {
		printf("reply: send to %d failed: %s (%d)\n",
		       replyAddress, strerror(status), status);
	}
}

PRIVATE void do_open(message* message)
{
	printf("do_open called\n");
	reply(message->m_source, message->IO_ENDPT, OK);
}

PRIVATE void do_close(message* message)
{
	printf("do_close called\n");
	reply(message->m_source, message->IO_ENDPT, DEV_CLOSE_REPL);
}

PRIVATE void do_read(message* message)
{
}

/*********************************************************************************
                                    UNUSED
*********************************************************************************/

PRIVATE void do_write(message* message)
{
}

PRIVATE void do_ioctl(message* message)
{
}

PRIVATE void do_select(message* message)
{
}

PRIVATE void do_cancel(message* message)
{
}

