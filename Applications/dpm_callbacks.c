/*******************************************************************************
 * @file   dpm_callbacks.c
 * @author USB Firmware Team
 * @brief  Implements the Device Policy Manager state machine functions.
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
*******************************************************************************/
#include "dpm_callbacks.h"
/**
 * @brief Callback for event #EVENT_TC_DETACHED
 * @param port_id
 * @param ctx
 */
static void dpm_event_tc_detached_cb(struct port_tcpd *dev, void *ctx)
{
    DPM_INFO_T *dpm_info = (DPM_INFO_T *)dev->dpm_info_priv;
    DPM_T      *dpm      = dpm_info->dpm;
#if CONFIG_ENABLED(LEGACY_CHARGING)
    dpm_info->pd_dev_attached = false;
    port_bc1p2_sm_disable(dpm_info->dpm->port_bc1p2[dev->port_tcpd_id]->port_bc1p2_priv);
#endif
#if CONFIG_ENABLED(POWER_SHARING)
    struct power_sharing_dev *ps_dev = &dpm_info->dpm->ps_dev;
    port_ps_notify_tc_detached(dev->port_tcpd_id, ctx, ps_dev);
#endif
#if CONFIG_ENABLED(CUSTOM_APPLICATION)
    pdps_ocp_init(&dpm_info->ocp_dev);
#endif
}

/**
 * @brief Callback for #EVENT_TC_ATTACHED
 * @param port_id
 * @param ctx
 */
static void dpm_event_tc_attached_cb(struct port_tcpd *dev, void *ctx)
{
    DPM_INFO_T *dpm_info = (DPM_INFO_T *)dev->dpm_info_priv;
#if CONFIG_ENABLED(LEGACY_CHARGING)
    port_bc1p2_sm_enable(dpm_info->dpm->port_bc1p2[dev->port_tcpd_id]->port_bc1p2_priv);
#endif
#if CONFIG_ENABLED(POWER_SHARING)
    struct power_sharing_dev *ps_dev = &dpm_info->dpm->ps_dev;
    port_ps_notify_tc_attached(dev->port_tcpd_id, ctx, ps_dev);
#endif
}

/**
 * @brief Callback for event when partner is PD capable #EVENT_PD_DEVICE
 * @param port_id
 * @param ctx
 */
static void dpm_event_pd_device_cb(struct port_tcpd *dev, void *ctx)
{
    DPM_INFO_T *dpm_info      = (DPM_INFO_T *)dev->dpm_info_priv;
    dpm_info->pd_dev_attached = true;
#if CONFIG_ENABLED(LEGACY_CHARGING)
    port_bc1p2_sm_disable(dpm_info->dpm->port_bc1p2[dev->port_tcpd_id]->port_bc1p2_priv);
#endif
#if CONFIG_ENABLED(POWER_SHARING)
    struct power_sharing_dev *ps_dev = &dpm_info->dpm->ps_dev;
    port_ps_notify_pd_device(dev->port_tcpd_id, ctx, ps_dev);
#endif
}

static void dpm_event_pd_get_pps_status(struct port_tcpd *dev, void *ctx)
{
    DPM_INFO_T                 *dpm = (DPM_INFO_T *)dev->dpm_info_priv;
    EVENT_PPS_STATUS_REQUEST_T *evt = (EVENT_PPS_STATUS_REQUEST_T *)ctx;

    evt->arg.ppssdb->OutputVoltage = fusbdev_tcpd_vbus_value(dev->tcpd_device) / 20;
    evt->arg.ppssdb->OutputCurrent = 0xFF;
    evt->arg.ppssdb->Flags         = 0;

    if (dpm->pps_mode == PPS_MODE_CL)
    {
        evt->arg.ppssdb->OMF = 1;
    }
    else
    {
        evt->arg.ppssdb->OMF = 0;
    }
    evt->ret.success = EVENT_STATUS_SUCCESS;
}

static void dpm_event_set_pps_monitor_cb(struct port_tcpd *dev, void *ctx)
{
    DPM_INFO_T                 *dpm = (DPM_INFO_T *)dev->dpm_info_priv;
    EVENT_PPS_MONITOR_ENABLE_T *evt = (EVENT_PPS_MONITOR_ENABLE_T *)ctx;
    dpm->pps_active                 = evt->arg.pps_active;
    evt->ret.success                = EVENT_STATUS_SUCCESS;
}

/*pps is active and just enabled, set the PPS shutdown range*/
static void dpm_event_set_pps_alarm_cb(struct port_tcpd *dev, void *ctx)
{
    EVENT_PPS_ALARM_T *evt = (EVENT_PPS_ALARM_T *)ctx;
    fusbdev_tcpd_set_vbus_alarm(dev->tcpd_device, evt->arg.mv_low, evt->arg.mv_high);
    evt->ret.success = EVENT_STATUS_SUCCESS;
}
#if !CONFIG_ENABLED(CUSTOM_APPLICATION)
void dpm_event_vbus_req_cb(struct port_tcpd *dev, void *ctx)
{
    EVENT_VBUS_REQ_T *evt      = (EVENT_VBUS_REQ_T *)ctx;
    DPM_INFO_T       *dpm_info = (DPM_INFO_T*)dev->dpm_info_priv;
    DPM_T            *dpm      = (DPM_T *)(dpm_info->dpm);
#if CONFIG_ENABLED(DCDC)
#if DEVICE_TYPE == FUSB15200 || (DEVICE_TYPE == FUSB15201 || DEVICE_TYPE == FUSB15201P)
    if (evt->arg.mv == 0)
    {
        /* Turn off vbus switch */
        port_dc_dc_set(dpm->port_dcdc->port_dcdc_priv, dev->port_tcpd_id, evt->arg.mv, evt->arg.ma);
        port_dc_dc_delayed_disable(dpm->port_dcdc->port_dcdc_priv, dev->port_tcpd_id);
    }
    else
    {
        if (dev->port_tcpd_id == HAL_USBPD_CH_B)
        {
            enable_swd(dpm->dpm_info, false);
        }
        port_dc_dc_enable(dpm->port_dcdc->port_dcdc_priv, dev->port_tcpd_id, true);
        port_dc_dc_delayed_vbus_write(dpm->port_dcdc->port_dcdc_priv, dev->port_tcpd_id,
                                      DCDC_VBUS_SCALE(evt->arg.mv), evt->arg.ma);
    }
    evt->ret.success           = EVENT_STATUS_SUCCESS;
#endif
#elif CONFIG_ENABLED(CUSTOM_APPLICATION)
#else
    evt->ret.success = EVENT_STATUS_DEFAULT;
#endif
}
#endif
//not power
#if !CONFIG_ENABLED(POWER_SHARING)

