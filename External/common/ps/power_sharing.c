/*******************************************************************************
 * @file     power_sharing.c
 * @author   USB PD Firmware Team
 *
 * @copyright @parblock
 * Copyright &copy; 2021 ON Semiconductor &reg;. All rights reserved.
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
#include "port.h"
#include "observer.h"
#include "vif_types.h"
#include "power_sharing.h"
#include "dpm.h"

#if CONFIG_ENABLED(POWER_SHARING)
static POWER_RESERVE_T *PS_DEV;

static void ps_renegotiate_ports(struct power_sharing_dev *ps_dev);
static void ps_renegotiate_highest_port(struct power_sharing_dev *ps_dev);

static uint8_t num_pd_ports(struct power_sharing_dev *ps_dev)
{
    uint8_t num_ports = 0;
    uint8_t i         = 0;

    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        if (ps_dev->sink_cap[i].pd_capable == 1)
        {
            num_ports++;
        }
    }

    return num_ports;
}

static void ps_init_port(struct SINK_CAP_T *sink_cap, struct port_tcpd *dev)
{
    sink_cap->min_power        = MIN_POWER_3A;
    sink_cap->op_power         = MIN_POWER_3A;
    sink_cap->max_power        = MIN_POWER_3A;
    sink_cap->pdo_power        = MIN_POWER_1_5A;
    sink_cap->advertised_power = MIN_POWER_1_5A;
    sink_cap->port_power       = dev->vif->PD_Power_as_Source;

    sink_cap->battery[0].capacity      = DEFAULT_BATT_CAP;
    sink_cap->battery[0].soc           = 0;
    sink_cap->battery[0].charge_status = BATTERY_CHARGING;

    sink_cap->renegotiate       = 0;
    sink_cap->mismatch          = 0;
    sink_cap->redistributing    = 0;
    sink_cap->mismatch_handling = 0;
    sink_cap->pd_capable        = 0;
}

void port_ps_init(struct power_sharing_dev *power)
{
    int               i         = 0;
    struct port_tcpd *port_tcpd = power->port_tcpd[0];

    power->shared_capacity       = port_tcpd->vif->Product_Total_Source_Power_mW;
    power->pdp_rating            = port_tcpd->vif->PD_Power_as_Source;
    power->available_power       = power->shared_capacity;
    power->cv_thresh             = MIN(60 * PS_SCALE / 100, PS_SCALE - 1);
    power->bist_shared_test_mode = 0;

    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        ps_init_port(&power->sink_cap[i], power->port_tcpd[i]);
        power->available_power -= power->sink_cap[i].pdo_power;
        timer_disable(&power->sink_cap[i].status_request_timer);
    }
    PS_DEV = power;
}

static void ps_renegotiate_highest_port(struct power_sharing_dev *ps_dev)
{
    uint32_t highest_power = 0;
    uint8_t  highest_port  = 0;
    uint8_t  i             = 0;
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        if (ps_dev->sink_cap[i].renegotiate == 1)
        {
            if (ps_dev->sink_cap[i].pdo_power > ps_dev->sink_cap[i].port_power)
            {
                highest_power = ps_dev->sink_cap[i].pdo_power;
                highest_port  = i;
                port_set_contract_valid(ps_dev->port_tcpd[i], false);
                break;
            }
            else
            {
                highest_power = ps_dev->sink_cap[i].pdo_power;
                highest_port  = i;
            }
        }
    }

    /* Return if no renegotiates */
    if (highest_power == 0)
        return;

    ps_dev->sink_cap[highest_port].renegotiate = 0;

    /* Send Source Caps to highest port */
    port_transmit_source_caps(ps_dev->port_tcpd[highest_port]);

    /* Update TypeC-only for highest port */
    if (port_policy_has_contract(ps_dev->port_tcpd[highest_port]) == false)
    {
        if (port_tc_state(ps_dev->port_tcpd[highest_port]) == AttachedSource)
        {
            ps_dev->available_power += ps_dev->sink_cap[highest_port].pdo_power;

            if (ps_dev->available_power >= MIN_POWER_3A)
            {
                ps_dev->sink_cap[highest_port].pdo_power = MIN_POWER_3A;
                port_source_set_rp_current(ps_dev->port_tcpd[highest_port], CC_RP_3p0A);
            }
            else
            {
                ps_dev->sink_cap[highest_port].pdo_power = MIN_POWER_1_5A;
                port_source_set_rp_current(ps_dev->port_tcpd[highest_port], CC_RP_1p5A);
            }

            ps_dev->available_power -= ps_dev->sink_cap[highest_port].pdo_power;
        }

        ps_renegotiate_ports(ps_dev);
    }
}

