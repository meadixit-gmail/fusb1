#include "firmware_update.h"
#include "flash_control.h"
#include "pd_types.h"

#define NVR2_ADDRESS 0x08000200
#define NVR_BOOT_KEY 0xB002C0DE

extern uint32_t __ApplicationStart;

#define FLASH_BOOT_OFFSET      (uint32_t) & __ApplicationStart
#define PROGRAM_START_ADDRESS  FLASH_BOOT_OFFSET
#define PROGRAM_END_ADDRESS    0x00021000
#define PROGRAM_MAX_IMAGE_SIZE (PROGRAM_END_ADDRESS - PROGRAM_START_ADDRESS)

#define DATA_BLOCK_BYTE_SIZE 256
#define MSG_BUFFER_BYTE_SIZE 270
#define HASH_SIZE            256

#define DATA_PAYLOAD_BYTE_OFFSET 10

#define PROGRAM_WAIT 1
#define SIG_WAIT     100

#define FLASH_CHUNK_ADDRESS(x)   (x * DATA_BLOCK_BYTE_SIZE + FLASH_BOOT_OFFSET)
#define BYTE_TO_WORD(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

#define RESET_VECTKEY 0x05FA0004

enum msg_type
{
    PDFU_DATA_REQ,
    PDFU_VALIDATE,
    PDFU_INITIATE,
    PDFU_UNEXPECTED
};

static uint8_t pdfu_transmit_pd(uint8_t message_type, uint8_t *data, uint16_t num_bytes,
                                uint8_t *msg_id);

/* TODO: temp hash functions */
static void hash_init(uint8_t *base)
{
    uint16_t i;

    for (i = 0; i < HASH_SIZE; i++)
    {
        base[i] = 0;
    }
}

static void hash(uint8_t *base, uint8_t *in, uint16_t len, uint8_t *out)
{
    uint16_t i;

    for (i = 0; i < HASH_SIZE; i++)
    {
        out[i] = in[i];
    }
}

static void pdfu_init_pd(void)
{
    Clock_Management_Unit->PCLK_FEN |= Clock_Management_Unit_PCLK_FEN_USBPDA_HSCLK_EN_Msk;
    USB_Type_C_PD_Port_A->USBPD_CONFIG |= USB_Type_C_PD_Port_USBPD_CONFIG_EN_PD_Msk
                                          | USB_Type_C_PD_Port_USBPD_CONFIG_EN_SOP_Msk
                                          | USB_Type_C_PD_Port_USBPD_CONFIG_EN_HRD_RST_Msk;
}

static uint8_t pdfu_receive_pd(uint8_t *buffer)
{
    uint8_t  receive    = 0;
    uint16_t i          = 0;
    uint32_t error_mask = USB_Type_C_PD_Port_USB_INTERRUPT_I_rx_badcrc_Msk
                          | USB_Type_C_PD_Port_USB_INTERRUPT_I_rx_overflow_Msk
                          | USB_Type_C_PD_Port_USB_INTERRUPT__I_rx_discard_Msk
                          | USB_Type_C_PD_Port_USB_INTERRUPT_I_rx_hard_rst_Msk;
    uint32_t interrupt;
    uint32_t data;

    while (!receive)
    {
        interrupt = USB_Type_C_PD_Port_A->USB_INTERRUPT;
        USB_Type_C_PD_Port_A->USB_INTERRUPT &= interrupt;
        if (interrupt & error_mask)
        {
            //it went to hell - do something
            receive = 2;
        }
        if (interrupt & USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_success_Msk)
        {
            //return the data
            receive = 1;
        }
        if (interrupt & USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_buf_rdy_Msk)
        {
            data        = USB_Type_C_PD_Port_A->USBPD_DATA_RX;
            buffer[i++] = data;
            buffer[i++] = data >> 8;
            buffer[i++] = data >> 16;
            buffer[i++] = data >> 24;
        }
    }

    return receive;
}