static void dpm_event_pd_get_src_cap(struct port_tcpd *dev, void *ctx)
{
    EVENT_PD_GET_SRC_CAP_T *evt = ctx;

    uint32_t max_power = ((struct dpm_info *)dev->dpm_info_priv)->port_pdp;

    /* Access VIF */
    int      i = 0;
    uint32_t max_current;
    uint32_t nom_v;
    uint32_t max_v;
    uint8_t  num_pdo = 0;

    for (i = 0; i < dev->vif->Num_Src_PDOs; i++)
    {
        switch (evt->arg.src_cap[i].PDO.SupplyType)
        {
            case pdoTypeFixed:
                max_current = (max_power * 2) / evt->arg.src_cap[i].FPDOSupply.Voltage;
                max_current = MIN(max_current, evt->arg.src_cap[i].FPDOSupply.MaxCurrent);
                evt->arg.src_cap[i].FPDOSupply.EPRCapable = dev->vif->EPR_Supported_As_Src;
                evt->arg.src_cap[i].FPDOSupply.MaxCurrent = max_current;
                num_pdo++;
                break;
            case pdoTypeAugmented:
                max_v = evt->arg.src_cap[i].PPSAPDO.MaxVoltage;
                if (max_v >= 200)
                {
                    nom_v = 200;
                }
                else if (max_v >= 150)
                {
                    nom_v = 150;
                }
                else if (max_v >= 90)
                {
                    nom_v = 90;
                }
                else
                {
                    nom_v = 50;
                }
                max_current = (max_power / 5) / nom_v;
                max_current = MIN(max_current, evt->arg.src_cap[i].PPSAPDO.MaxCurrent);

                evt->arg.src_cap[i].PPSAPDO.MaxCurrent = max_current;
                if (max_power < (max_current * max_v * 5))
                {
                    evt->arg.src_cap[i].PPSAPDO.PowerLimited = 1;
                }

                /* Do not advertise PPS < 1A */
                if (max_current >= 20)
                {
                    num_pdo++;
                }

                break;
            case pdoTypeBattery:
                break;
            case pdoTypeVariable:
                break;
            default:
                break;
        }
    }

    evt->ret.num     = num_pdo;
    evt->ret.success = EVENT_STATUS_SUCCESS;
}

static void dpm_event_pd_get_snk_cap(struct port_tcpd *dev, void *ctx)
{
    EVENT_PD_GET_SNK_CAP_T *evt = ctx;

    uint32_t max_power = ((struct dpm_info *)dev->dpm_info_priv)->port_pdp;

    /* Access VIF */
    int      i = 0;
    uint32_t max_current;
    uint32_t nom_v;
    uint32_t max_v;
    uint8_t  num_pdo = 0;

    for (i = 0; i < dev->vif->Num_Snk_PDOs; i++)
    {
        switch (evt->arg.snk_cap[i].PDO.SupplyType)
        {
            case pdoTypeFixed:
                max_current = (max_power * 2) / evt->arg.snk_cap[i].FPDOSupply.Voltage;
                max_current = MIN(max_current, evt->arg.snk_cap[i].FPDOSupply.MaxCurrent);
                evt->arg.snk_cap[i].FPDOSupply.EPRCapable = dev->vif->EPR_Mode_Capable;
                evt->arg.snk_cap[i].FPDOSupply.MaxCurrent = max_current;
                num_pdo++;
                break;
            case pdoTypeAugmented:
                max_v = evt->arg.snk_cap[i].PPSAPDO.MaxVoltage;
                if (max_v >= 200)
                {
                    nom_v = 200;
                }
                else if (max_v >= 150)
                {
                    nom_v = 150;
                }
                else if (max_v >= 90)
                {
                    nom_v = 90;
                }
                else
                {
                    nom_v = 50;
                }
                max_current = (max_power / 5) / nom_v;
                max_current = MIN(max_current, evt->arg.snk_cap[i].PPSAPDO.MaxCurrent);

                evt->arg.snk_cap[i].PPSAPDO.MaxCurrent = max_current;
                if (max_power < (max_current * max_v * 5))
                {
                    evt->arg.snk_cap[i].PPSAPDO.PowerLimited = 1;
                }

                /* Do not advertise PPS < 1A */
                if (max_current >= 20)
                {
                    num_pdo++;
                }

                break;
            case pdoTypeBattery:
                break;
            case pdoTypeVariable:
                break;
            default:
                break;
        }
    }

    evt->ret.num     = num_pdo;
    evt->ret.success = EVENT_STATUS_SUCCESS;
}
#endif
#if CONFIG_ENABLED(EPR)
static void dpm_event_pd_get_epr_src_cap(struct port_tcpd *dev, void *ctx)
{
    EVENT_PD_GET_SRC_CAP_T *evt  = ctx;

    uint32_t max_power = ((struct dpm_info *)dev->dpm_info_priv)->port_pdp;

    /* Access VIF */
    int      i = 0;
    uint32_t max_current;
    bool     req_ok = false;
    uint32_t prev_nom_v;
    uint32_t nom_v;
    uint32_t max_v;
    uint8_t  num_pdo = 0;

    for (i = 7; i < 7 + dev->vif->Num_Src_EPR_PDOs; i++)
    {
        switch (evt->arg.src_cap[i].PDO.SupplyType)
        {
            case pdoTypeFixed:
                max_current = (max_power * 2) / evt->arg.src_cap[i].FPDOSupply.Voltage;
                max_current = MIN(max_current, evt->arg.src_cap[i].FPDOSupply.MaxCurrent);
                evt->arg.src_cap[i].FPDOSupply.MaxCurrent = max_current;
                num_pdo++;
                break;
            case pdoTypeAugmented:
                max_v = evt->arg.src_cap[i].EPRAVSAPDOSupply.MaxVoltage * 100;
                if (max_v >= 48000)
                {
                    nom_v      = 48000;
                    prev_nom_v = 36000;
                }
                else if (max_v >= 36000)
                {
                    nom_v      = 36000;
                    prev_nom_v = 28000;
                }
                else if (max_v >= 28000)
                {
                    nom_v      = 28000;
                    prev_nom_v = 20000;
                }
                else
                {
                    nom_v = 28000;
                }
                if ((max_power / 1000) > ((prev_nom_v * 5) / 1000))
                {
                    num_pdo++;
                }
                break;
            case pdoTypeBattery:
                break;
            case pdoTypeVariable:
                break;
            default:
                break;
        }
    }

    /* Note that num_pdos is always going to be 7 + number of EPR pdos */
    evt->ret.num     = 7 + num_pdo;
    evt->ret.success = EVENT_STATUS_SUCCESS;
}

