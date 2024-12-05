
#include "dpm_callbacks.h"
#if CONFIG_ENABLED(CUSTOM_APPLICATION)
void dpm_event_vbus_req_cb(struct port_tcpd *dev, void *ctx)
{
    EVENT_VBUS_REQ_T *evt      = (EVENT_VBUS_REQ_T *)ctx;
    DPM_INFO_T       *dpm_info = (DPM_INFO_T*)dev->dpm_info_priv;
    DPM_T            *dpm      = (DPM_T *)(dpm_info->dpm);
    struct port          *port = dev->port_tcpd_priv;
    MPQ4260_STATUS_FAULTS fault;
    int8_t                status;
    EVENT_VBUS_REQ_T     *vbus_req = ctx;
    PPS_MODE_T old_pps_mode = dpm_info->pps_mode;
    /* Set PPS UVP alarm if in CL mode */
    if (vbus_req->arg.type == pdoTypeAugmented)
    {
        /* Clear */
        pdpsReadFaultStatus();
        /* Get Latest */
        pdpsReadFaultStatus();
        fault = pdpsGetFaultStatus(dev->port_tcpd_id);
        if (fault.IOUT_OC_FAULT)
        {
            /*Set UVP Alarm */
            fusbdev_tcpd_set_vbus_alarm(port->dev->tcpd_device, PERCENT(PPS_MIN_V, UV_PERCENT),
                                        VBUS_LOW_THRESHOLD(vbus_req->arg.mv));
        }
    }
    pdpsReadFaultStatus();
    /* Get Latest */
    pdpsReadFaultStatus();
    /* Set Voltage / Current */
    status = pdpsSetVI(dev->port_tcpd_id, (uint16_t)vbus_req->arg.mv,
                       (uint16_t)((vbus_req->arg.ma *100)/100), vbus_req->arg.type == pdoTypeAugmented);
    if (status == -1 && vbus_req->arg.mv != 0)
    {
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
        pdps_ocp_reset(&dpm_info->ocp_dev, &dpm->ps_dev, dpm->port_bc1p2[dev->port_tcpd_id],
                       dpm->port_tcpd[dev->port_tcpd_id]);
#else
        //port_set_error_recovery(dpm->port_tcpd[dev->port_tcpd_id]);
#endif
    }
    vbus_req->ret.success = EVENT_STATUS_SUCCESS;
    timer_disable(&dpm_info->pps_query_timer);
}
#endif
