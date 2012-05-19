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
	/* Get a request message. */
	r= driver_receive(ANY, &eightBallMessage, &ipc_status);
	if (r != 0)
		panic("driver_receive failed with: %d", r);

	/* Execute the requested device driver function. */
	switch (eightBallMessage.m_type) {
	    case DEV_READ_S:	 do_read(&eightBallMessage);	  break;
	    case DEV_WRITE_S:	 do_write(&eightBallMessage);	  break;
	    case DEV_IOCTL_S:	 do_ioctl(&eightBallMessage);	  break;
	    case DEV_OPEN:	 do_open(&eightBallMessage);	  break;
	    case DEV_CLOSE:	 do_close(&eightBallMessage);	  break;
	    case DEV_SELECT:	 do_select(&eightBallMessage);	  break;
	    case CANCEL:	 do_cancel(&eightBallMessage);	  break;
	    default:		
		printf("Warning, TTY got unexpected request %d from %d\n",
			eightBallMessage.m_type, eightBallMessage.m_source);
	}
  }

  return 0;
}

PRIVATE void init()
{
}

PRIVATE void do_read(message* message)
{
}

PRIVATE void do_write(message* message)
{
}

PRIVATE void do_ioctl(message* message)
{
}

PRIVATE void do_open(message* message)
{
}

PRIVATE void do_close(message* message)
{
}

PRIVATE void do_select(message* message)
{
}

PRIVATE void do_cancel(message* message)
{
}

