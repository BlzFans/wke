#if !defined(__COREFOUNDATION_CFWINDOWSMESSAGE__)
#define __COREFOUNDATION_CFWINDOWSMESSAGE__ 1

#if TARGET_OS_WIN32

#include <CoreFoundation/CFBase.h>

#define kCFWindowsNamedPipeInLineMemory 4096*10

// A message layout comparable to mach
// With CFWindowsNamedPipes, every client talking to a named pipe gets the same handle.
// So, we can't use msgh_remote_pipe as a unique per client key.
// Instead, we'll add the client pid in each message, and use that as our per client key.
// Since we never use msgh_local_pipe, we won't keep that around.

typedef unsigned int windows_msg_copy_options_t;
typedef unsigned int windows_msg_bits_t;
typedef	unsigned int windows_msg_size_t;
typedef int windows_msg_id_t;
typedef unsigned int windows_msg_descriptor_type_t;

typedef	struct
{
    windows_msg_bits_t	msgh_bits;
    windows_msg_size_t	msgh_size;
    void *	    msgh_remote_pipe;
    windows_msg_size_t 	msgh_reserved;
    windows_msg_id_t     		msgh_id;
} windows_msg_header_t;

typedef struct
{
    windows_msg_size_t    msgh_descriptor_count;
} windows_msg_body_t;
    
#define WINDOWS_MSG_BACKED_MEMORY_NAME_SIZE	    16

typedef struct
{
    void*				address;
    windows_msg_size_t       	size;
    Boolean     		deallocate: 8;
    windows_msg_copy_options_t       copy: 8;
    unsigned int     		pad1: 8;
    char                backed_memory_name[WINDOWS_MSG_BACKED_MEMORY_NAME_SIZE];
    windows_msg_descriptor_type_t    type: 8;
} windows_msg_ool_descriptor_t;

typedef union
{
    windows_msg_ool_descriptor_t		out_of_line;
} windows_msg_descriptor_t;
    

#define WINDOWS_MSGH_BITS(remote, local)				\
((remote) | ((local) << 8))

#define WINDOWS_MSG_TYPE_MOVE_SEND_ONCE    18      /* Must hold sendonce rights */
#define WINDOWS_MSG_TYPE_COPY_SEND		19	/* Must hold send rights */
#define WINDOWS_MSG_TYPE_MAKE_SEND		20	/* Must hold receive rights */
#define WINDOWS_MSG_TYPE_MAKE_SEND_ONCE    21      /* Must hold receive rights */

#define WINDOWS_MSG_PHYSICAL_COPY          0
#define WINDOWS_MSG_VIRTUAL_COPY           1

#define WINDOWS_MSG_OOL_DESCRIPTOR                 1

#define WINDOWS_MSGH_BITS_COMPLEX          0x80000000U

#define WINDOWS_NAMED_PIPE_NULL           0

#define WINDOWS_SEND_TIMEOUT       0x00000010
#define WINDOWS_SEND_TIMED_OUT		0x10000004

#define WINDOWS_MSG_SUCCESS 0

// This is the return value when windows_msg is called for receive, when the underlying pipe has just been connected to,
// and thus has no useful information.  If windows_msg returns this, runloop observers should conceptually do a continue.
// They'll be called on the next run of the runloop with good information.
#define WINDOWS_MSG_RCV_CONNECT -1

#define WINDOWS_SEND_MSG           0x00000001
#define WINDOWS_RCV_MSG            0x00000002

/*
 *  The timeout mechanism uses mach_msg_timeout_t values,
 *  passed by value.  The timeout units are milliseconds.
 *  It is controlled with the WINDOWS_SEND_TIMEOUT
 *  and WINDOWS_RCV_TIMEOUT options.
 */

typedef unsigned int windows_msg_timeout_t;

// NOTE WELL!!!  When calling this to receive a message, the return code is the value of the subsequent read
// on that port.  Do your cleanup, but be sure to forward your initial read as appropriate.
CF_EXPORT int windows_msg(
    windows_msg_header_t *msg,
    int                  option,
    unsigned int         send_size,
    unsigned int         *rcv_size,
    // Timeout is currently only used on sending messages
    windows_msg_timeout_t timeout);

#endif

#endif /* ! __COREFOUNDATION_CFWINDOWSMESSAGE__ */

