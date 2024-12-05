/*******************************************************************************
 * @file     protocol.c
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
 * protocol.c
 *
 * Implements the Protocol state machine functions
 */
#include "vif_types.h"
#include "port.h"
#include "protocol.h"
#include "policy.h"
#include "observer.h"
#include "logging.h"
#define tProtocolTxTimeout    10 * kMSTimeFactor
#define PROTOCOL_RETRIES(rev) ((rev == 2) ? (2) : (3))

static void protocol_state_idle(struct port *port);

#if CONFIG_ENABLED(LOG)
static const char *const prot_string[] = {
#define STATE_STR(str) #str,
    CREATE_PROT_STATES(STATE_STR)};
#define protocol_set_state(port, state)                                                            \
    do                                                                                             \
    {                                                                                              \
        dbg_msg("%d:%s", port->dev->port_tcpd_id, prot_string[state]);                             \
        protocol_set_state_func(port, state);                                                      \
    } while (0)
#else
#define protocol_set_state(port, state) protocol_set_state_func(port, state)
#endif

static inline void protocol_set_state_func(struct port *port, enum protocol_state_t state)
{
    port->prl_state = state;
    port->event     = true;
}

/**
 * @brief reset the cable protocol variables
 */
static void protocol_reset_sop1(struct port *port)
{
    port->prl_tx_msgid[SOP_SOP1] = 0;
    port->prl_rx_msgid[SOP_SOP1] = 0xFF;
    port->prl_tx_msgid[SOP_SOP1] = 0;
    port->prl_rx_msgid[SOP_SOP1] = 0xFF;
}

/**
 * @brief reset the cable protocol variables
 */
static void protocol_reset_sop2(struct port *port)
{
    port->prl_tx_msgid[SOP_SOP2] = 0;
    port->prl_rx_msgid[SOP_SOP2] = 0xFF;
    port->prl_tx_msgid[SOP_SOP2] = 0;
    port->prl_rx_msgid[SOP_SOP2] = 0xFF;
}

/**
 * @brief reset the sop protocol variables
 */
static void protocol_reset_sop0(struct port *port)
{
    port->prl_tx_msgid[SOP_SOP0] = 0;
    port->prl_rx_msgid[SOP_SOP0] = 0xFF;
    port->prl_tx_msgid[SOP_SOP0] = 0;
    port->prl_rx_msgid[SOP_SOP0] = 0xFF;
}

void protocol_reset_sop(struct port *port, unsigned mask)
{
    if (mask & SOP_MASK_SOP0)
    {
        protocol_reset_sop0(port);
    }
    if (mask & SOP_MASK_SOP1)
    {
        protocol_reset_sop1(port);
    }
    if (mask & SOP_MASK_SOP2)
    {
        protocol_reset_sop2(port);
    }
}

void reset_protocol_variables(struct port * port)
{
    protocol_set_state(port, PRLIdle);
    port->prl_tx_status       = txIdle;
    port->pd_tx_flag          = false;
    port->prl_msg_rx          = false;
    port->prl_msg_rx_sop      = SOP_SOP0;
    port->prl_msg_tx_sop      = SOP_SOP0;
    port->protocol_use_sinktx = false;
    timer_disable(&port->timers[PROT_TIMER]);
}
void protocol_reset(struct port *port)
{
	reset_protocol_variables(port);
    if (CONFIG_ENABLED(EXTMSG))
    {
        port->prl_ext_num_bytes  = 0;
        port->prl_ext_chunk_num  = 0;
        port->prl_ext_req_chunk  = 0;
        port->prl_ext_send_chunk = false;
        port->prl_msg_rx_pending = false;
        port->prl_ext_req_cmd    = 0;
    }

    protocol_reset_sop(port, SOP_MASK_SOP0 | SOP_MASK_SOP1 | SOP_MASK_SOP2);
}

static void protocol_check_rx_hardreset(struct port *port)
{
    if (fusbdev_tcpd_rx_hardreset(port->tcpd_device))
    {
        policy_receive_hardreset(port);
        protocol_reset(port);
        event_notify(EVENT_HARD_RESET, port->dev, 0);
        port->is_hard_reset   = true;
        port->policy_substate = 0;
    }
}

static inline bool protocol_check_ext_msg_completion(struct port *port)
{
    unsigned status = true;
    if (port->prl_ext_num_bytes > port->prl_ext_chunk_num * MAX_EXT_MSG_LEGACY_LEN || port->prl_msg_rx_pending)
    {
        port->prl_tx_status = txBusy;
        status              = false;
    }
    else
    {
        port->prl_ext_state_active = false;
    }
    return status;
}