static void dpm_event_pd_get_epr_snk_cap(struct port_tcpd *dev, void *ctx)
{
    EVENT_PD_GET_SNK_CAP_T *evt = ctx;
    evt->ret.success            = EVENT_STATUS_DEFAULT;
    uint32_t max_power          = dev->vif->PD_Power_as_Sink;
    /* Access VIF */
    int      i = 0;
    uint32_t max_current;
    uint32_t nom_v;
    uint32_t max_v;
    uint8_t  num_pdo = 0;
    if (evt->arg.pd_rev == 3)
    {
        for (i = 7; i < 7 + dev->vif->Num_Snk_EPR_PDOs; i++)
        {
            switch (evt->arg.snk_cap[i].PDO.SupplyType)
            {
                case pdoTypeFixed:
                    max_current = (max_power * 2) / evt->arg.snk_cap[i].FPDOSupply.Voltage;
                    max_current = MIN(max_current, evt->arg.snk_cap[i].FPDOSupply.MaxCurrent);
                    evt->arg.snk_cap[i].FPDOSupply.MaxCurrent = max_current;
                    num_pdo++;
                    break;
                case pdoTypeAugmented:
                    max_v = evt->arg.snk_cap[i].PPSAPDO.MaxVoltage;
                    if (max_v >= 200)
                    {
                        nom_v = 200;
                    }
                    else if (max_v >= 150)
                    {
                        nom_v = 150;
                    }
                    else if (max_v >= 90)
                    {
                        nom_v = 90;
                    }
                    else
                    {
                        nom_v = 50;
                    }
                    max_current = (max_power / 5) / nom_v;
                    max_current = MIN(max_current, evt->arg.snk_cap[i].PPSAPDO.MaxCurrent);

                    evt->arg.snk_cap[i].PPSAPDO.MaxCurrent = max_current;
                    if (max_power < (max_current * max_v * 5))
                    {
                        evt->arg.snk_cap[i].PPSAPDO.PowerLimited = 1;
                    }

                    /* Do not advertise PPS < 1A */
                    if (max_current >= 20)
                    {
                        num_pdo++;
                    }
                    break;
                case pdoTypeBattery:
                    break;
                case pdoTypeVariable:
                    break;
                default:
                    break;
            }
        }
        /* Note that num_pdos is always going to be 7 + number of EPR pdos */
        evt->ret.num     = 7 + num_pdo;
        evt->ret.success = EVENT_STATUS_SUCCESS;
    }
}
#endif
static void dpm_event_populate_source_caps(struct port_tcpd *dev, void *ctx)
{
    EVENT_PD_POP_SRC_CAPS_T *evt      = (EVENT_PD_POP_SRC_CAPS_T *)ctx;
    DPM_INFO_T              *dpm_info = dev->dpm_info_priv;
    struct port             *port     = (struct port *)dev->port_tcpd_priv;
    DPM_T                   *dpm      = dpm_info->dpm;
    int                      index    = 0;
#if CONFIG_ENABLED(CUSTOM_APPLICATION) && CONFIG_ENABLED(CUSTOM_APPLICATION_001)
    for (uint32_t i = 0; i < dev->vif->Num_Src_PDOs; i++)
    {
        if (dpm->ps_dev.shared_capacity == 30000 && i == 3)
        {
            index++;
        }
        else if (dpm->ps_dev.shared_capacity != 30000 && i == 4)
        {
            index++;
        }
        evt->arg.caps[i] = dev->vif->src_caps[index];
        index++;
    }
#else
    for (int i = 0; i < port->dev->vif->Num_Src_PDOs; i++)
    {
        port->caps_source[i] = port->dev->vif->src_caps[i];
    }
#if CONFIG_ENABLED(EPR)
    if (port->epr_mode)
    {
        /* if in EPR mode send all EPR source caps as well */
        for (int i = 7; i < (7 + (port->dev->vif->Num_Src_EPR_PDOs)); i++)
        {
            port->caps_source[i] = port->dev->vif->src_caps[i];
        }
    }
#endif
#endif
    evt->ret.success = EVENT_STATUS_SUCCESS;
}
/**
 * @brief Callback for event #EVENT_PD_SRC_EVAL_SNK_REQ
 * @param port_id
 * @param ctx event argument
 */

#if CONFIG_ENABLED(POWER_SHARING)

static void dpm_event_pd_req_cb(struct port_tcpd *dev, void *ctx)
{
    ps_event_pd_req_cb(dev, ctx);
}

static void dpm_event_pd_new_contract(struct port_tcpd *dev, void *ctx)
{
    ps_event_pd_new_contract(dev, ctx);
}

static void dpm_event_bist_shared_test_mode(struct port_tcpd *dev, void *ctx)
{
    ps_event_bist_shared_test_mode(dev, ctx);
}

static void dpm_event_pd_snk_cap(struct port_tcpd *dev, void *ctx)
{
    ps_event_pd_snk_cap(dev, ctx);
}

static void dpm_event_pd_snk_cap_ext(struct port_tcpd *dev, void *ctx)
{
    ps_event_pd_snk_cap_ext(dev, ctx);
}

static void dpm_event_battery_cap(struct port_tcpd *dev, void *ctx)
{
    ps_event_battery_cap(dev, ctx);
}

static void dpm_event_battery_status(struct port_tcpd *dev, void *ctx)
{
    ps_event_battery_status(dev, ctx);
}