static void ps_update_mismatches(struct power_sharing_dev *ps_dev)
{
    uint8_t i = 0;
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        if (ps_dev->sink_cap[i].redistributing)
        {
            ps_dev->sink_cap[i].redistributing = 0;
            if (ps_dev->sink_cap[i].mismatch)
            {
                port_transmit_source_caps(ps_dev->port_tcpd[i]);
                break;
            }
        }
    }
}

static void ps_renegotiate_ports(struct power_sharing_dev *ps_dev)
{
    uint8_t renegotiate       = 0;
    uint8_t redistributing    = 0;
    uint8_t mismatch_handling = 1;
    uint8_t i                 = 0;
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        renegotiate |= ps_dev->sink_cap[i].renegotiate;
        redistributing |= ps_dev->sink_cap[i].redistributing;
        mismatch_handling &= ps_dev->sink_cap[i].mismatch_handling;
    }

    if (renegotiate)
    {
        ps_renegotiate_highest_port(ps_dev);
    }
    else if (redistributing && mismatch_handling)
    {
        ps_update_mismatches(ps_dev);
    }
}

static void ps_battery_balancing(struct power_sharing_dev *ps_dev, uint32_t *max_power)
{
    uint32_t total_capacity  = 0;
    uint32_t total_weight    = 0;
    int32_t  port_weight[2]  = {0};
    int32_t  percent_cap     = 0;
    uint32_t power           = 0;
    uint32_t remaining_power = *max_power;

    int i;
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        /* Update capacity to be >= soc for correcting phones */
        ps_dev->sink_cap[i].battery[0].capacity =
            MAX(ps_dev->sink_cap[i].battery[0].capacity, ps_dev->sink_cap[i].battery[0].soc);
        total_capacity += ps_dev->sink_cap[i].battery[0].capacity;
    }

    if (total_capacity != 0)
    {
        for (i = 0; i < TYPE_C_PORT_COUNT; i++)
        {
            if (ps_dev->sink_cap[i].battery[0].capacity == 0)
                continue;

            percent_cap = (ps_dev->sink_cap[i].battery[0].capacity * PS_SCALE) / total_capacity;

            port_weight[i] = ((ps_dev->sink_cap[i].battery[0].soc * PS_SCALE)
                              / ps_dev->sink_cap[i].battery[0].capacity)
                             - ps_dev->cv_thresh;
            port_weight[i] = MAX(port_weight[i], 0);
            port_weight[i] = (port_weight[i] * PS_SCALE) / (PS_SCALE - ps_dev->cv_thresh);
            port_weight[i] = PS_SCALE - port_weight[i];

            port_weight[i] = (port_weight[i] * percent_cap) / PS_SCALE;

            total_weight += port_weight[i];
        }

        /* Normalize */
        if (total_weight == 0)
        {
            /* Zero */
            for (i = 0; i < TYPE_C_PORT_COUNT; i++)
            {
                port_weight[i] = 0;
            }
        }
        else
        {
            /* Normalize */
            for (i = 0; i < TYPE_C_PORT_COUNT; i++)
            {
                port_weight[i] = (port_weight[i] * PS_SCALE) / total_weight;
            }
        }
    }
    else
    {
        /* Even split */
        for (i = 0; i < TYPE_C_PORT_COUNT; i++)
        {
            port_weight[i] = PS_SCALE / TYPE_C_PORT_COUNT;
        }
    }

    /* Distribute remaining max power */
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        power = (remaining_power * port_weight[i]) / PS_SCALE;
        ps_dev->sink_cap[i].port_power += power;
        *max_power -= power;
    }
}