void protocol_tx_hardreset(struct port *port, enum sop_t sop)
{
    port->prl_tx_status = txBusy;
    protocol_set_state(port, PRLResetWait);
    fusbdev_tcpd_tx_hardreset(port->tcpd_device, sop);
    timer_start(&port->timers[PD_RSP_TIMER], ktHardResetComplete);
}

static void protocol_rx_packet(struct port *port)
{
    unsigned i = 0;
    uint16_t buf[15];

    struct pd_header_t *pdHeader = (struct pd_header_t *)&buf[0];
    uint8_t            *pData    = (uint8_t *)&buf[1];
    struct pd_header_t *extHdr   = (struct pd_header_t *)&buf[1];
    uint8_t            *pExtData = (uint8_t *)&buf[2];

    port->prl_msg_rx_sop     = fusbdev_tcpd_pd_rx(port->tcpd_device, (uint8_t *)buf, sizeof(buf));
    port->prl_rx_header.word = pdHeader->word;

    if (port->prl_rx_header.sop.NumDataObjects == 0
        && port->prl_rx_header.sop.MessageType == CMTGoodCRC)
    {
        /* Skip over the GoodCRC */
        return;
    }
    else if (port->prl_rx_header.sop.NumDataObjects == 0
             && port->prl_rx_header.sop.MessageType == CMTSoftReset)
    {
        protocol_reset_sop(port, SOP_MASK(port->prl_msg_rx_sop));
    }
    else if (port->prl_rx_header.sop.MessageID != port->prl_rx_msgid[port->prl_msg_rx_sop])
    {
        port->prl_rx_msgid[port->prl_msg_rx_sop] = port->prl_rx_header.sop.MessageID;
    }
    else
    {
        /* Drop anything else - possible retried message with same ID */
        return;
    }

    if (port->prl_rx_header.sop.NumDataObjects > 0)
    {
        /* Data message */
        if (CONFIG_ENABLED(EXTMSG) && port->vif->Chunking_Implemented_SOP
            && port->prl_rx_header.sop.Extended == 1)
        {
            /* Extended length message */
            if (extHdr->ext.Chunked)
            {
                if (extHdr->ext.RequestChunk == 1)
                {
                    /* Rec'd a request - tell the tx function to continue */
                    port->prl_ext_send_chunk = true;
                }
                else
                {
                    port->prl_ext_req_chunk    = true;
                    port->prl_ext_state_active = true;
                    port->prl_msg_rx_pending = true;
                    if (extHdr->ext.ChunkNumber == 0)
                    {
                        /* First Chunk */
                        port->prl_ext_chunk_num = 0;
                        port->prl_ext_chunk_num = 0;
                        port->prl_ext_num_bytes = 0;
                        port->prl_ext_buf[0]    = extHdr->byte[0];
                        port->prl_ext_buf[1]    = extHdr->byte[1];
                    }
                    if (port->prl_ext_chunk_num == extHdr->ext.ChunkNumber)
                    {
                        i = 0;
                        while (i < MAX_EXT_MSG_LEGACY_LEN
                               && port->prl_ext_num_bytes < extHdr->ext.DataSize)
                        {
                            port->prl_ext_buf[sizeof(*extHdr) + port->prl_ext_num_bytes++] =
                                pExtData[i++];
                        }
                        if (port->prl_ext_num_bytes >= extHdr->ext.DataSize)
                        {
                            /* Done */
                            port->prl_ext_req_chunk    = false;
                            port->prl_ext_state_active = false;
                            port->prl_tx_status        = txIdle;
                            port->prl_msg_rx           = true;
                            port->prl_msg_rx_pending = false;
                        }
                        else
                        {
                            if (port->prl_ext_req_chunk)
                            {
                                port->prl_ext_chunk_num += 1;
                                port->prl_ext_req_cmd = port->prl_rx_header.sop.MessageType;
                            }
                        }
                    }
                }
            }
            else
            {
                /* TODO: Handle unchunked */
            }
        }
        else
        {
            /* All other data message */
            for (i = 0; i < port->prl_rx_header.sop.NumDataObjects; i++)
            {
                port->prl_rx_data[i].byte[0] = *pData++;
                port->prl_rx_data[i].byte[1] = *pData++;
                port->prl_rx_data[i].byte[2] = *pData++;
                port->prl_rx_data[i].byte[3] = *pData++;
            }

            /*
			Reset extended message flags upon receiving a non-extended message
			if in a extended message state
			*/
            if (port->prl_ext_state_active)
            {
                port->prl_ext_req_chunk    = false;
                port->prl_ext_state_active = false;
            }

            /* Set the flag to pass the message to the policy engine */
            port->prl_msg_rx = true;
        }
    }
    else
    {
        port->prl_msg_rx = true;
    }
}