#else
static void dpm_event_pd_req_cb(struct port_tcpd *dev, void *ctx)
{
    EVENT_PD_SRC_EVAL_SNK_REQ_T *evt = ctx;
    struct port *port = (struct port *) dev->port_tcpd_priv;
    short req_idx;
    enum pdo_t pdo;
    bool epr_req = evt->arg.caps_count > 7;
    bool req_ok = false;
    unsigned short ireq, isrc, vsrcmax, vsrcmin, vreq;
    uint32_t pdp, maxpdp;
    struct dpm_info * dpm_info = (struct dpm_info *)dev->dpm_info_priv;
    maxpdp = dpm_info->port_pdp;
    ireq = isrc = vsrcmax = vsrcmin = vreq = 0;
    req_idx = evt->arg.req->FVRDO.ObjectPosition - 1;
    if (req_idx < evt->arg.caps_count && req_idx > -1)
    {
        if (evt->arg.caps[req_idx].object != 0)
        {
            pdo = evt->arg.caps[req_idx].FPDOSupply.SupplyType;
            if (pdo == pdoTypeFixed)
            {
                /* Check if port can meet the sink operational current */
                if (evt->arg.req->FVRDO.OpCurrent <= evt->arg.caps[req_idx].FPDOSupply.MaxCurrent)
                {
                    req_ok = true;
                }
            }
            else if (pdo == pdoTypeAugmented)
            {
                if (evt->arg.pd_rev >= PD_REV3
                    && evt->arg.caps[req_idx].APDO.APDOType == apdoTypePPS)
                {
                    ireq = evt->arg.req->PPSRDO.OpCurrent;
                    vreq = evt->arg.req->PPSRDO.OpVoltage;
                    isrc = evt->arg.caps[req_idx].PPSAPDO.MaxCurrent;
                    vsrcmax = evt->arg.caps[req_idx].PPSAPDO.MaxVoltage * 5;
                    vsrcmin = evt->arg.caps[req_idx].PPSAPDO.MinVoltage * 5;
                    pdp = ireq * vreq;
                    /* can't accept PPS requests with current less than 1A anymore */
					if ((ireq <= isrc) && (ireq >= 20) && (vreq <= vsrcmax) && (vreq >= vsrcmin))
					{
#if (CONFIG_ENABLED(POWER_LIMITED))
						if (pdp > maxpdp)
						{
							evt->arg.req->PPSRDO.OpCurrent = MIN(maxpdp / vreq, ireq);
						}
#endif
						req_ok = true;
					}
                }
                else if (evt->arg.pd_rev >= PD_REV3
                        && evt->arg.caps[req_idx].SPRAVSAPDOSupply.APDOType == apdoTypeSPRAVS)
                {
                	vreq = evt->arg.req->AVSRDO.OpVoltage * 25;
                	ireq = evt->arg.req->AVSRDO.OpCurrent * 50;
                	if (evt->arg.caps[req_idx].SPRAVSAPDOSupply.maxCurrent20 &&
                			(vreq >= 15000))
                	{
                		vsrcmax = 20000; /*in milliVolts here */
                		vsrcmin = 15000;
                		isrc = evt->arg.caps[req_idx].SPRAVSAPDOSupply.maxCurrent20 * 10;
                	}
                	else
                	{
                		vsrcmax = 15000;/*in milliVolts here */
                		vsrcmin = 9000;
						isrc = evt->arg.caps[req_idx].SPRAVSAPDOSupply.maxCurrent15 * 10;
                	}
                	pdp = (vsrcmax * isrc)/1000;
                	maxpdp = MIN(pdp,((struct dpm_info *)dev->dpm_info_priv)->port_pdp);
                	/* Now that we know the PDP of the PDO, recalculate pdp for request */
                	pdp = (vreq * ireq)/1250;
                	/* make sure at least 1A + request is below max requested current */
                	if((vreq <= vsrcmax) && (vsrcmin <= vreq) && (ireq <= isrc) && (ireq >= 20))
                	{
                		if (pdp > maxpdp)
                		{
                			evt->arg.req->AVSRDO.OpCurrent = (MIN(maxpdp / vreq, ireq));
                		}
						req_ok = true;
                	}
                }
            }
            else if (pdo == pdoTypeBattery)
            {
            }
            else if (pdo == pdoTypeVariable)
            {
            }
        }
    }
    /* For EPR mode, check the objects match so that it matches the behavior */
    if (epr_req)
    {
        /* Might be incorrect */
        if (evt->arg.caps[req_idx].object != evt->arg.req[1].object)
        {
            /* If normal request sent, should get an error earlier. Thus, this must be an EPR_Request */
            req_ok = false;
        }
    }
    evt->ret.success = req_ok ? EVENT_STATUS_SUCCESS : EVENT_STATUS_FAILED;
}
#endif