static void ps_partition_power(struct power_sharing_dev *ps_dev)
{
    uint32_t max_power = ps_dev->shared_capacity;
    uint32_t power     = 0;

    /* For each port */
    int i;

    /* Set each port to default */
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        power = MIN_POWER_1_5A;
        if (max_power >= power)
        {
            ps_dev->sink_cap[i].port_power = power;
            max_power -= power;
        }
    }

    /* Give remaining power up to min */
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        power = ps_dev->sink_cap[i].min_power - ps_dev->sink_cap[i].port_power;
        if (max_power >= power)
        {
            ps_dev->sink_cap[i].port_power += power;
            max_power -= power;
        }
    }

    ps_battery_balancing(ps_dev, &max_power);

    /* Get back some power if greater than max */
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        if (ps_dev->sink_cap[i].port_power > ps_dev->sink_cap[i].max_power)
        {
            max_power += ps_dev->sink_cap[i].port_power - ps_dev->sink_cap[i].max_power;
            ps_dev->sink_cap[i].port_power = ps_dev->sink_cap[i].max_power;
        }
    }

    /* Distribute any extra power */
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        power = ps_dev->sink_cap[i].max_power - ps_dev->sink_cap[i].port_power;
        power = MIN(power, max_power);
        if (max_power >= power)
        {
            ps_dev->sink_cap[i].port_power += power;
            max_power -= power;
        }
    }

    /* Give above max power to remaining ports */
    uint8_t pd_ports = num_pd_ports(ps_dev);
    if (pd_ports < 1)
    {
        pd_ports = TYPE_C_PORT_COUNT;
    }

    power = max_power / pd_ports;
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        if (ps_dev->sink_cap[i].pd_capable)
        {
            ps_dev->sink_cap[i].port_power += power;
            max_power -= power;
        }
    }

    /* Set ports to renegotiate */
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        ps_dev->sink_cap[i].renegotiate    = 1;
        ps_dev->sink_cap[i].redistributing = 1;
    }
}

static uint8_t ps_num_attached_ports(struct port_tcpd *port_tcpd[TYPE_C_PORT_COUNT])
{
    uint8_t num_ports = 0;
    uint8_t i         = 0;

    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        if (port_tc_state(port_tcpd[i]) == AttachedSource)
        {
            num_ports++;
        }
    }

    return num_ports;
}

uint32_t ps_max_contract_power(struct power_sharing_dev *ps_dev, uint8_t port_id)
{
    uint32_t max_power = 0;

    /* Limit power to min of available total and port PDP */
    max_power = ps_dev->available_power + ps_dev->sink_cap[port_id].pdo_power;
    max_power = MIN(ps_dev->pdp_rating, max_power);

    /* Further limit power based on power redistribution unless cap mismatch */
    if (ps_dev->sink_cap[port_id].redistributing || !ps_dev->sink_cap[port_id].mismatch_handling
        || !ps_dev->sink_cap[port_id].mismatch)
    {
        max_power = MIN(ps_dev->sink_cap[port_id].port_power, max_power);
    }

    /* Ignore limits in BIST Shared Test Mode */
    if (ps_dev->bist_shared_test_mode)
    {
        max_power = ps_dev->pdp_rating;
    }

    return max_power;
}