static void protocol_tx_packet(struct port *port)
{
    unsigned i       = 0;
    uint32_t txLen   = 0;
    uint16_t buf[15] = {0};
    uint8_t  retry   = 0;

    uint8_t            *pTxData  = (uint8_t *)&buf[1];
    uint8_t            *pExtData = (uint8_t *)&buf[2];
    struct pd_header_t *txHdr    = (struct pd_header_t *)&buf;
    struct pd_header_t *extHdr   = (struct pd_header_t *)&buf[1];

    txHdr->word = port->prl_tx_header.word;

    if ((txHdr->sop.NumDataObjects == 0) && (txHdr->sop.MessageType == CMTSoftReset))
    {
        protocol_reset_sop(port, SOP_MASK(port->prl_msg_rx_sop));
    }

    /* Update the tx message id to send */
    txHdr->sop.MessageID = port->prl_tx_msgid[port->prl_msg_tx_sop];

    if (CONFIG_ENABLED(EXTMSG) && port->vif->Chunking_Implemented_SOP
        && port->prl_ext_req_chunk)
    {
        port->prl_ext_req_chunk   = false;
        port->prl_msg_tx_sop      = port->prl_msg_rx_sop;
        /* Update sop when replying to chunk as sop1 / sop2 messages exist */
        /* and after tx sop1 and rx chunk 0, the messageID on SOP1 is incremented  */
        /* to fix this, update the messageID once more after correcting sop */
        txHdr->sop.MessageID = port->prl_tx_msgid[port->prl_msg_tx_sop];
        txHdr->sop.Extended       = true;
        txHdr->sop.NumDataObjects = 1;
        txHdr->sop.PortPowerRole  = port->policy_is_source;
        txHdr->sop.MessageType    = port->prl_ext_req_cmd & PDMsgTypeMask;
        txHdr->sop.SpecRevision   = port_pd_ver(port, port->prl_msg_tx_sop);
        if (port->prl_msg_tx_sop == SOP_SOP0)
        {
            /* This field is reserved for SOP', SOP'' */
            txHdr->sop.PortDataRole = port->policy_is_dfp;
        }
        /* Finish extended header */
        extHdr->word             = 0;
        extHdr->ext.Chunked      = 1;
        extHdr->ext.RequestChunk = 1;
        extHdr->ext.ChunkNumber  = port->prl_ext_chunk_num;
        txLen += sizeof(struct pd_header_t);
        /* Pad zeros */
        *pExtData++ = 0;
        txLen++;
        *pExtData++ = 0;
        txLen++;
    }
    else if (CONFIG_ENABLED(EXTMSG) && port->vif->Chunking_Implemented_SOP
             && port->prl_ext_send_chunk && txHdr->sop.Extended)
    {
        port->prl_ext_send_chunk = false;
        extHdr->word             = 0;
        extHdr->ext.Chunked      = 1;
        extHdr->ext.ChunkNumber  = port->prl_ext_chunk_num;
        extHdr->ext.DataSize     = port->prl_ext_num_bytes;
        txLen = (extHdr->ext.DataSize - (extHdr->ext.ChunkNumber * MAX_EXT_MSG_LEGACY_LEN));
        if (txLen > MAX_EXT_MSG_LEGACY_LEN)
        {
            txLen = MAX_EXT_MSG_LEGACY_LEN;
        }
        /* Copy the extended message  */
        for (i = 0; i < txLen; i++)
        {
            *pExtData++ = port->prl_ext_buf[i + extHdr->ext.ChunkNumber * MAX_EXT_MSG_LEGACY_LEN];
        }
        /* Two bytes extended header need to be added */
        txLen += sizeof(struct pd_header_t);
        /* Pad with 0's if necessary */
        while (txLen & 0x3)
        {
            *pExtData++ = 0;
            txLen++;
        }
        txHdr->sop.NumDataObjects = txLen / 4;
        port->prl_ext_chunk_num += 1;
    }
    else
    {
        for (i = 0; i < txHdr->sop.NumDataObjects; i++)
        {
            /* Load the data objects */
            *pTxData++ = port->prl_tx_data[i].byte[0];
            *pTxData++ = port->prl_tx_data[i].byte[1];
            *pTxData++ = port->prl_tx_data[i].byte[2];
            *pTxData++ = port->prl_tx_data[i].byte[3];
        }
        txLen = txHdr->sop.NumDataObjects * 4;
    }
    /* Add Pd header */
    txLen += sizeof(struct pd_header_t);
    /* Move on to waiting for a success or fail */
    port->prl_tx_status = txBusy;
    retry               = PROTOCOL_RETRIES(port_pd_ver(port, port->prl_msg_rx_sop));
    if (fusbdev_tcpd_pd_tx(port->tcpd_device, (uint8_t *)buf, txLen, port->prl_msg_tx_sop, retry))
    {
        protocol_set_state(port, PRLTxSendingMessage);
        /* Timeout specifically for chunked messages, but used
         * with each transmit to prevent a theoretical protocol hang.
         */
        timer_start(&port->timers[PROT_TIMER], ktChunkSenderRequest);

        /*is this a necessary check?/*/
    }
    else
    {
        port->prl_tx_status = txAbort;
        protocol_set_state(port, PRLIdle);
    }
}