static void dpm_event_fw_id_cb(struct port_tcpd *dev, void *ctx)
{
    EVENT_PD_GET_FW_ID_T     *evt        = ctx;
    GET_FW_ID_RESP_PAYLOAD_T *fw_id_resp = evt->arg.fw_id_resp;

    /* Implementation note: These are most likely statically defined */
    fw_id_resp->ProtocolVersion = PDFU_PROTOCOL_V1;
    fw_id_resp->MessageType     = RESP_GET_FW_ID;
    fw_id_resp->Status          = OK;
    fw_id_resp->VIDL            = LOW_BYTE(dev->vif->Manufacturer_Info_VID_Port);
    fw_id_resp->VIDH            = HIGH_BYTE(dev->vif->Manufacturer_Info_VID_Port);
    fw_id_resp->PIDL            = LOW_BYTE(dev->vif->Manufacturer_Info_PID_Port);
    fw_id_resp->PIDH            = HIGH_BYTE(dev->vif->Manufacturer_Info_PID_Port);
    fw_id_resp->HWVersionMajor  = 0;
    fw_id_resp->HWVersionMinor  = 0;
    fw_id_resp->SiVersionBase   = 0;

    fw_id_resp->FWVersion1H = HIGH_BYTE(FW_VERSION_1);
    fw_id_resp->FWVersion1L = LOW_BYTE(FW_VERSION_1);
    fw_id_resp->FWVersion2H = HIGH_BYTE(FW_VERSION_2);
    fw_id_resp->FWVersion2L = LOW_BYTE(FW_VERSION_2);
    fw_id_resp->FWVersion3H = HIGH_BYTE(FW_VERSION_3);
    fw_id_resp->FWVersion3L = LOW_BYTE(FW_VERSION_3);
    fw_id_resp->FWVersion4H = HIGH_BYTE(FW_VERSION_4);
    fw_id_resp->FWVersion4L = LOW_BYTE(FW_VERSION_4);

    fw_id_resp->Flags1PDUpdate = CONFIG_ENABLED(FW_UPDATE);

    fw_id_resp->Flags1USBUpdate    = 0;
    fw_id_resp->Flags1NotUpdatable = 0;
    fw_id_resp->Flags1Silent       = 0;

    fw_id_resp->Flags2FullyFunctional = 0;
    fw_id_resp->Flags2UnplugSafe      = 0;

    fw_id_resp->Flags3HardResetRequired = 0;
    fw_id_resp->Flags3USBFunction       = 0;
    fw_id_resp->Flags3AltModes          = 0;
    fw_id_resp->Flags3PowerLimit        = 1;
    fw_id_resp->Flags3MorePower         = 0;

    fw_id_resp->Flags4UnmountStorage = 0;
    fw_id_resp->Flags4Replug         = 0;
    fw_id_resp->Flags4SwapCable      = 0;
    fw_id_resp->Flags4PowerCycle     = 0;

    evt->ret.success = EVENT_STATUS_SUCCESS;
}
#if (CONFIG_ENABLED(EPR))
static void dpm_event_epr_mode_entry(struct port_tcpd *dev, void *ctx)
{
    EVENT_EPR_MODE_ENTRY_T *evt = ctx;
#if (DEVICE_TYPE == FUSB15201P || DEVICE_TYPE == FUSB15201 || DEVICE_TYPE == FUSB15200)
#if (CONFIG_ENABLED(DCDC))
#if (CONFIG_ENABLED(POWER_SHARING))
    evt->arg.eprinfo.availablePower =
        (dev->vif->Product_Total_Source_Power_mW - MIN_POWER_1_5A) / 500;
#else
    evt->arg.eprinfo.availablePower = dev->vif->Product_Total_Source_Power_mW / 2;
    /*Max voltage fixed by device and DCDC board*/
#endif
    evt->arg.eprinfo.maxVoltage = 280;
    /*Max voltage fixed by device and cable (limited by cable)*/
    evt->arg.eprinfo.peakCurrent = 100;
#endif
#endif
    if (evt->arg.eprinfo.availablePower > 300 && evt->arg.eprinfo.maxVoltage >= 280)
    {
        evt->ret.success = EVENT_STATUS_SUCCESS;
    }
    else
    {
        evt->ret.success = EVENT_STATUS_FAILED;
    }
}

static void dpm_event_pd_snk_eval_src_cap(struct port_tcpd *dev, void *ctx)
{
    EVENT_PD_SNK_EVAL_SRC_CAP_T *evt = (EVENT_PD_SNK_EVAL_SRC_CAP_T *)ctx;
    evt->ret.success                 = EVENT_STATUS_DEFAULT;
    struct port *port                = (struct port *)dev->port_tcpd_priv;

    int max_power_index            = 0;
    unsigned long src_voltage_mv   = 0;
    unsigned long src_current_ma   = 0;
    unsigned long snk_power_mw     = port->vif->PD_Power_as_Sink;
    unsigned long src_power_mw     = 0;
    unsigned long max_voltage      = 0;
    unsigned long max_current      = 0;
    uint32_t max_power             = 0;
    uint8_t i                      = 0;
    if (port->negotiate_epr_contract && port->epr_mode)
    {
        i = 7;
        evt->arg.num = port->prl_ext_num_bytes / 4;
    }
    else if (port->epr_mode)
    {
        evt->arg.num = 7;
    }
    /* Find out which index contains the max power */
    for (; i < evt->arg.num; i++)
    {
        switch(evt->arg.pdo->FPDOSupply.SupplyType)
        {
            case pdoTypeFixed:
                src_voltage_mv   = evt->arg.pdo[i].FPDOSupply.Voltage*50;
                src_current_ma   = evt->arg.pdo[i].FPDOSupply.MaxCurrent*10;
                src_power_mw     = (src_voltage_mv * src_current_ma)/1000;
                break;
            case pdoTypeAugmented:
                if (evt->arg.pdo[i].PPSAPDO.APDOType  == 0)
                {
                    //SPR - PPS Augmented PDO
                    src_voltage_mv   = PD_100MV_STEP_TO_MV(evt->arg.pdo[i].PPSAPDO.MaxVoltage);
                    src_current_ma   = PD_50MA_STEP_TO_MA(evt->arg.pdo[i].PPSAPDO.MaxCurrent);
                    src_power_mw     = (src_voltage_mv * src_current_ma)/1000;
                }
                else if (evt->arg.pdo[i].PPSAPDO.APDOType  == 1)
                {
                    //EPR - AVS Augmented PDO
                    src_voltage_mv   = PD_100MV_STEP_TO_MV(evt->arg.pdo[i].EPRAVSAPDOSupply.MaxVoltage);
                    src_current_ma   = PD_50MA_STEP_TO_MA(evt->arg.pdo[i].EPRAVSAPDOSupply.PeakCurrent);
                    src_power_mw     = (src_voltage_mv * src_current_ma)/1000;
                }
                break;
            case pdoTypeBattery:
                break;
            case pdoTypeVariable:
                break;
            default:
                break;
        }

        if(max_power < src_power_mw)
        {
            max_power = src_power_mw;
            max_voltage = src_voltage_mv;
            max_current = src_current_ma;
            max_power_index = i;
        }
    }
    evt->ret.index = max_power_index;
    evt->ret.type  = evt->arg.pdo[max_power_index].FPDOSupply.SupplyType;
    evt->ret.req   = evt->arg.pdo[max_power_index];
    switch(evt->ret.type)
    {
        case pdoTypeFixed:
            evt->ret.req.FVRDO.ObjectPosition  = max_power_index + 1;
            evt->ret.req.FVRDO.GiveBack        = port->vif->GiveBack_May_Be_Set;
            evt->ret.req.FVRDO.NoUSBSuspend    = port->vif->No_USB_Suspend_May_Be_Set;
            evt->ret.req.FVRDO.UnchunkedExtMsg = port->vif->Unchunked_Extended_Messages_Supported;
            evt->ret.req.FVRDO.OpCurrent       = port->vif->snk_caps[max_power_index].FPDOSink.OperationalCurrent;
            if (evt->ret.req.FVRDO.OpCurrent > max_current)
            {
                /* Sink cannot meet required current so request the max */
                evt->ret.req.FVRDO.OpCurrent = max_current;
            }
            evt->ret.req.FVRDO.MinMaxCurrent = (2 * snk_power_mw) / (10 * max_voltage); // 10 mA unit
            if (max_power < snk_power_mw)
            {
                evt->ret.req.FVRDO.CapabilityMismatch = 1;
            }
            break;
        case pdoTypeAugmented:
            if (evt->ret.req.PPSAPDO.APDOType  == 0)
            {
                //SPR - PPS Augmented PDO
                evt->ret.req.PPSRDO.ObjectPosition  = max_power_index + 1;
                evt->ret.req.PPSRDO.NoUSBSuspend    = port->vif->No_USB_Suspend_May_Be_Set;
                evt->ret.req.PPSRDO.UnchunkedExtMsg = port->vif->Unchunked_Extended_Messages_Supported;
                evt->ret.req.PPSRDO.OpCurrent       = max_current / 50; //50 mA unit
                evt->ret.req.PPSRDO.OpVoltage       = max_voltage / 20; //20 mV unit
                if (max_power < snk_power_mw)
                {
                    evt->ret.req.PPSRDO.CapabilityMismatch = 1;
                }
            }
            else if (evt->ret.req.PPSAPDO.APDOType == 1)
            {
                //EPR - AVS Augmented PDO
                evt->ret.req.AVSRDO.ObjectPosition  = max_power_index + 1;
                evt->ret.req.AVSRDO.NoUSBSuspend    = port->vif->No_USB_Suspend_May_Be_Set;
                evt->ret.req.AVSRDO.UnchunkedExtMsg = port->vif->Unchunked_Extended_Messages_Supported;
                evt->ret.req.AVSRDO.OpCurrent       = max_current / 50; //50 mA unit
                evt->ret.req.AVSRDO.OpVoltage       = max_voltage / 25; //25 mV unit
                if (max_power < snk_power_mw)
                {
                    evt->ret.req.AVSRDO.CapabilityMismatch = 1;
                }
            }
            break;
        case pdoTypeBattery:
            break;
        case pdoTypeVariable:
            break;
        default:
            break;
    }
    evt->ret.success = EVENT_STATUS_SUCCESS;

}

