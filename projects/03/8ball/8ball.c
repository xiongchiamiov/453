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

int eightBall_queuedResponses;

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
			case DEV_WRITE_S:   do_write(&eightBallMessage);   break;
			/*case DEV_SCATTER_S: break;*/
			/*case DEV_GATHER_S:  break;*/
			/*case DEV_IOCTL_S:   do_ioctl(&eightBallMessage);   break;*/
			/*case DEV_MMAP_S:    break;*/
			default:
				printf("Warning, 8ball got unexpected request %d from %d\n",
				       eightBallMessage.m_type-DEV_RQ_BASE,
				       eightBallMessage.m_source);
				reply(eightBallMessage.m_source, eightBallMessage.IO_ENDPT, EINVAL);
		}
	}

	return 0;
}

PRIVATE void init()
{
	printf("Shaking the Magic 8 Ball(TM)...");
	sef_startup();
	eightBall_queuedResponses = 0;
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
	printf("do_open called from process %d\n", message->IO_ENDPT);
	reply(message->m_source, message->IO_ENDPT, OK);
}

PRIVATE void do_close(message* message)
{
	printf("do_close called from process %d\n", message->IO_ENDPT);
	reply(message->m_source, message->IO_ENDPT, DEV_CLOSE_REPL);
}

PRIVATE void do_read(message* message)
{
	int r;
	int destinationProcess = message->IO_ENDPT;
	vir_bytes destinationAddress = (vir_bytes)(message->ADDRESS);
	phys_bytes bytesToCopy;
	char* response = (char*)malloc(1024);
	size_t responseSize = strlen(response);
	
	printf("do_read called from process %d\n", message->IO_ENDPT);
	
	/*if (eightBall_queuedResponses == 0) {
		printf("No queued 8ball responses!\n");
		r = EIO;
	} else if (message->COUNT <= 0) { */
	if (message->COUNT <= 0) {
		r = EINVAL;
	} else {
		if (eightBall_queuedResponses == 0) {
			strcpy(response, "\0");
		} else {
			strcpy(response, "Hello from the 8ball!");
			eightBall_queuedResponses--;
		}
		bytesToCopy = (message->COUNT > responseSize)
		              ? responseSize
		              : message->COUNT;
		printf("Copying the first %d bytes of %s to %d in %d...\n",
		       bytesToCopy, (vir_bytes)response, destinationAddress,
		       destinationProcess);
		/* minix/syslib.h */
		/* The lab instructions say to use sys_datacopy, but I couldn't get
		 * that to do anything useful.  So, we use sys_safecopyto, which is
		 * what tty.c uses.                                               */
		sys_safecopyto(
			destinationProcess,
			destinationAddress,
			0,
			(vir_bytes)response,
			bytesToCopy,
			D
		);
		r = bytesToCopy;
		printf("%d responses left\n", eightBall_queuedResponses);
	}
	
	reply(message->m_source, message->IO_ENDPT, r);
}

PRIVATE void do_write(message* message)
{
	int r;
	
	printf("do_write called from process %d\n", message->IO_ENDPT);
	
	if (message->COUNT <= 0) {
		r = EINVAL;
	} else {
		/* TODO: Increase per newline, not per write. */
		eightBall_queuedResponses++;
	}
	
	reply(message->m_source, message->IO_ENDPT, r);
}

/*********************************************************************************
                                    UNUSED
*********************************************************************************/

PRIVATE void do_ioctl(message* message)
{
}

PRIVATE void do_select(message* message)
{
}

PRIVATE void do_cancel(message* message)
{
}