static inline void protocol_copy_extended_message(struct port *port, uint8_t *data, uint32_t len)
{
    unsigned           i      = 0;
    struct pd_header_t exthdr = {0};
    /* Extended Data Message */
    if (CONFIG_ENABLED(EXTMSG) && port->vif->Chunking_Implemented_SOP)
    {
        port->prl_ext_chunk_num    = 0;
        port->prl_ext_send_chunk   = true;
        port->prl_ext_state_active = true;
        port->prl_ext_num_bytes    = len;
        if (port->prl_ext_num_bytes > MAX_EXT_MSG_LEN)
        {
            port->prl_ext_num_bytes = MAX_EXT_MSG_LEN;
        }
        for (i = 0; i < port->prl_ext_num_bytes; i++)
        {
            port->prl_ext_buf[i] = data[i];
        }
    }
    else
    {
        /* If chunking is not implemented then just insert then try sending
         * first chunk of extended message as non-extended message. 2 byte
         * Ext header plus 26 byte data message */
        if (len > MAX_EXT_MSG_LEGACY_LEN)
        {
            len = MAX_EXT_MSG_LEGACY_LEN;
        }
        exthdr.word         = 0;
        exthdr.ext.Chunked  = 1;
        exthdr.ext.DataSize = len;
        uint8_t *buf        = (uint8_t *)port->prl_tx_data[0].byte;
        i                   = 0;
        buf[i++]            = exthdr.byte[0];
        buf[i++]            = exthdr.byte[1];
        while (i < len + 2)
        {
            buf[i] = data[i - 2];
            i++;
        }
        /* Align to 4 byte boundary */
        while ((i & 0x3) && i < 28)
        {
            buf[i++] = 0;
        }
        port->prl_tx_header.sop.NumDataObjects = i / 4;
    }
}

static inline void protocol_copy_data_message(struct port *port, uint8_t *data, uint32_t len)
{
    unsigned i = 0;
    if (len > MAX_MSG_LEGACY_LEN)
    {
        len = MAX_MSG_LEGACY_LEN;
    }
    port->prl_tx_header.sop.NumDataObjects = len / 4;
    for (i = 0; i < port->prl_tx_header.sop.NumDataObjects; i++)
    {
        port->prl_tx_data[i].byte[0] = data[0 + i * 4];
        port->prl_tx_data[i].byte[1] = data[1 + i * 4];
        port->prl_tx_data[i].byte[2] = data[2 + i * 4];
        port->prl_tx_data[i].byte[3] = data[3 + i * 4];
    }
}