void ps_event_pd_get_src_cap(struct port_tcpd *dev, void *ctx)
{
    struct power_sharing_dev *ps_dev = PS_DEV;
    EVENT_PD_GET_SRC_CAP_T   *evt    = ctx;

    uint32_t max_power = ps_max_contract_power(ps_dev, dev->port_tcpd_id);
    ps_dev->sink_cap[dev->port_tcpd_id].advertised_power = max_power;

    /* Go through existing pdos and adjust parameters
     * based on available power
     */

    /* Adhere to normative options based on pdp and available power */
    /* Reduce current on non-normative voltages or just remove? */
    /* <= 15W - 5V XA
     * <= 27W - ..., 9V XA
     * <= 45W - ..., 15V XA
     * <= 60W - ..., 20V XA
     * Max 3A or non-cable, 5A for cable
     */

    /* Access VIF */
    int      i           = 0;
    uint32_t max_current = 0;
    uint32_t nom_v       = 0;
    uint32_t max_v       = 0;
    uint8_t  num_pdo     = 0;

    for (i = 0; i < dev->vif->Num_Src_PDOs; i++)
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

                if (CONFIG_ENABLED(NOMINAL_PPS_CURRENT))
                {
                    max_current = (max_power / 5) / nom_v;
                }
                else
                {
                    max_current = (max_power / 5) / max_v;
                }

                max_current = MIN(max_current, evt->arg.src_cap[i].PPSAPDO.MaxCurrent);

                evt->arg.src_cap[i].PPSAPDO.MaxCurrent = max_current;
                if (CONFIG_ENABLED(POWER_LIMITED) && max_power < (max_current * max_v * 5))
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

void ps_event_pd_req_cb(struct port_tcpd *dev, void *ctx)
{
    struct power_sharing_dev    *ps_dev  = PS_DEV;
    EVENT_PD_SRC_EVAL_SNK_REQ_T *evt     = (EVENT_PD_SRC_EVAL_SNK_REQ_T *)ctx;
    uint32_t                     req_idx = 0;
    PDO_T                        pdo     = {0};
    bool                         req_ok  = false;
    uint16_t                     ireq    = 0;
    uint16_t                     isrc    = 0;
    uint16_t                     vsrcmax = 0;
    uint16_t                     vsrcmin = 0;
    uint16_t                     vreq    = 0;
    uint16_t                     nom_v   = 0;

    uint32_t op_current      = 0;
    uint32_t pdo_v           = 0;
    uint32_t pdo_current     = 0;
    uint32_t pdo_power       = 0;
    uint32_t min_max_current = 0;
    uint32_t op_power        = 0;
    uint32_t min_max_power   = 0;
    uint8_t  i               = 0;

    uint32_t max_power = ps_max_contract_power(ps_dev, dev->port_tcpd_id);

    /* Notes:
     * Fixed PDO/Request:
     * 	V in 50mV units (100 - 5V)
     * 	I in 10mA units (300 - 3A)
     * 	V * I / 2 = 1mW (15,000 - 15W)
     * Battery Request:
     * 	P in 250mW units
     * PPS Request:
     * 	V in 20mV units (250 - 5V)
     * 	I in 50mA units (60 - 3A)
     * 	V * I = 1mW		(15,000 - 15W)
     * PPS APDO:
     * 	V in 100mV units (50 - 5V)
     * 	I in 50mA units  (60 - 3A)
     * 	V * I * 5 = 1mW  (15,000 - 15W)
     */

    /* If there's not enough, see if you can reclaim some */
    /* 1. Queue a Wait message for this port
    	 * 2. See if any port can give back
    	 * 3. If so, do a gotomin for the other port(s)
    	 * 4. After gotomins or if no giveback, send new source caps to original port
    	 */

    uint32_t last_pdo_power = ps_dev->sink_cap[dev->port_tcpd_id].pdo_power;
    uint8_t  last_mismatch  = ps_dev->sink_cap[dev->port_tcpd_id].mismatch;

    req_idx = evt->arg.req->FVRDO.ObjectPosition - 1;
    if (req_idx < evt->arg.caps_count)
    {
        pdo = evt->arg.caps[req_idx].FPDOSupply.SupplyType;
        if (pdo == pdoTypeFixed)
        {
            op_current      = evt->arg.req->FVRDO.OpCurrent;
            min_max_current = evt->arg.req->FVRDO.MinMaxCurrent;
            pdo_v           = evt->arg.caps[req_idx].FPDOSupply.Voltage;
            pdo_current     = evt->arg.caps[req_idx].FPDOSupply.MaxCurrent;
            pdo_power       = (pdo_v * pdo_current) >> 1;
            op_power        = (op_current * pdo_v) >> 1;
            min_max_power   = (min_max_current * pdo_v) >> 1;

            /* Reclaim current contract */
            ps_dev->available_power += ps_dev->sink_cap[dev->port_tcpd_id].pdo_power;

            /* Make sure we can meet request */
            if (((ps_dev->available_power >= pdo_power) && (pdo_current >= op_current))
                || ps_dev->bist_shared_test_mode)
            {
                ps_dev->sink_cap[dev->port_tcpd_id].pdo_power = pdo_power;

                ps_dev->sink_cap[dev->port_tcpd_id].giveback = evt->arg.req->FVRDO.GiveBack;

                ps_dev->sink_cap[dev->port_tcpd_id].mismatch =
                    evt->arg.req->FVRDO.CapabilityMismatch;

                if (evt->arg.req->FVRDO.GiveBack)
                {
                    ps_dev->sink_cap[dev->port_tcpd_id].reserve = op_power - min_max_power;
                }
                else
                {
                    ps_dev->sink_cap[dev->port_tcpd_id].reserve = min_max_power - op_power;
                }

                req_ok = true;
            }

            /* Adjust power available power */
            ps_dev->available_power -= ps_dev->sink_cap[dev->port_tcpd_id].pdo_power;
        }
        else if (pdo == pdoTypeAugmented)
        {
            if (evt->arg.pd_rev >= PD_REV3 && evt->arg.caps[req_idx].APDO.APDOType == apdoTypePPS)
            {
                ireq    = evt->arg.req->PPSRDO.OpCurrent;
                vreq    = evt->arg.req->PPSRDO.OpVoltage;
                isrc    = evt->arg.caps[req_idx].PPSAPDO.MaxCurrent;
                vsrcmax = evt->arg.caps[req_idx].PPSAPDO.MaxVoltage * 5;
                vsrcmin = evt->arg.caps[req_idx].PPSAPDO.MinVoltage * 5;

                op_power  = vsrcmax * isrc;
                pdo_power = op_power;

                /* Reclaim current contract */
                ps_dev->available_power += ps_dev->sink_cap[dev->port_tcpd_id].pdo_power;

                if (((ireq <= isrc) && (ireq >= 20) && (vreq <= vsrcmax) && (vreq >= vsrcmin))
                    || ps_dev->bist_shared_test_mode)
                {
                    ps_dev->sink_cap[dev->port_tcpd_id].pdo_power = pdo_power;

                    /* If requested power is too high, limit power */
                    if (CONFIG_ENABLED(POWER_LIMITED) && max_power < pdo_power)
                    {
                        ps_dev->sink_cap[dev->port_tcpd_id].pdo_power = max_power;
                        evt->arg.req->PPSRDO.OpCurrent                = MIN(max_power / vreq, ireq);
                    }

                    ps_dev->sink_cap[dev->port_tcpd_id].giveback = 0;

                    ps_dev->sink_cap[dev->port_tcpd_id].mismatch =
                        evt->arg.req->FVRDO.CapabilityMismatch;

                    ps_dev->sink_cap[dev->port_tcpd_id].reserve = 0;

                    req_ok = true;
                }

                /* Adjust power available power */
                ps_dev->available_power -= ps_dev->sink_cap[dev->port_tcpd_id].pdo_power;
            }
        }
        else if (pdo == pdoTypeBattery)
        {
        }
        else if (pdo == pdoTypeVariable)
        {
        }
    }
    evt->ret.success = req_ok ? EVENT_STATUS_SUCCESS : EVENT_STATUS_FAILED;

    /* Resend Source Caps if already in a contract if Rejected */
    if (!req_ok)
    {
        port_transmit_source_caps(ps_dev->port_tcpd[dev->port_tcpd_id]);
    }

    /* No additional handling in BIST test mode */
    if (ps_dev->bist_shared_test_mode)
        return;

    /* Update this port if mismatch becomes set */
    if ((ps_dev->sink_cap[dev->port_tcpd_id].mismatch == 1) && (last_mismatch == 0))
    {
        ps_dev->sink_cap[dev->port_tcpd_id].redistributing = 1;
    }

    /* Update other ports if available power changes during mismatch handling */
    if (last_pdo_power != ps_dev->sink_cap[dev->port_tcpd_id].pdo_power)
    {
        for (i = 0; i < TYPE_C_PORT_COUNT; i++)
        {
            if (i == dev->port_tcpd_id)
                continue;
            if (ps_dev->sink_cap[i].mismatch_handling)
            {
                ps_dev->sink_cap[i].redistributing = 1;
            }
        }
    }

    /* Update other port if advertised power becomes invalid */
    if (dev->port_tcpd_id == 0)
    {
        if (ps_dev->sink_cap[1].advertised_power
            > (ps_dev->available_power + ps_dev->sink_cap[1].pdo_power))
        {
            port_transmit_source_caps(ps_dev->port_tcpd[1]);
        }
    }
    else if (dev->port_tcpd_id == 1)
    {
        if (ps_dev->sink_cap[0].advertised_power
            > (ps_dev->available_power + ps_dev->sink_cap[0].pdo_power))
        {
            port_transmit_source_caps(ps_dev->port_tcpd[0]);
        }
    }
}

void ps_event_pd_new_contract(struct port_tcpd *dev, void *ctx)
{
    struct power_sharing_dev *ps_dev = PS_DEV;
    ps_renegotiate_ports(ps_dev);
}

void ps_event_bist_shared_test_mode(struct port_tcpd *dev, void *ctx)
{
    struct power_sharing_dev *ps_dev = PS_DEV;
    uint8_t                   i      = 0;
    BIST_SHARED_TEST_MODE_T  *evt    = (BIST_SHARED_TEST_MODE_T *)ctx;

    ps_dev->bist_shared_test_mode = evt->arg.enter;

    /* Transmit Source Caps on entry, Error Recovery on exit */
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        if (evt->arg.enter)
        {
            port_transmit_source_caps(ps_dev->port_tcpd[i]);
        }
        else
        {
            port_set_error_recovery(ps_dev->port_tcpd[i]);
        }
    }
}

