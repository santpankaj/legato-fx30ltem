/**
 * @file atForward.h
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef _ATFORWARD_H
#define _ATFORWARD_H

//--------------------------------------------------------------------------------------------------
/**
 * Helper to prepare the raw ATFWD REG request message
 */
//--------------------------------------------------------------------------------------------------
int qmi_atcop_reg_at_command_fwd_req_msg_helper
(
    qmi_atcop_at_cmd_fwd_req_type     *cmdFwdReq,
    unsigned char                     *msg,
    int                               *msgSize
);

//--------------------------------------------------------------------------------------------------
/**
 * Helper to prepare the raw ATFWD REG response message
 */
//--------------------------------------------------------------------------------------------------
int qmi_atcop_fwd_at_cmd_resp_helper
(
    qmi_atcop_fwd_resp_at_resp_type   *atResp,
    int                               *qmiErrCode,
    unsigned char                     *msg,
    int                               *msgSize
);

//--------------------------------------------------------------------------------------------------
/**
 * Helper to prepare the raw ATFWD URC request message
 */
//--------------------------------------------------------------------------------------------------
int qmi_atcop_fwd_at_urc_req_msg_helper
(
  qmi_atcop_at_fwd_urc_req_type     *urcFwdReq,
  unsigned char                     *msg,
  int                               *msgSize
);

//--------------------------------------------------------------------------------------------------
/**
 * Helper for processing the indication callback
 */
//--------------------------------------------------------------------------------------------------
void qmi_atcop_srvc_indication_cb_helper
(
    unsigned long         msgId,
    unsigned char         *rxMsgBuf,
    int                   rxMsgLen,
    qmi_atcop_indication_data_type    *indData;
);

#endif /* _ATFORWARD_H */