static uint8_t pdfu_receive_message(uint8_t *buffer, uint8_t *msg_id)
{
    uint8_t     receive = 0;
    uint8_t     chunk   = 1;
    uint8_t     index   = 6;
    uint8_t     chunked_buffer[36];
    uint16_t    bytes_remaining;
    uint8_t     bytes_received;
    PD_HEADER_T header;
    PD_HEADER_T ext_tx_header;
    uint8_t     i;
    uint8_t     ret;

    /* Receive message */
    ret = pdfu_receive_pd(buffer);
    if (ret == 2)
    {
        return 2;
    }

    header.byte[0]  = buffer[4];
    header.byte[1]  = buffer[5];
    bytes_remaining = header.ext.DataSize;

    while (!receive)
    {
        /* If chunked + need more, request next chunk */
        if (header.ext.Chunked && bytes_remaining > MAX_EXT_MSG_LEGACY_LEN)
        {
            ext_tx_header.ext.ChunkNumber  = chunk++;
            ext_tx_header.ext.Chunked      = 1;
            ext_tx_header.ext.DataSize     = 0;
            ext_tx_header.ext.RequestChunk = 1;
            ext_tx_header.ext.Reserved     = 0;

            chunked_buffer[0] = ext_tx_header.byte[0];
            chunked_buffer[1] = ext_tx_header.byte[1];
            chunked_buffer[2] = 0;
            chunked_buffer[3] = 0;

            pdfu_transmit_pd(EMTFWUpdateRequest, chunked_buffer, 6, msg_id);
            pdfu_receive_pd(chunked_buffer);

            /* Append received data to buffer */
            header.byte[0] = chunked_buffer[0];
            header.byte[1] = chunked_buffer[1];
            bytes_received = header.sop.NumDataObjects * 4 - 2;
            bytes_remaining -= bytes_received;
            index += MAX_EXT_MSG_LEGACY_LEN;
            for (i = 0; i < bytes_received; i++)
            {
                buffer[index + i] = chunked_buffer[6 + i];
            }
        }
        else
        {
            receive = 1;
        }
    }

    return receive;
}

static uint8_t pdfu_transmit_pd(uint8_t message_type, uint8_t *data, uint16_t num_bytes,
                                uint8_t *msg_id)
{
    uint8_t  transmit     = 0;
    uint8_t  num_data_obj = 0;
    uint16_t i            = 0;
    uint32_t error_mask   = USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_fail_Msk
                          | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_discard_Msk
                          | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_buferror_Msk
                          | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_hard_rst_Msk;
    uint32_t    interrupt;
    PD_HEADER_T ext_header;
    PD_HEADER_T header;

    ext_header.byte[0] = data[0];
    ext_header.byte[1] = data[1];

    header.word              = 0;
    header.sop.MessageType   = message_type;
    header.sop.PortDataRole  = 1;
    header.sop.SpecRevision  = 2;
    header.sop.PortPowerRole = 1;
    header.sop.MessageID     = *msg_id;
    for (i = 0; i < ext_header.ext.DataSize + 2; i += 4)
    {
        num_data_obj++;
    }
    header.sop.NumDataObjects = num_data_obj;
    header.sop.Extended       = 1;

    *msg_id = (*msg_id + 1) & 0xF;

    USB_Type_C_PD_Port_A->USBPD_MSG_HEADER = (uint32_t)header.word;
    USB_Type_C_PD_Port_A->USBPD_DATA_TX    = BYTE_TO_WORD(data[0], data[1], data[2], data[3]);
    i                                      = 1;
    USB_Type_C_PD_Port_A->USBPD_TRANSMIT = num_bytes | USB_Type_C_PD_Port_USBPD_TRANSMIT_RETRY_CNT_0
                                           | USB_Type_C_PD_Port_USBPD_TRANSMIT_TX_SOP_SOP;

    while (!transmit)
    {
        interrupt                           = USB_Type_C_PD_Port_A->USB_INTERRUPT;
        USB_Type_C_PD_Port_A->USB_INTERRUPT = interrupt;
        if (interrupt & error_mask)
        {
            //it went to hell - do something
            transmit = 2;
        }
        if (interrupt & USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_success_Msk)
        {
            transmit = 1;
        }
        if (interrupt & USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_buf_rdy_Msk)
        {
            USB_Type_C_PD_Port_A->USBPD_DATA_TX =
                BYTE_TO_WORD(data[i * 4], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3]);
            i++;
        }
    }

	while(!transmit) 
	{
		interrupt = USB_Type_C_PD_Port_A->USB_INTERRUPT;
		USB_Type_C_PD_Port_A->USB_INTERRUPT = interrupt;
		if(interrupt & error_mask) {
			//it went to hell - do something
			transmit = 2;
		}
		if(interrupt & USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_success_Msk) {
			transmit = 1;
		}
		if(interrupt & USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_buf_rdy_Msk) {
			USB_Type_C_PD_Port_A->USBPD_DATA_TX = BYTE_TO_WORD(data[i*4], data[i*4+1],
					data[i*4+2], data[i*4+3]);
			i++;
		}
	}

	return transmit;
}

static enum msg_type pdfu_process_msg(uint8_t *msg, uint16_t *data_len)
{
    enum msg_type      type;
    PD_HEADER_T        header;
    FW_UPDATE_HEADER_T pdfu_header;

