#ifndef QAPI_SFS_API_SERVICE_01_H
#define QAPI_SFS_API_SERVICE_01_H
/**
  @file qapi_sfs_v01.h

  @brief This is the public header file which defines the qapi_sfs_api service Data structures.

  This header file defines the types and structures that were defined in
  qapi_sfs_api. It contains the constant values defined, enums, structures,
  messages, and service message IDs (in that order) Structures that were
  defined in the IDL as messages contain mandatory elements, optional
  elements, a combination of mandatory and optional elements (mandatory
  always come before optionals in the structure), or nothing (null message)

  An optional element in a message is preceded by a uint8_t value that must be
  set to true if the element is going to be included. When decoding a received
  message, the uint8_t values will be set to true or false by the decode
  routine, and should be checked before accessing the values that they
  correspond to.

  Variable sized arrays are defined as static sized arrays with an unsigned
  integer (32 bit) preceding it that must be set to the number of elements
  in the array that are valid. For Example:

  uint32_t test_opaque_len;
  uint8_t test_opaque[16];

  If only 4 elements are added to test_opaque[] then test_opaque_len must be
  set to 4 before sending the message.  When decoding, the _len value is set 
  by the decode routine and should be checked so that the correct number of
  elements in the array will be accessed.

*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  

  
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Mon Mar 20 2017 (Spin 0)
   From IDL File: qapi_sfs_v01.idl */

/** @defgroup qapi_sfs_api_qmi_consts Constant values defined in the IDL */
/** @defgroup qapi_sfs_api_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup qapi_sfs_api_qmi_enums Enumerated types used in QMI messages */
/** @defgroup qapi_sfs_api_qmi_messages Structures sent as QMI messages */
/** @defgroup qapi_sfs_api_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup qapi_sfs_api_qmi_accessor Accessor for QMI service object */
/** @defgroup qapi_sfs_api_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "qapi_swi_common_v01.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qapi_sfs_api_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define QAPI_SFS_API_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QAPI_SFS_API_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QAPI_SFS_API_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define QAPI_SFS_API_V01_MAX_MESSAGE_ID 0x000B
/**
    @}
  */


/** @addtogroup qapi_sfs_api_qmi_consts 
    @{ 
  */
#define MAX_PATH_SIZE_V01 1000
#define MAX_DATA_SIZE_V01 4096
#define SFS_CREATE_V01 0
#define SFS_APPEND_V01 1
#define SFS_READONLY_V01 2
#define SFS_READWRITE_V01 3
#define SFS_TRUNCATE_V01 4
#define SFS_SEEK_SOF_V01 0
#define SFS_SEEK_CURRENT_V01 1
#define SFS_SEEK_EOF_V01 2
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Request Message; This command is used to open an existing or create (depending on the input parameters)
    a SFS file. A SFS file must be stored in a SFS folder, so QMI_SWI_SFS_MKDIR must be called before
    creating this SFS file. Note that the file need to be closed if this file will not be used, this is to save 
    the resources of the system. */
typedef struct {

  /* Mandatory */
  /*  SFS path, The file will be created in sfs_usr_bck/path/xxx or sfs_usr/path/xxx in Qualcomm EFS depends on the path_type */
  char path[MAX_PATH_SIZE_V01];

  /* Mandatory */
  /*  SFS path type, valid value are 0, 1, 2. */
  int32_t path_type;
  /**<  
                                0x00 - the path is in backup folder sfs_usr_bck/path/xxx, the file will be backup if QMI_SWI_SFS_BACKUP is triggered
                                0x01 - the path isn't in backup folder sfs_usr/path/xxx, the file won't be backup if QMI_SWI_SFS_BACKUP is triggered
                                0x02 - path of AVMS credentials
                                */

  /* Mandatory */
  /*  method to open the file */
  int32_t mode;
  /**<  
                                0x00 - Create new file,Implies create, read, write, truncation
                                0x01 - Open a file for append access, Implies append, read, write
                                0x02 - Open a file for read-only access, Implies read
                                0x03 - Open a file for read-write access, Implies create, read, write
                                0x04 - Truncates file contents, Implies read, write, truncation
                                */
}swi_sfs_open_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Response Message; This command is used to open an existing or create (depending on the input parameters)
    a SFS file. A SFS file must be stored in a SFS folder, so QMI_SWI_SFS_MKDIR must be called before
    creating this SFS file. Note that the file need to be closed if this file will not be used, this is to save 
    the resources of the system. */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  int32_t fp;
  /**<  
                                0 - open file failed.
                                Other value -return file handler
                                */
}swi_sfs_open_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Request Message; This message informs the modem that close a SFS file in EFS */
typedef struct {

  /* Mandatory */
  /*  File handler to be closed */
  int32_t fp;
  /**<  
                                file handler
                                */
}swi_sfs_close_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Response Message; This message informs the modem that close a SFS file in EFS */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}swi_sfs_close_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Request Message; This message informs the modem that write to a open SFS file in EFS.
    The bytes are written to the current file position. If the APPEND flag was set 
    in sfs_open(), the bytes are written at the end of the file unless 
    sfs_seek() was issued before sfs_write(). In this case, the data is written to 
    the file position that sfs_seek() set. The file position advances by the number 
    of bytes written. It is permitted for efs_write to write fewer bytes than were requested,
    even if space is available.. */