int protocol_send(struct port *port, struct protocol_tx_t *tx)
{
    int status = STAT_BUSY;
    /* Shortcut for hard reset */
    if (port->prl_tx_status == txIdle)
    {
        if (tx->len == 0 && tx->type == CMTHardreset)
        {
            port->prl_msg_tx_sop = tx->sop;
            protocol_tx_hardreset(port, port->prl_msg_tx_sop);
            return STAT_BUSY;
        }
        /* Construct header */
        port->prl_tx_header.word               = 0x0;
        port->prl_tx_header.sop.NumDataObjects = tx->len / 4;
        port->prl_tx_header.sop.MessageType    = tx->type & PDMsgTypeMask;
        port->prl_tx_header.sop.SpecRevision   = port_pd_ver(port, tx->sop);
        if (tx->sop == SOP_SOP0)
        {
            port->prl_tx_header.sop.PortDataRole  = port->policy_is_dfp;
            port->prl_tx_header.sop.PortPowerRole = port->policy_is_source;
        }
        port->prl_tx_header.sop.Extended = tx->ext;
        port->prl_msg_tx_sop             = tx->sop;

        if (tx->len > 0)
        {
            if (tx->ext)
            {
                protocol_copy_extended_message(port, tx->data, tx->len);
            }
            else
            {
                protocol_copy_data_message(port, tx->data, tx->len);
            }
        }

        if (port->policy_is_source && tx->ams)
        {
            /* AMS required */
            if (dev_tcpd_port_set_sinktx(port, SinkTxNG))
            {
                timer_start(&port->timers[SINKTX_TIMER], ktSinkTx);
            }
        }

        port->prl_tx_status = txPending;
    }

    /*message to be sent waiting for the SinkTxNG timer to expire*/
    if (port->prl_tx_status == txPending)
    {
        if (timer_expired(&port->timers[SINKTX_TIMER]))
        {
            timer_disable(&port->timers[SINKTX_TIMER]);
            port->prl_tx_status = txSend;
        }
    }

    /*sinktxng timer expired and message can now to be sent*/
    if (port->prl_tx_status == txSend)
    {
        if (port->prl_state == PRLIdle)
        {
            protocol_state_idle(port);
        }

        /*PD data transmit failed, or goodCRC not received */
    }
    if (port->prl_tx_status == txError)
    {
        port->prl_tx_status = txIdle;
        status              = STAT_ERROR;

        /* txAbort conditions
	 * a) PD message is received while transmitting a extended length unchunked message
	 * b) PD message is received before tx data is written into the tx buffer
	 * c) PD message is received while waiting for SinkTxOK
	 *
	 * txCollision condition
	 * a) PD message transmit is aborted due to collision indicated through hardware interrupt
	 */
    }
    else if ((port->prl_tx_status == txAbort) || (port->prl_tx_status == txCollision))
    {
        /* Unread message PD message interrupt current tx */
        if (CONFIG_ENABLED(EXTMSG) && tx->ext)
        {
            port->prl_ext_state_active = false;
        }
        port->prl_tx_status = txIdle;
        status              = STAT_ABORT;

        /*PD data transmit is successful */
    }
    else if (port->prl_tx_status == txSuccess)
    {
        /* GoodCRC response received */
        port->prl_tx_status = txIdle;
        status              = STAT_SUCCESS;
    }

    return status;
}

enum sop_t protocol_rx_header(struct port *port, struct pd_header_t *header)
{
    header->word = port->prl_rx_header.word;
    return port->prl_msg_rx_sop;
}

enum sop_t protocol_receive(struct port *port, struct pd_header_t *header, struct pd_msg_t *pdo,
                            unsigned long len)
{
    unsigned           i      = 0;
    struct pd_header_t extHdr = {0};
    header->word              = port->prl_rx_header.word;

    if (CONFIG_ENABLED(EXTMSG) && port->vif->Chunking_Implemented_SOP && header->sop.Extended)
    {
        /* If Chunking is supported then copy extended buffer */
        extHdr.byte[0] = port->prl_ext_buf[0];
        extHdr.byte[1] = port->prl_ext_buf[1];
        pdo->byte[0]   = port->prl_ext_buf[0];
        pdo->byte[1]   = port->prl_ext_buf[1];
        for (i = 0; i < extHdr.ext.DataSize && i < len; i++)
        {
            pdo->byte[i + 2] = port->prl_ext_buf[i + 2];
        }
    }
    else
    {
        for (i = 0; i < header->sop.NumDataObjects && i < len; i++)
        {
            pdo[i].object = port->prl_rx_data[i].object;
        }
    }

    return port->prl_msg_rx_sop;
}