    /* Check Message Header */
    header.byte[0] = msg[0];
    header.byte[1] = msg[1];
    if (header.sop.MessageType != EMTFWUpdateRequest && header.sop.Extended != 1)
    {
        return PDFU_UNEXPECTED;
    }

    /* Check extended header */
    header.byte[0] = msg[4];
    header.byte[1] = msg[5];
    //	if(header.ext.Chunked == 1) {
    //		return PDFU_UNEXPECTED;
    //	}
    *data_len = header.ext.DataSize - 4;

    /* Check Update Request Header */
    pdfu_header.byte[0] = msg[6];
    pdfu_header.byte[1] = msg[7];
    switch (pdfu_header.MessageType)
    {
        case REQ_PDFU_DATA:
            type = PDFU_DATA_REQ;
            break;
        case REQ_PDFU_VALIDATE:
            type = PDFU_VALIDATE;
            break;
        case REQ_PDFU_INITIATE:
            type = PDFU_INITIATE;
            break;
        default:
            type = PDFU_UNEXPECTED;
            break;
    }

    //TODO: Check DataBlockIndex

    return type;
}

static void pdfu_construct_response(enum msg_type type, uint8_t status, uint8_t wait, uint16_t data,
                                    uint8_t *message, uint16_t *msg_size)
{
    PDFU_DATA_RESP_PAYLOAD_T     response;
    PDFU_VALIDATE_RESP_PAYLOAD_T validate;
    PDFU_INITIATE_RESP_PAYLOAD_T initiate;
    uint8_t                     *msg;
    PD_HEADER_T                  ext_header;
    uint16_t                     size;
    uint16_t                     i;

    //request next message or respond to signature (valid?)
    switch (type)
    {
        case PDFU_DATA_REQ:
            response.ProtocolVersion = 1;
            response.MessageType     = RESP_PDFU_DATA;
            response.Status          = status;
            response.WaitTime        = wait;
            response.NumDataNR       = 0;
            response.DataBlockNum    = data;

            size = PDFU_DATA_RESP_SIZE;
            msg  = response.byte;

            break;
        case PDFU_VALIDATE:
            validate.ProtocolVersion = 1;
            validate.MessageType     = RESP_PDFU_VALIDATE;
            validate.Status          = status;
            validate.WaitTime        = wait;
            validate.FlagsSuccess    = data;
            validate.FlagsReserved   = 0;

            size = PDFU_VALIDATE_RESP_SIZE;
            msg  = validate.byte;
            break;
        case PDFU_INITIATE:
            initiate.ProtocolVersion = 1;
            initiate.MessageType     = RESP_PDFU_INITIATE;
            initiate.Status          = status;
            initiate.WaitTime        = wait;
            initiate.MaxImageSize    = PROGRAM_MAX_IMAGE_SIZE;

            size = PDFU_INITIATE_RESP_SIZE;
            msg  = initiate.byte;
            break;
        default:
            break;
    }

    ext_header.word            = 0;
    ext_header.ext.ChunkNumber = 0;
    ext_header.ext.Chunked     = 1;
    ext_header.ext.DataSize    = size;

    message[0] = ext_header.byte[0];
    message[1] = ext_header.byte[1];

    for (i = 0; i < size; i++)
    {
        message[i + 2] = msg[i];
    }

    *msg_size = size + 4;
}

static void pdfu_unencrypt(uint8_t *in, uint8_t *out, uint16_t len)
{
    uint16_t i;

    for (i = 0; i < len; i++)
    {
        out[i] = in[i];
    }
    return;
}

static uint8_t pdfu_validate_signature(uint8_t *program_hash, uint8_t *sig)
{
    uint16_t i;

    /* Unencrypt Sig */
    pdfu_unencrypt(sig, sig, HASH_SIZE);

    /* Compare data hash to unencrypted signature */
    for (i = 0; i < HASH_SIZE; i++)
    {
        if (program_hash[i] != sig[i])
        {
            return 0;
        }
    }

    return 1;
}

static void set_cc_terminations(void)
{
    Clock_Management_Unit->PCLK_FEN |= Clock_Management_Unit_PCLK_FEN_USBPDA_LSCLK_EN_Msk;
    USB_Type_C_PD_Port_A->USBPD_CONFIG =
        USB_Type_C_PD_Port_USBPD_CONFIG_EN_PD_Msk | USB_Type_C_PD_Port_USBC_CTRL_CC_OVP_EN_Msk
        | USB_Type_C_PD_Port_USBC_CTRL_RP2_VAL_Msk | USB_Type_C_PD_Port_USBC_CTRL_RP1_VAL_Msk
        | USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_Msk | USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_Msk;
}