#endif
static void dpm_event_fw_initiate_cb(struct port_tcpd *dev, void *ctx)
{
    EVENT_PD_FW_INITIATE_T       *evt            = ctx;
    uint16_t                      fw_ver_1       = evt->arg.fw_init_req->FWVersion1;
    uint16_t                      fw_ver_2       = evt->arg.fw_init_req->FWVersion2;
    uint16_t                      fw_ver_3       = evt->arg.fw_init_req->FWVersion3;
    uint16_t                      fw_ver_4       = evt->arg.fw_init_req->FWVersion4;
    PDFU_INITIATE_RESP_PAYLOAD_T *fw_init_resp   = evt->arg.fw_init_resp;
    uint16_t                      current_ver[4] = {0};

    fw_init_resp->ProtocolVersion = PDFU_PROTOCOL_V1;
    fw_init_resp->MessageType     = RESP_PDFU_INITIATE;
    if (dev->port_tcpd_id == 0
        && fw_version_check(current_ver, fw_ver_1, fw_ver_2, fw_ver_3, fw_ver_4))
    {
        fw_init_resp->Status       = OK;
        fw_init_resp->WaitTime     = 150;     /* 10ms units */
        fw_init_resp->MaxImageSize = 0x19000; /* Bytes */

        evt->ret.success = EVENT_STATUS_SUCCESS;
    }
    else
    {
        fw_init_resp->Status = errTarget;
        evt->ret.success     = EVENT_STATUS_FAILED;
    }
}

#if CONFIG_ENABLED(FW_UPDATE)
static void dpm_event_fw_reconfig_cb(struct port_tcpd *dev, void *ctx)
{
    /* Disable VBUS on all ports */
    struct tcpd_device *dev_a = &fusb15200_dpm_objects[HAL_USBPD_CH_A].port_dev;
    struct tcpd_device *dev_b = &fusb15200_dpm_objects[HAL_USBPD_CH_B].port_dev;
    fusb15200_set_vbus_source(dev_a, VBUS_OFF, VBUS_OFF, 0);
    fusb15200_set_vbus_source(dev_b, VBUS_OFF, VBUS_OFF, 0);

    /* Go to firmware programming mode! */
    FLASH->FLASH_KEY_NVR = 0xCDEF5678;

    /* Erase NVR */
    FLASH->FLASH_ADDR    = 0x08000200;
    FLASH->FLASH_NVR_CTL = 0x2;
    while (FLASH->FLASH_INT_STS == 0)
    {
    }
    if (FLASH->FLASH_INT_STS & 0x4)
    {
        FLASH->FLASH_INT_STS &= 0x4;
        return;
    }
    else
    {
        FLASH->FLASH_INT_STS &= 0x2;
    }

    /* Issue reset */
    SCB->AIRCR = 0x05FA0004;
}
#endif

static void dpm_event_error_recovery(struct port_tcpd *dev, void *ctx)
{
    DPM_INFO_T *dpm_info = dev->dpm_info_priv;
    DPM_T      *dpm      = dpm_info->dpm;
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
    pdps_ocp_reset(&dpm_info->ocp_dev, &dpm->ps_dev, dpm->port_bc1p2[dev->port_tcpd_id],
                   dpm->port_tcpd[dev->port_tcpd_id]);
#else
    port_set_error_recovery(dev);

#endif
}
static void dpm_event_pd_give_source_info(struct port_tcpd *dev, void *ctx)
{
    EVENT_PD_GIVE_SOURCE_INFO_T *evt      = ctx;
    struct pd_msg_t             *info     = evt->arg.info;
    DPM_INFO_T                  *dpm_info = dev->dpm_info_priv;
    struct port                 *port     = dev->port_tcpd_priv;
    DPM_T                       *dpm      = dpm_info->dpm;
    info->SIDO.PortType                   = dev->vif->PD_Port_Type;
#if (NUM_TYPEC_PORTS == 2) && (CONFIG_ENABLED(POWER_SHARING))
    info->SIDO.PortReportedPDP = dpm->ps_dev.sink_cap[dev->port_tcpd_id].advertised_power / 1000;
    info->SIDO.PortMaximumPDP  = dev->vif->PD_Power_as_Source / 1000;
    info->SIDO.PortPresentPDP  = dpm->ps_dev.sink_cap[dev->port_tcpd_id].port_power / 1000;
    info->SIDO.PortType = port->vif->Port_Managed_Guaranteed_Type;
#else
    info->SIDO.PortReportedPDP = dpm_info->port_pdp / 1000;
    info->SIDO.PortMaximumPDP = dev->vif->PD_Power_as_Source / 1000;
    info->SIDO.PortPresentPDP = dpm_info->port_pdp / 1000;
    info->SIDO.PortType = port->vif->Port_Managed_Guaranteed_Type;
#endif
}

