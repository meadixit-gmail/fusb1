/*******************************************************************************
 * @file     observer.c
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
#include "observer.h"
#if (CONFIG_ENABLED(CONST_EVENTS))
extern event_callback_fn EVENT_HANDLES[CONFIG_MAX_EVENTS];
#else
static event_callback_fn EVENT_HANDLES[CONFIG_MAX_EVENTS] = {0};

bool event_subscribe(long event, event_callback_fn handler)
{
    bool status = false;
    if (event < CONFIG_MAX_EVENTS)
    {
        EVENT_HANDLES[event] = handler;
        status               = true;
    }

    return status;
}

void event_unsubscribe(long event)
{
    if (event < CONFIG_MAX_EVENTS)
    {
        EVENT_HANDLES[event] = 0;
    }
}
#endif
void event_notify(long event, struct port_tcpd *dev, void *usr_ctx)
{
    if (EVENT_HANDLES[event] != 0 && event < CONFIG_MAX_EVENTS)
    {
    	EVENT_HANDLES[event](dev, usr_ctx);
    }
}