void ps_event_tc_attached(struct port_tcpd *dev, void *ctx)
{
    struct power_sharing_dev *ps_dev = PS_DEV;
    EVENT_TC_ATTACHED_T      *evt    = (EVENT_TC_ATTACHED_T *)ctx;

    /* Set Rp to 3A if available */
    if (evt->arg.src == true)
    {
        if (ps_num_attached_ports(ps_dev->port_tcpd) > 1)
        {
            ps_partition_power(ps_dev);
            ps_renegotiate_ports(ps_dev);
        }
        else
        {
            /* Reclaim current contract */
            ps_dev->available_power += ps_dev->sink_cap[dev->port_tcpd_id].pdo_power;

            if (ps_dev->available_power >= MIN_POWER_3A)
            {
                ps_dev->sink_cap[dev->port_tcpd_id].pdo_power = MIN_POWER_3A;
                port_source_set_rp_current(dev, CC_RP_3p0A);
            }

            ps_dev->available_power -= ps_dev->sink_cap[dev->port_tcpd_id].pdo_power;
        }
    }
}

void ps_event_tc_detached(struct port_tcpd *dev, void *ctx)
{
    struct power_sharing_dev *ps_dev = PS_DEV;
    struct port              *port   = dev->port_tcpd_priv;
    uint8_t                   i      = 0;
    /* Reclaim current contract */
    ps_dev->available_power += ps_dev->sink_cap[dev->port_tcpd_id].pdo_power;

    /* Reserve default power */
    ps_init_port(&ps_dev->sink_cap[dev->port_tcpd_id], dev);

    ps_dev->available_power -= ps_dev->sink_cap[dev->port_tcpd_id].pdo_power;

    /* Set the initial current advertisement to 1.5A min */
    port_source_set_rp_current(dev, MIN(CC_RP_1p5A, dev->vif->Rp_Value));

    /* Set ports to renegotiate */
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        ps_dev->sink_cap[i].port_power  = ps_dev->port_tcpd[i]->vif->PD_Power_as_Source;
        ps_dev->sink_cap[i].renegotiate = 1;
    }
    /* if no attached ports or if attached port is calling detach event */
    if (ps_num_attached_ports(ps_dev->port_tcpd) == 0
        || (port->tc_state == AttachedSource && ps_num_attached_ports(ps_dev->port_tcpd) == 1))
    {
        ps_dev->bist_shared_test_mode = 0;
    }
    ps_renegotiate_ports(ps_dev);
}