typedef struct {

  /* Mandatory */
  /*  file handler to operate */
  int32_t fp;
  /**<  
                                file handler
                                */

  /* Mandatory */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[MAX_DATA_SIZE_V01];
  /**<  
                                         data buffer to be written
                                         */
}swi_sfs_write_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Response Message; This message informs the modem that write to a open SFS file in EFS.
    The bytes are written to the current file position. If the APPEND flag was set 
    in sfs_open(), the bytes are written at the end of the file unless 
    sfs_seek() was issued before sfs_write(). In this case, the data is written to 
    the file position that sfs_seek() set. The file position advances by the number 
    of bytes written. It is permitted for efs_write to write fewer bytes than were requested,
    even if space is available.. */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  int32_t bytes_written;
  /**<  
                                   Returns the number of bytes successfully written or -1 for an error
                                   */
}swi_sfs_write_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Request Message; This message informs the modem that read from an open SFS file in EFS.
    Zero is a valid result to indicate that the end of file has been reached.
    It is permitted for efs_read to return fewer bytes than were requested, even if the
    data is available in the file. */
typedef struct {

  /* Mandatory */
  int32_t fp;
  /**<  
                                file handler
                                */

  /* Mandatory */
  int32_t data_len;
  /**<  
                                file length to be read
                                */
}swi_sfs_read_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Response Message; This message informs the modem that read from an open SFS file in EFS.
    Zero is a valid result to indicate that the end of file has been reached.
    It is permitted for efs_read to return fewer bytes than were requested, even if the
    data is available in the file. */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  buffer to store the data */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[MAX_DATA_SIZE_V01];
  /**<  
                                         data buffer, return 0 bytes means that reach the end of the file
                                         */
}swi_sfs_read_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Request Message; This message informs the modem that seek from an open SFS file in EFS */
typedef struct {

  /* Mandatory */
  /*  File handler */
  int32_t fp;
  /**<  
                                file handler
                                */

  /* Mandatory */
  int32_t offset;

  /* Mandatory */
  /*  offset File offset to seek in bytes */
  int32_t whence;
  /**<  
                                0x00 ¨C Start of the file
                                0x01 ¨C Current file point
                                0x02 ¨C End of the file
                                */
}swi_sfs_seek_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Response Message; This message informs the modem that seek from an open SFS file in EFS */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  int32_t current_location;
  /**<   Returns the current location if successful, or -1 if an error occurred*/
}swi_sfs_seek_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Request Message; This message informs the modem to create a SFS directory. SFS directory must under a 
    normal directory, and the format must be "a/b/c/d...", a,b,c will be a normal directory, 
    and d is a secure folder. That means the root folder must be normal folder, and you cannot 
    create a SFS folder under another SFS folder, other limitations are found for this QMI:
      - Directories cannot contain both directories and files.
      - Directories cannot be listed or iterated over.
      - Directories cannot be deleted. */
typedef struct {

  /* Mandatory */
  /*  path to create directory */
  char path[MAX_PATH_SIZE_V01];
  /**<   path of the directory*/

  /* Mandatory */
  /*  SFS path type, valid value are 0 and 1. */
  int32_t path_type;
  /**<  
                                0x00 - the path is in backup folder sfs_usr_bck/path/xxx, the file will be backup if QMI_SWI_SFS_BACKUP is triggered
                                0x01 - the path isn't in backup folder sfs_usr/path/xxx, the file won't be backup if QMI_SWI_SFS_BACKUP is triggered
                                0x02 - path of AVMS credentials
                                */
}swi_sfs_mkdir_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Response Message; This message informs the modem to create a SFS directory. SFS directory must under a 
    normal directory, and the format must be "a/b/c/d...", a,b,c will be a normal directory, 
    and d is a secure folder. That means the root folder must be normal folder, and you cannot 
    create a SFS folder under another SFS folder, other limitations are found for this QMI:
      - Directories cannot contain both directories and files.
      - Directories cannot be listed or iterated over.
      - Directories cannot be deleted. */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  Result Code */
  int32_t result;
  /**<  
                                0 - directory is created successfully.
                                Other value -Error occurred
                                */
}swi_sfs_mkdir_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Request Message; This message informs the modem to remove a SFS file. This QMI is only used to remove files but not folders. */
typedef struct {

  /* Mandatory */
  /*  file path to be removed */
  char path[MAX_PATH_SIZE_V01];

  /* Mandatory */
  /*  SFS path type, valid value are 0, 1, 2. */
  int32_t path_type;
  /**<  
                                0x00 - the path is in backup folder sfs_usr_bck/path/xxx
                                0x01 - the path isn't in backup folder sfs_usr/path/xxx
                                0x02 - path of AVMS credentials
                                */
}swi_sfs_rm_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Response Message; This message informs the modem to remove a SFS file. This QMI is only used to remove files but not folders. */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  Result Code, only support remove file currently */
  int32_t result;
  /**<  
                                0 - file is removed successfully.
                                Other value -Error occurred
                                */
}swi_sfs_rm_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Request Message; This message informs the modem that get the size of an open sfs file */
typedef struct {

  /* Mandatory */
  /*  file handler */
  int32_t fp;
}swi_sfs_getsize_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Response Message; This message informs the modem that get the size of an open sfs file */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/

  /* Mandatory */
  /*  Result Code */
  int32_t result;
  /**<  
                                0 - operation is  successful.
                                Other value -Error occurred
                                */

  /* Mandatory */
  int32_t size;
  /**<   file size*/
}swi_sfs_getsize_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_sfs_backup_req_msg_v01;

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Response Message; This message informs the modem to backup all sfs files in sfs_usr_bck/ folder */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Result Code */
  int32_t result;
  /**<  
                                0 - operation is failed.
                                1 - operation is successful.
                                */
}swi_sfs_backup_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_sfs_restore_req_msg_v01;

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Response Message; This message informs the modem to restore all sfs files which backuped before */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  /*  Result Code */
  int32_t result;
  /**<  
                                0 - operation is failed.
                                1 - operation is successful.
                                */
}swi_sfs_restore_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}swi_sfs_get_space_info_req_msg_v01;