static void dpm_event_pd_give_revision(struct port_tcpd *dev, void *ctx)
{
    EVENT_PD_GIVE_REVISION_T *evt = ctx;
    struct pd_msg_t          *rev = evt->arg.revision;
    rev->RMDO.RevisionMajor       = dev->vif->PD_Spec_Revision_Major;
    rev->RMDO.RevisionMinor       = dev->vif->PD_Spec_Revision_Minor;
    rev->RMDO.VersionMajor        = dev->vif->PD_Spec_Version_Major;
    rev->RMDO.VersionMinor        = dev->vif->PD_Spec_Version_Minor;
    evt->ret.success              = EVENT_STATUS_SUCCESS;
}
#if CONFIG_ENABLED(USB4)
static void dpm_event_data_reset_exit_cb(struct port_tcpd * dev, void *ctx)
{
	EVENT_PD_DATA_RESET_T * evt = (EVENT_PD_DATA_RESET_T *)ctx;
	struct port * port = (struct port *)dev->port_tcpd_priv;
	port->vdm_auto_state = true;
	port->vdm_check_cbl = true;
    port->usb_mode       = false;
    port->msgtx.booltxmsg.enterusb = false;
    port->usb_supported_sopx = 0;
    port->enter_usb_rejected = 0;
    port->enter_usb_sent_sopx = 0;
    port->disc_id_sent_sopx = 0;
}
static void dpm_event_enter_usb_received_cb(struct port_tcpd * dev, void *ctx)
{
	struct event_enter_usb_received_t* evt = (struct event_enter_usb_received_t *)ctx;
	evt->ret.success = (evt->arg.enter_usb_msg->EUDO.USBMode <= USB4) ? EVENT_STATUS_SUCCESS : EVENT_STATUS_FAILED;
}

static void dpm_event_enter_usb_request_cb(struct port_tcpd * dev, void *ctx)
{
	struct event_enter_usb_request_t* evt = (struct event_enter_usb_request_t *)ctx;
	struct port * port = dev->port_tcpd_priv;
	uint8_t min_speed = MIN(port->cable_vdo_cable_highest_usb_speed, port->partner_product_type_ufp.SVDM.UFPVDO.usb_highest_speed);
	uint8_t usb_mode = 0;
	/* correct usb mode based on max speed */
	if (min_speed >= VDM_UFP_VDO_USB_3_2_GEN1)
	{
		if (port->cable_vdo_cable_type_ufp == VDM_ID_UFP_SOP_1_ACTIVE_CABLE)
		{
			if (port->cable_vdo_cable_highest_usb4_mode)
			{
				usb_mode = USB4;
			}
			else if (port->cable_vdo_cable_highest_usb3_2_mode)
			{
				usb_mode = USB3;
			}
			else
			{
				usb_mode = USB2;
			}
		}
		else
		{
			usb_mode = USB4;
		}
	}
	else
	{
		usb_mode = USB2;
	}
	/* correct usb type independent of TBT3 presence */
	/* if TBT3 does not exist,  */
	if (port->tbt3vdo.SVDM.TBT3MODEVDO.Active)
	{
		if (port->tbt3vdo.SVDM.TBT3MODEVDO.Optical)
		{
			evt->arg.enter_usb_msg->EUDO.CableType = USB_OPTICAL_ISOLATED;
		}
		else if (port->tbt3vdo.SVDM.TBT3MODEVDO.Retimer)
		{
			evt->arg.enter_usb_msg->EUDO.CableType = USB_ACTIVE_RETIMER;
		}
		else
		{
			evt->arg.enter_usb_msg->EUDO.CableType = USB_ACTIVE_REDRIVER;
		}
	}
	else
	{
		evt->arg.enter_usb_msg->EUDO.CableType = USB_PASSIVE;
	}
	evt->arg.enter_usb_msg->EUDO.HostPresent = 1;
	evt->arg.enter_usb_msg->EUDO.PCIeSupport = 0;
	evt->arg.enter_usb_msg->EUDO.TBTSupport = 0;
	evt->arg.enter_usb_msg->EUDO.USBMode = usb_mode;
	evt->arg.enter_usb_msg->EUDO.USB4DRD = dev->vif->DR_Swap_To_DFP_Supported || dev->vif->DR_Swap_To_UFP_Supported;
	evt->arg.enter_usb_msg->EUDO.USB3DRD = dev->vif->DR_Swap_To_DFP_Supported || dev->vif->DR_Swap_To_UFP_Supported;
	evt->arg.enter_usb_msg->EUDO.CableSpeed = min_speed;
	evt->arg.enter_usb_msg->EUDO.DPSupport = 1;
}
#endif
static void dpm_event_pps_cl(struct port_tcpd * dev, void *ctx)
{
	struct dpm_info *dpm_info = (struct dpm_info*)dev->dpm_info_priv;
	EVENT_PD_SRC_EVAL_SNK_REQ_T *evt = (EVENT_PD_SRC_EVAL_SNK_REQ_T *)ctx;
	evt->ret.success = dpm_info->pps_mode == PPS_MODE_CL ? EVENT_STATUS_SUCCESS : EVENT_STATUS_FAILED;
}

static void dpm_event_pd_get_alert_req(struct port_tcpd * dev, void *ctx)
{
	struct dpm_info *dpm_info = (struct dpm_info*)dev->dpm_info_priv;
	EVENT_PD_ALERT_T *evt = (EVENT_PD_ALERT_T *)ctx;
	struct port * port = dev->port_tcpd_priv;
	if (port->pps_mode_change) {
		evt->arg.alert_msg->ALERT.Type = ALERT_TYPE_OPERATION;
	}
	evt->ret.success = EVENT_STATUS_SUCCESS;
}

static void dpm_event_tx_req_cb(struct port_tcpd * dev, void *ctx)
{
	//don't bother passing CTX, all we need to check message legality is in port struct
	struct port * port = dev->port_tcpd_priv;
}