void ps_event_pd_snk_cap(struct port_tcpd *dev, void *ctx)
{
    struct power_sharing_dev    *ps_dev = PS_DEV;
    EVENT_PD_SNK_CAP_RECEIVED_T *evt    = (EVENT_PD_SNK_CAP_RECEIVED_T *)ctx;

    uint32_t min   = MIN_POWER_3A;
    uint32_t op    = min;
    uint32_t max   = op;
    uint32_t power = 0;
    uint32_t i     = 0;

    /* Try to guess PDP parameters for PD 2.0 */
    if (evt->arg.success)
    {
        /* Decode Sink Caps */
        for (i = 0; i < evt->arg.num; i++)
        {
            switch (evt->arg.snk_cap[i].PDO.SupplyType)
            {
                case pdoTypeFixed:
                    power = (evt->arg.snk_cap[i].FPDOSink.OperationalCurrent
                             * evt->arg.snk_cap[i].FPDOSink.Voltage)
                            >> 1;
                    if (evt->arg.snk_cap[i].FPDOSink.HigherCapability == 0)
                    {
                        op = MIN(op, power);
                    }
                    max = MAX(max, power);
                    break;
                case pdoTypeAugmented:
                    power = (evt->arg.snk_cap[i].PPSAPDO.MaxCurrent
                             * evt->arg.snk_cap[i].PPSAPDO.MaxVoltage)
                            * 5;
                    op  = MIN(op, power);
                    max = MAX(max, power);
                    break;
                case pdoTypeBattery:
                    power = (evt->arg.snk_cap[i].BPDO.MaxPower * 250);
                    max   = MAX(max, power);
                    break;
                default:
                    break;
            }
        }

        ps_dev->sink_cap[dev->port_tcpd_id].min_power = min;
        ps_dev->sink_cap[dev->port_tcpd_id].op_power  = op;
        ps_dev->sink_cap[dev->port_tcpd_id].max_power = max;
    }

    if (evt->arg.pd_rev < PD_REV3)
    {
        ps_dev->sink_cap[dev->port_tcpd_id].mismatch_handling = 1;
        if (ps_num_attached_ports(ps_dev->port_tcpd) > 1)
        {
            ps_partition_power(ps_dev);
            ps_renegotiate_ports(ps_dev);
        }
    }
}

