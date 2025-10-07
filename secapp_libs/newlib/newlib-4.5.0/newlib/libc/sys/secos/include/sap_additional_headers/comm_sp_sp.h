// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SP_COMM_ISP_H
#define SP_COMM_ISP_H

#define COMM_ISP_SHAREDMEM_STATUS_ESTABLISHED      (1)


#define SP_COMM_LOG	        LOG_INFO

#ifndef __ASSEMBLY__

#define SP_COMM_DEBUG(format, ...) \
	do{	\
		if(SP_COMM_LOG == LOG_DEBUG) {  \
		printf("[SecProcess-COMM-Debug] "format, ## __VA_ARGS__); } }while(0)

#define SP_COMM_INFO(format, ...) \
	do{	\
		if((SP_COMM_LOG >= LOG_INFO) && (SP_COMM_LOG != LOG_NONE)) {  \
		printf("[SecProcess-COMM-INFO] "format, ## __VA_ARGS__); } }while(0)

#define SP_COMM_ERR(format, ...) \
	do{	\
		if((SP_COMM_LOG >= LOG_ERR) && (SP_COMM_LOG != LOG_NONE)) {  \
		printf("[SecProcess-COMM-ERR] "format, ## __VA_ARGS__); } }while(0)

// The SecProcess's role in a Inter-SecProcess shared memory channel
typedef enum
{
	COMM_ISP_ROLE_INVALID = 0,
	COMM_ISP_ROLE_OWNER = 1,
	COMM_ISP_ROLE_USER = 2
} COMM_ISP_ROLE;

typedef struct _comm_isp
{
	shandle_t		h_comm;

	vaddr_t			comm_buf;
	size_t			comm_buf_size;

	COMM_ISP_ROLE	role;
	spid_t			owner_spid;
	spid_t			user_spid;

	/********* Data for Handling Accidentally Closed IPC Servers *********/
	// The code address of error handling code when the current active COMM_ISP_SHAREDMEM is accidentally closed by the IPC server
	void* 			buffer_access_err_handler;
} COMM_ISP_SHAREDMEM;

typedef struct
{
    uint32_t    is_response:1,
                _reserved1:31;

	uint32_t 	status;
} COMM_ISP_SHAREDMEM_ESTABLISH_RESPONSE;

/// @brief Create an Inter-SecProcess shared memory channel.
///
/// @return Return a valid pointer on succss.
///         Return NULL on errors.
extern COMM_ISP_SHAREDMEM* comm_isp_sharedmem_create(spid_t target_sp_id, uint32_t num_pages);

/// @brief The caller SecProcess accepts to establish the given Inter-SecProcess shared memory channel <h_comm> 
extern COMM_ISP_SHAREDMEM* comm_isp_sharedmem_accept(spid_t requester_spid, shandle_t h_comm);

/// @brief Close an Inter-SecProcess shared memory channel. 
extern void comm_isp_sharedmem_close(COMM_ISP_SHAREDMEM* comm);

/// @brief Issue a synchronous call to a SecProcess via messaging provided by SecOS.
/// SecProcess can issue a call only after the previous call is returned.
///
/// @param req_mailbox_response Set to true if the caller wants to receive a notification in its <sp_events> mailbox
extern SAP_STATUS comm_isp_msg_call(
    spid_t target_sp_id, 
    bool req_mailbox_response, 
    uint32_t sub_cmd, 
    void* in_data, 
    size_t in_data_sz,
    void* out_data, 
    size_t* out_data_size
);

/// @brief Send Inter-SecProcess call response to the caller via messaging provided by SecOS.
extern SAP_STATUS comm_isp_msg_send_response(spid_t target_sp_id, uint32_t req_msg_id, void* in_data, size_t in_data_sz);




/********* Management of All <COMM_ISP_SHAREDMEM> Info *********/
/// @brief Clear all record entries.
extern void comm_isp_sharedmem_mgr_clear(void);

/// @brief Register <comm> so the SecProcess can use a SecProcess ID to find it later.
extern SAP_STATUS comm_isp_sharedmem_register(COMM_ISP_SHAREDMEM* comm);
/// @brief Unregister <comm>.
extern void comm_isp_sharedmem_unregister(COMM_ISP_SHAREDMEM* comm);
/// @brief Return the <COMM_ISP_SHAREDMEM> whose owner or user SecProcess has the given <target_spid>.
extern COMM_ISP_SHAREDMEM* comm_isp_sharedmem_find(spid_t target_spid);

/// @brief Close all registered Inter-SecProcess shared memory channels.
extern void comm_isp_sharedmem_unregister_and_close_all(void);
/// @brief Close the given registered Inter-SecProcess shared memory channels.
extern void comm_isp_sharedmem_unregister_and_close(COMM_ISP_SHAREDMEM* comm);

/// @brief Read the content in the shared memory <comm> into the buffer <dest>.
/// [NOTE] Because IPC servers of shared memory allocate the memory, accidentally termination of an IPC server will cause its 
/// IPC clients encounter #PF because of the dangling memory references of the shared memory left in the IPC clients.
/// Thus, IPC clients should always use <comm_isp_read_buffer> and <comm_isp_write_buffer> to access shared memory and 
/// handle the #PF in an elegant manner.
///
/// @param comm 
/// @param dest 
/// @param offset 
/// @param size
/// @return Return SUCCESS if the buffer access is succeed.
///			Return EFAULT if the shared memory associated with the ISP channel <comm> is accidently freed.
///			Return EINVAL if any parameters or the role of the ISP channel <comm->role> are invalid
extern SAP_STATUS comm_isp_read_buffer(COMM_ISP_SHAREDMEM* comm, void* dest, offset_t offset, size_t size);

/// @brief Write the buffer <src> to the shared memory <comm>.
/// [NOTE] Because IPC servers of shared memory allocate the memory, accidentally termination of an IPC server will cause its 
/// IPC clients encounter #PF because of the dangling memory references of the shared memory left in the IPC clients.
/// Thus, IPC clients should always use <comm_isp_read_buffer> and <comm_isp_write_buffer> to access shared memory and 
/// handle the #PF in an elegant manner.
///
/// @param comm 
/// @param src 
/// @param offset 
/// @param size 
/// @return Return SUCCESS if the buffer access is succeed.
///			Return EFAULT if the shared memory associated with the ISP channel <comm> is accidently freed.
///			Return EINVAL if any parameters or the role of the ISP channel <comm->role> are invalid
extern SAP_STATUS comm_isp_write_buffer(COMM_ISP_SHAREDMEM* comm, void* src, offset_t offset, size_t size);

#endif  // __ASSEMBLY__
#endif // SP_COMM_ISP_H