#if (CONFIG_ENABLED(CONST_EVENTS))
const event_callback_fn EVENT_HANDLES[CONFIG_MAX_EVENTS] = {
#if CONFIG_ENABLED(USB4)
    dpm_event_data_reset_exit_cb,
    dpm_event_enter_usb_received_cb,
    dpm_event_enter_usb_request_cb,
    0,
#endif
#if CONFIG_ENABLED(EXTENDED_EVENTS)
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
#endif
#if CONFIG_ENABLED(LEGACY_CHARGING) || CONFIG_ENABLED(POWER_SHARING)
    dpm_event_tc_attached_cb,
    dpm_event_tc_detached_cb,
    dpm_event_pd_device_cb,
#endif
#if CONFIG_ENABLED(SNK) && CONFIG_ENABLED(EPR)
	0,
	dpm_event_pd_snk_eval_src_cap,
#elif CONFIG_ENABLED(SNK)
	0,
	0,
#endif
	dpm_event_tx_req_cb,
	dpm_event_vbus_req_cb,
#if CONFIG_ENABLED(POWER_SHARING)
	ps_event_pd_get_src_cap,
	0,
	ps_event_pd_snk_cap,
	ps_event_pd_snk_cap_ext,
#else
	dpm_event_pd_get_src_cap,
	dpm_event_pd_get_snk_cap,
	0,
	0,
#endif
#if CONFIG_ENABLED(BATTERY_EVENTS) || CONFIG_ENABLED(POWER_SHARING)
	ps_event_battery_cap,
	ps_event_battery_status,
#if CONFIG_ENABLED(EXTENDED_EVENTS)
	0,
	0,
#endif
#endif
#if CONFIG_ENABLED(POWER_SHARING)
    ps_event_pd_req_cb,
#else
    dpm_event_pd_req_cb,
#endif
    dpm_event_pd_get_pps_status,
    dpm_event_set_pps_monitor_cb,
    dpm_event_set_pps_alarm_cb,
#if CONFIG_ENABLED(VDM) && !CONFIG_ENABLED(VDM_MINIMAL)
#if (MODAL_OPERATION_SUPPORTED)
    0,
    0,
    0,
#endif
    0,
#endif
	0,
    dpm_event_error_recovery,
#if CONFIG_ENABLED(POWER_SHARING)
    ps_event_bist_shared_test_mode,
    ps_event_pd_new_contract,
#else
    0,
    0,
#endif
    dpm_event_fw_id_cb,
    dpm_event_fw_initiate_cb,
#if CONFIG_ENABLED(FW_UPDATE)
    dpm_event_fw_reconfig_cb,
#else
    0,
#endif
#if CONFIG_ENABLED(EPR)
    dpm_event_epr_mode_entry,
#else
    0,
#endif
    dpm_event_pd_give_source_info,
    dpm_event_pd_give_revision,
    dpm_event_pps_cl,
#if CONFIG_ENABLED(EPR)
    dpm_event_pd_get_epr_src_cap,
    dpm_event_pd_get_epr_snk_cap,
#else
    0,
    0,
#endif
    0,
    0,
    dpm_event_pd_get_alert_req,
    0,
} ;
#else
void dpm_register_callbacks(void)
{
    /*PPS Callbacks*/
	event_subscribe(EVENT_TX_REQ, dpm_event_tx_req_cb);
    event_subscribe(EVENT_PPS_ALARM, dpm_event_set_pps_alarm_cb);
    event_subscribe(EVENT_PPS_MONITOR, dpm_event_set_pps_monitor_cb);
    event_subscribe(EVENT_PPS_STATUS_REQUEST, dpm_event_pd_get_pps_status);

    event_subscribe(EVENT_PD_GET_ALERT_REQ, dpm_event_pd_get_alert_req);
    event_subscribe(EVENT_PPS_CL, dpm_event_pps_cl);

    /* Hard Reset event (necessary with PDPS OCP machine) */
    event_subscribe(EVENT_ERROR_RECOVERY, dpm_event_error_recovery);
    /*Firmware Update support Callbacks*/
    event_subscribe(EVENT_PD_GET_FW_ID, dpm_event_fw_id_cb);
    event_subscribe(EVENT_PD_FW_INITIATE, dpm_event_fw_initiate_cb);

    event_subscribe(EVENT_VBUS_REQ, dpm_event_vbus_req_cb);
#if CONFIG_ENABLED(USB4)
    event_subscribe(EVENT_DATA_RESET_EXIT, dpm_event_data_reset_exit_cb);
    event_subscribe(EVENT_ENTER_USB_RECEIVED, dpm_event_enter_usb_received_cb);
    event_subscribe(EVENT_ENTER_USB_REQUEST, dpm_event_enter_usb_request_cb);
#endif
#if CONFIG_ENABLED(FW_UPDATE)
    event_subscribe(EVENT_PD_INITIATE_RESP_SENT, dpm_event_fw_reconfig_cb);
#endif

#if !CONFIG_ENABLED(POWER_SHARING)
    /* Comment out for new testing without 21V PPS PDO */
    event_subscribe(EVENT_PD_SRC_EVAL_SNK_REQ, dpm_event_pd_req_cb);
    event_subscribe(EVENT_PD_GET_SRC_CAP, dpm_event_pd_get_src_cap);
    event_subscribe(EVENT_PD_GET_SNK_CAP, dpm_event_pd_get_snk_cap);
#endif

    event_subscribe(EVENT_PD_GIVE_REVISION, dpm_event_pd_give_revision);
    event_subscribe(EVENT_PD_GIVE_SOURCE_INFO, dpm_event_pd_give_source_info);

#if CONFIG_ENABLED(LEGACY_CHARGING) || CONFIG_ENABLED(POWER_SHARING)
    event_subscribe(EVENT_TC_ATTACHED, dpm_event_tc_attached_cb);
    event_subscribe(EVENT_TC_DETACHED, dpm_event_tc_detached_cb);
    event_subscribe(EVENT_PD_DEVICE, dpm_event_pd_device_cb);
#endif
#if (CONFIG_ENABLED(EPR))
    event_subscribe(EVENT_EPR_MODE_ENTRY, dpm_event_epr_mode_entry);
    event_subscribe(EVENT_PD_GET_SRC_EPR_CAP, dpm_event_pd_get_epr_src_cap);
    event_subscribe(EVENT_PD_GET_SNK_EPR_CAP, dpm_event_pd_get_epr_snk_cap);
#if CONFIG_ENABLED(SNK)
    event_subscribe(EVENT_PD_SNK_EVAL_SRC_CAP, dpm_event_pd_snk_eval_src_cap);
#endif
#endif
}
#endif