static uint8_t detect_attach(void)
{
    if (((USB_Type_C_PD_Port_A->USBC_STS & USB_Type_C_PD_Port_USBC_STS_CC1_STS_Msk)
         == USB_Type_C_PD_Port_USBC_STS_CC1_STS_SRC_RD)
        || ((USB_Type_C_PD_Port_A->USBC_STS & USB_Type_C_PD_Port_USBC_STS_CC2_STS_Msk)
            == USB_Type_C_PD_Port_USBC_STS_CC2_STS_SRC_RD))
    {
        return 1;
    }

    return 0;
}

static void set_orientation(void)
{
    if ((USB_Type_C_PD_Port_A->USBC_STS & USB_Type_C_PD_Port_USBC_STS_CC2_STS_Msk)
        == USB_Type_C_PD_Port_USBC_STS_CC2_STS_SRC_RD)
    {
        USB_Type_C_PD_Port_A->USBC_STS |= USB_Type_C_PD_Port_USBC_CTRL_CC_ORIENT_Msk;
    }
}

uint8_t firmware_boot_check(void)
{
    if (*(uint32_t *)(NVR2_ADDRESS) != NVR_BOOT_KEY)
    {
        return 1;
    }

    return 0;
}

void firmware_update(void)
{
    uint8_t  message_buffer[MSG_BUFFER_BYTE_SIZE];
    uint8_t  message_response[12];
    uint8_t *program_data;
    uint8_t  msg_id = 0;
    uint16_t msg_size;

    uint8_t       program_hash[256];
    uint32_t      buffer;
    uint16_t      data_len;
    uint16_t      chunk       = 0;
    uint8_t       programming = 1;
    uint8_t       fw_valid    = 0;
    uint16_t      validated   = 0;
    enum msg_type message_type;
    uint8_t       ret;

    /* Wait for Attach */
    set_cc_terminations();
    while (!detect_attach())
    {
    }
    set_orientation();

    /* Erase Flash */
    flash_init();
    flash_erase(PROGRAM_START_ADDRESS, PROGRAM_END_ADDRESS);

    /* Init hash */
    hash_init(program_hash);

    /* Initialize PD */
    pdfu_init_pd();

    while (programming)
    {
        ret = pdfu_receive_message(message_buffer, &msg_id);
        if (ret == 2)
        {
            break;
        }
        message_type = pdfu_process_msg(message_buffer, &data_len);

        if (message_type == PDFU_DATA_REQ)
        {
            pdfu_construct_response(PDFU_DATA_REQ, OK, PROGRAM_WAIT, chunk + 1, message_response,
                                    &msg_size);
            pdfu_transmit_pd(EMTFWUpdateResponse, message_response, msg_size, &msg_id);

            program_data = &message_buffer[DATA_PAYLOAD_BYTE_OFFSET];

            pdfu_unencrypt(program_data, program_data, data_len);

            hash(program_hash, program_data, data_len, program_hash);
            //TODO: Keep direct track of absolute address for reading signature
            //from device memory
            flash_program(program_data, FLASH_CHUNK_ADDRESS(chunk), data_len);
            chunk++;
        }
        else if (message_type == PDFU_VALIDATE)
        {
            if (!validated)
            {
                pdfu_construct_response(PDFU_VALIDATE, OK, SIG_WAIT, validated, message_response,
                                        &msg_size);
                pdfu_transmit_pd(EMTFWUpdateResponse, message_response, msg_size, &msg_id);

                if (pdfu_validate_signature(program_hash, program_hash))
                {
                    /* Update boot indicator */
                    nvr_init();
                    nvr_erase(NVR2_ADDRESS);
                    buffer = NVR_BOOT_KEY;
                    nvr_program(&buffer, NVR2_ADDRESS, 1);
                    validated = 1;
                    fw_valid  = 1;
                }
                else
                {
                    validated = 1;
                    fw_valid  = 0;
                }
            }
            else
            {
                pdfu_construct_response(PDFU_VALIDATE, OK, 0, fw_valid, message_response,
                                        &msg_size);
                pdfu_transmit_pd(EMTFWUpdateResponse, message_response, msg_size, &msg_id);
                programming = 0;
            }
        }
        else if (message_type == PDFU_INITIATE)
        {
            /* Return initiate success */
            pdfu_construct_response(PDFU_INITIATE, OK, 0, 0, message_response, &msg_size);
            pdfu_transmit_pd(EMTFWUpdateResponse, message_response, msg_size, &msg_id);
        }
        else
        {
            programming = 0;
        }
    }

    /* Issue reset */
    SCB->AIRCR = RESET_VECTKEY;

	/* Issue reset */
	SCB->AIRCR = RESET_VECTKEY;

	while(1)
    {
    }
}
