/*******************************************************************************
 * @file     protocol.h
 * @author   USB PD Firmware Team
 *
 * @copyright @parblock
 * Copyright &copy; 2020 ON Semiconductor &reg;. All rights reserved.
 *
 * This software and/or documentation is licensed by ON Semiconductor under
 * limited terms and conditions. The terms and conditions pertaining to the
 * software and/or documentation are available at [ONSMI_T&C.pdf]
 * (http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf)
 *
 * ("ON Semiconductor Standard Terms and Conditions of Sale,
 *   Section 8 Software").
 *
 * DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
 * READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
 * SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS AND CONDITIONS.
 * @endparblock
 ******************************************************************************/
/*
 * Defines the PD Protocol state machine functions
 */
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include "dev_tcpd.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define PROTOCOL_RETRIES(rev) ((rev == 2) ? (2) : (3))

    /**
 * Defines the Message from Policy to Protocol
 */
    struct protocol_tx_t
    {
        enum sop_t    sop;
        unsigned long len;
        bool          ext;
        bool          ams;
        uint8_t      *data;
        uint8_t       type;
    };

    /**
 * @brief initialize protocol variables and sop0, sop1, sop2 to default
 */
    void protocol_reset(struct port *);
    
    /**
     * @brief reset protocol variables to default, not including any sop0/1/2 variables
     */
    void reset_protocol_variables(struct port * port);

    /**
 * @brief Executes the statemachine for protocol layer
 * @param[in] port
 */
    void protocol_engine(struct port *);

    /**
 * @brief Executes the statemachine for protocol layer.
 * Called after soft reset or hard reset.
 * @param[in] port
 * @param[in] mask
 */
    void protocol_reset_sop(struct port *, unsigned);

    /**
 * @brief
 */
    int protocol_send(struct port *, struct protocol_tx_t *);

    /**
 * @brief  Get the message header of the current received message.
 * @param port
 * @param header
 * @return
 */
    enum sop_t protocol_rx_header(struct port *, struct pd_header_t *);

    /**
 * @brief Read n bytes for received message buffer.
 * @param port
 * @param header
 * @param pdo
 * @return
 */

    /**
 * @brief Get the message header of the current received message and
 * current message. Check the PD header to know how many bytes has been
 * received.
 */
    enum sop_t protocol_receive(struct port *, struct pd_header_t *, struct pd_msg_t *,
                                unsigned long);

#ifdef __cplusplus
}
#endif

#endif /* _PROTOCOL_H_ */