static void protocol_state_idle(struct port *port)
{
    if (fusbdev_tcpd_pd_check_rx(port->tcpd_device))
    {
        /* If we have received a message */
        protocol_rx_packet(port);
        dbg_msg("rx msg %x", port->prl_rx_header.word);
        if (CONFIG_ENABLED(EXTMSG) && port->prl_ext_state_active)
        {
            if (port->prl_ext_req_chunk || port->prl_ext_send_chunk)
            {
                port->prl_tx_status = txSend;
            }
            else
            {
                /* rx new message during transmitting ext message */
                port->prl_tx_status = txAbort;
            }
        }
    }
    /*check if new message is received*/
    if (port->prl_msg_rx)
    {
        /*check if message is about to be sent and an rx data is receive*/
        if ((port->prl_tx_status == txSend) ||
            /*or a message is about to be sent but still waiting for timer to expire
			 * and rx data is received * */
            (port->prl_tx_status == txPending))
        {
            port->prl_tx_status = txAbort;
            protocol_set_state(port, PRLIdle);
        }
    }
    else
    {
        if (port->protocol_use_sinktx == true)
        {
            protocol_set_state(port, PRLSinkTx);
        }
        else
        {
            /*check if ready to send*/
            if (port->prl_tx_status == txSend)
            {
                protocol_tx_packet(port);
            }
        }
    }
}

static void protocol_state_tx_wait(struct port *port)
{
    /* Wait on CC == TxOK before transmitting */
    if (dev_tcpd_port_sinktx_state(port) == SinkTxOK)
    {
        port->protocol_use_sinktx = false;
        /* Initiate the transmission */
        protocol_tx_packet(port);
    }
    else if (fusbdev_tcpd_pd_check_rx(port->tcpd_device))
    {
        /* Interrupted by an incoming message */
        port->prl_tx_status = txAbort;
        protocol_set_state(port, PRLIdle);
    }
}

static void protocol_state_resetwait(struct port *port)
{
    enum port_tx_stat_t txstat = fusbdev_tcpd_port_tx_status(port->tcpd_device);

    /* Hard reset sent interrupt signaled by I_TX_Success */
    if (txstat == TX_SUCCESS || timer_expired(&port->timers[PD_RSP_TIMER]))
    {
        protocol_set_state(port, PRLIdle);
        port->prl_tx_status = txSuccess;
        event_notify(EVENT_HARD_RESET, port->dev, 0);
        timer_disable(&port->timers[PD_RSP_TIMER]);
    }
}

static void protocol_state_sending_message(struct port *port)
{
    enum sop_t          sop    = port->prl_msg_tx_sop;
    enum port_tx_stat_t txstat = fusbdev_tcpd_port_tx_status(port->tcpd_device);

    if (txstat == TX_SUCCESS)
    {
        dbg_msg("%d:tx_success", port->dev->port_tcpd_id);
        /* Transmission successful */
        if (port->prl_ext_state_active)
        {
        	if (protocol_check_ext_msg_completion(port))
        	{
        		port->prl_tx_status = txSuccess;
        	}
        }
        else
        {
        	port->prl_tx_status = txSuccess;
        }
        protocol_set_state(port, PRLIdle);
        port->prl_tx_msgid[sop] = (port->prl_tx_msgid[sop] + 1) & 0x7;
        timer_disable(&port->timers[PROT_TIMER]);
    }
    else if (txstat == TX_DISCARD)
    {
        dbg_msg("%d:tx_discard", port->dev->port_tcpd_id);
        /* Transmission interrupted due to collision*/
        port->prl_tx_status = txCollision;
        protocol_set_state(port, PRLIdle);
        port->prl_tx_msgid[sop] = (port->prl_tx_msgid[sop] + 1) & 0x7;
        timer_disable(&port->timers[PROT_TIMER]);
    }
    else if (txstat == TX_FAILED)
    {
        dbg_msg("%d:tx_error", port->dev->port_tcpd_id);
        /* Transmission failed */
        port->prl_tx_status = txError;
        protocol_set_state(port, PRLIdle);
        port->prl_tx_msgid[sop] = (port->prl_tx_msgid[sop] + 1) & 0x7;
        timer_disable(&port->timers[PROT_TIMER]);
    }
}

void protocol_engine(struct port *port)
{
    protocol_check_rx_hardreset(port);
    switch (port->prl_state)
    {
        case PRLResetWait:
            /* Waiting for the reset signaling to complete */
            protocol_state_resetwait(port);
            break;
        case PRLRxWait:
            break;
        case PRLIdle:
            /* Waiting to send or receive a message */
            protocol_state_idle(port);
            break;
        case PRLSinkTx:
            /* Waiting on Sink TxOK to transmit message */
            protocol_state_tx_wait(port);
            break;
        case PRLTxSendingMessage:
            /* We have attempted to transmit and are waiting for it to */
            /* complete or detect a collision or failure */
            protocol_state_sending_message(port);
            break;
        case PRLDisabled:
            /* In the disabled state, don't do anything */
            break;
        default:
            break;
    }
}