/** @addtogroup qapi_sfs_api_qmi_messages
    @{
  */
/** Response Message; This message gets the information of used memory and free memory of SFS */
typedef struct {

  /* Mandatory */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Mandatory */
  int32_t used_mem;
  /**<  
                                used memory of the SFS
                                */

  /* Mandatory */
  int32_t remaining_mem;
  /**<  
                                remaining memory of the SFS
                                */
}swi_sfs_get_space_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QMI_SWI_SFS_BACKUP_V01 
//#define REMOVE_QMI_SWI_SFS_CLOSE_V01 
//#define REMOVE_QMI_SWI_SFS_GETSIZE_V01 
//#define REMOVE_QMI_SWI_SFS_GET_SPACE_INFO_V01 
//#define REMOVE_QMI_SWI_SFS_MKDIR_V01 
//#define REMOVE_QMI_SWI_SFS_OPEN_V01 
//#define REMOVE_QMI_SWI_SFS_PING_V01 
//#define REMOVE_QMI_SWI_SFS_READ_V01 
//#define REMOVE_QMI_SWI_SFS_RESTORE_V01 
//#define REMOVE_QMI_SWI_SFS_RM_V01 
//#define REMOVE_QMI_SWI_SFS_SEEK_V01 
//#define REMOVE_QMI_SWI_SFS_WRITE_V01 

/*Service Message Definition*/
/** @addtogroup qapi_sfs_api_qmi_msg_ids
    @{
  */
#define QMI_SWI_SFS_PING_REQ_V01 0x0000
#define QMI_SWI_SFS_PING_RESP_V01 0x0000
#define QMI_SWI_SFS_PING_IND_V01 0x0000
#define QMI_SWI_SFS_OPEN_REQ_V01 0x0001
#define QMI_SWI_SFS_OPEN_RESP_V01 0x0001
#define QMI_SWI_SFS_CLOSE_REQ_V01 0x0002
#define QMI_SWI_SFS_CLOSE_RESP_V01 0x0002
#define QMI_SWI_SFS_WRITE_REQ_V01 0x0003
#define QMI_SWI_SFS_WRITE_RESP_V01 0x0003
#define QMI_SWI_SFS_READ_REQ_V01 0x0004
#define QMI_SWI_SFS_READ_RESP_V01 0x0004
#define QMI_SWI_SFS_SEEK_REQ_V01 0x0005
#define QMI_SWI_SFS_SEEK_RESP_V01 0x0005
#define QMI_SWI_SFS_MKDIR_REQ_V01 0x0006
#define QMI_SWI_SFS_MKDIR_RESP_V01 0x0006
#define QMI_SWI_SFS_RM_REQ_V01 0x0007
#define QMI_SWI_SFS_RM_RESP_V01 0x0007
#define QMI_SWI_SFS_GETSIZE_REQ_V01 0x0008
#define QMI_SWI_SFS_GETSIZE_RESP_V01 0x0008
#define QMI_SWI_SFS_BACKUP_REQ_V01 0x0009
#define QMI_SWI_SFS_BACKUP_RESP_V01 0x0009
#define QMI_SWI_SFS_RESTORE_REQ_V01 0x000A
#define QMI_SWI_SFS_RESTORE_RESP_V01 0x000A
#define QMI_SWI_SFS_GET_SPACE_INFO_REQ_V01 0x000B
#define QMI_SWI_SFS_GET_SPACE_INFO_RESP_V01 0x000B
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro qapi_sfs_api_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type qapi_sfs_api_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define qapi_sfs_api_get_service_object_v01( ) \
          qapi_sfs_api_get_service_object_internal_v01( \
            QAPI_SFS_API_V01_IDL_MAJOR_VERS, QAPI_SFS_API_V01_IDL_MINOR_VERS, \
            QAPI_SFS_API_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