void ps_event_pd_device(struct port_tcpd *dev, void *ctx)
{
    struct power_sharing_dev *ps_dev               = PS_DEV;
    ps_dev->sink_cap[dev->port_tcpd_id].pd_capable = 1;
}

void ps_event_pd_snk_cap_ext(struct port_tcpd *dev, void *ctx)
{
    struct power_sharing_dev *ps_dev  = PS_DEV;
    EVENT_PD_EXT_SNK_CAP_T   *evt     = (EVENT_PD_EXT_SNK_CAP_T *)ctx;
    uint32_t                  ext_min = 0;
    uint32_t                  ext_op  = 0;
    uint32_t                  ext_max = 0;

    if (evt->arg.success)
    {
        ext_min = evt->arg.buf->SinkMinimumPDP * 1000;
        ext_op  = evt->arg.buf->SinkOperationalPDP * 1000;
        ext_max = evt->arg.buf->SinkMaximumPDP * 1000;

        ps_dev->sink_cap[dev->port_tcpd_id].min_power =
            MAX(ps_dev->sink_cap[dev->port_tcpd_id].min_power, ext_min);
        ps_dev->sink_cap[dev->port_tcpd_id].op_power =
            MAX(ps_dev->sink_cap[dev->port_tcpd_id].op_power, ext_op);
        ps_dev->sink_cap[dev->port_tcpd_id].max_power =
            MAX(ps_dev->sink_cap[dev->port_tcpd_id].max_power, ext_max);
    }

    port_transmit_get_battery_capability(ps_dev->port_tcpd[dev->port_tcpd_id]);
}

void ps_event_battery_cap(struct port_tcpd *dev, void *ctx)
{
    struct power_sharing_dev *ps_dev = PS_DEV;
    EVENT_PD_GET_BAT_CAP_T   *evt    = (EVENT_PD_GET_BAT_CAP_T *)ctx;

    if (evt->arg.success)
    {
        if (evt->arg.cap->InvalidReference == 0 && evt->arg.cap->DesignCap != 0x0000
            && evt->arg.cap->DesignCap != 0xFFFF)
        {
            ps_dev->sink_cap[dev->port_tcpd_id].battery[evt->arg.ref].capacity =
                evt->arg.cap->DesignCap;
            //TODO: Is this better?
            //			ps_dev->sink_cap[dev->id].battery[evt->arg.ref].capacity =
            //					evt->arg.cap->LastFullCap;
        }
    }

    port_transmit_get_battery_status(ps_dev->port_tcpd[dev->port_tcpd_id]);
}

void ps_event_battery_status(struct port_tcpd *dev, void *ctx)
{
    struct power_sharing_dev *ps_dev = PS_DEV;
    EVENT_PD_GET_BAT_STAT_T  *evt    = (EVENT_PD_GET_BAT_STAT_T *)ctx;

    if (evt->arg.success)
    {
        if (evt->ret.bat_stat->BSDO.InvalidReference == 0
            && evt->ret.bat_stat->BSDO.BatteryPresent == 1)
        {
            if (evt->ret.bat_stat->BSDO.BatteryCapacity != 0xFFFF)
            {
                ps_dev->sink_cap[dev->port_tcpd_id].battery[evt->arg.ref].soc =
                    evt->ret.bat_stat->BSDO.BatteryCapacity;

                timer_start(&ps_dev->sink_cap[dev->port_tcpd_id].status_request_timer,
                            BATT_STATUS_TIME);
            }
            ps_dev->sink_cap[dev->port_tcpd_id].battery[evt->arg.ref].charge_status =
                evt->ret.bat_stat->BSDO.BatteryStatus;
        }
    }

    ps_dev->sink_cap[dev->port_tcpd_id].mismatch_handling = 1;

    if (ps_num_attached_ports(ps_dev->port_tcpd) > 1)
    {
        ps_partition_power(ps_dev);
        ps_renegotiate_ports(ps_dev);
    }
}

uint32_t ps_min_timer(struct power_sharing_dev *ps_dev)
{
    uint32_t min  = ~0;
    uint32_t time = ~0;
    uint8_t  i    = 0;

    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        if (!timer_disabled(&ps_dev->sink_cap[i].status_request_timer))
        {
            time = timer_remaining(&ps_dev->sink_cap[i].status_request_timer);
            if (time < min)
            {
                min = time;
            }
        }
    }

    return min;
}
#if !CONFIG_ENABLED(CONST_EVENTS)
void port_ps_register_callbacks(void)
{
    /* Power Sharing */
    event_subscribe(EVENT_PD_GET_SRC_CAP, ps_event_pd_get_src_cap);
    event_subscribe(EVENT_PD_SRC_EVAL_SNK_REQ, ps_event_pd_req_cb);
    event_subscribe(EVENT_PD_NEW_CONTRACT, ps_event_pd_new_contract);
    event_subscribe(EVENT_BIST_SHARED_TEST_MODE, ps_event_bist_shared_test_mode);

    /* Intelligent Power Sharing */
    event_subscribe(EVENT_PD_SNK_CAP_RECEIVED, ps_event_pd_snk_cap);
    event_subscribe(EVENT_EXT_SNK_CAP_RECEIVED, ps_event_pd_snk_cap_ext);
    event_subscribe(EVENT_PD_BAT_CAP_RECEIVED, ps_event_battery_cap);
    event_subscribe(EVENT_PD_BAT_STAT_RECEIVED, ps_event_battery_status);
}
#endif
void port_ps_processing(struct power_sharing_dev *ps_dev)
{
    uint8_t i = 0;

    /* Resend Get Battery Status after timeout */
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        if (timer_expired(&ps_dev->sink_cap[i].status_request_timer)
            && !timer_disabled(&ps_dev->sink_cap[i].status_request_timer))
        {
            timer_disable(&ps_dev->sink_cap[i].status_request_timer);
            port_transmit_get_battery_status(ps_dev->port_tcpd[i]);
        }
    }
}

void ps_update_shared_capacity(struct power_sharing_dev *ps_dev, uint32_t new_capacity)
{
    uint8_t i = 0;

    if (new_capacity == ps_dev->shared_capacity)
    {
        return;
    }

    /* Set current PD contract to invalid if going down in power */
    if (new_capacity < ps_dev->shared_capacity)
    {
        for (i = 0; i < TYPE_C_PORT_COUNT; i++)
        {
            port_set_contract_valid(ps_dev->port_tcpd[i], false);
            ps_dev->available_power += ps_dev->sink_cap[i].pdo_power;
            ps_dev->sink_cap[i].pdo_power = 7500;
            ps_dev->available_power -= ps_dev->sink_cap[i].pdo_power;
        }
    }

    /* Recalculate power profile */
    ps_dev->available_power += new_capacity - ps_dev->shared_capacity;
    ps_dev->pdp_rating =
        MIN(ps_dev->port_tcpd[0]->vif->PD_Power_as_Source, (new_capacity - MIN_POWER_1_5A));
    ps_dev->shared_capacity = new_capacity;

    /* Set ports to renegotiate */
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        if (port_policy_has_contract(ps_dev->port_tcpd[i]) == true)
        {
            ps_dev->sink_cap[i].port_power  = ps_dev->pdp_rating;
            ps_dev->sink_cap[i].renegotiate = 1;
        }
    }

    /* Repartition power if multiple ports attached */
    if (ps_num_attached_ports(ps_dev->port_tcpd) > 1)
    {
        ps_partition_power(ps_dev);
    }

    ps_renegotiate_ports(ps_dev);
}

#endif // CONFIG_ENABLED(POWER_SHARING)
